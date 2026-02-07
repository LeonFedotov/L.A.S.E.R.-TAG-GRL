/**
 * PerspectiveWarp - WebGL-based perspective warp rendering
 * Renders a source canvas onto a target canvas with homography-based perspective correction.
 *
 * Uses a fragment shader that applies the inverse homography to compute texture
 * lookup coords per pixel, supporting both forward warp (rect→quad for projector)
 * and inverse warp (quad→rect for camera un-warping).
 */
import { Homography } from '../utils/Homography.js';

const VERTEX_SHADER = `
  attribute vec2 a_position;
  varying vec2 v_texCoord;
  void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    // Map clip coords (-1..1) to tex coords (0..1)
    v_texCoord = (a_position + 1.0) * 0.5;
  }
`;

const FRAGMENT_SHADER = `
  precision mediump float;
  varying vec2 v_texCoord;
  uniform sampler2D u_texture;
  uniform mat3 u_homography;
  uniform bool u_showCheckerboard;
  uniform float u_checkerSize;

  void main() {
    vec3 srcCoord = u_homography * vec3(v_texCoord, 1.0);
    vec2 uv = srcCoord.xy / srcCoord.z;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
      gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
      return;
    }

    vec4 color = texture2D(u_texture, uv);

    if (u_showCheckerboard) {
      float checker = mod(floor(v_texCoord.x * u_checkerSize) + floor(v_texCoord.y * u_checkerSize), 2.0);
      color = mix(color, vec4(checker, checker, checker, 1.0), 0.4);
    }

    gl_FragColor = color;
  }
`;

export class PerspectiveWarp {
  constructor() {
    this.gl = null;
    this.canvas = null;
    this.program = null;
    this.texture = null;
    this.positionBuffer = null;

    // Uniform locations (cached)
    this.uniforms = {};

    // Current homography (identity)
    this.homography = [1, 0, 0, 0, 1, 0, 0, 0, 1];
    this.showCheckerboard = false;
    this.checkerSize = 10.0;

    // Track canvas dimensions for auto-resize
    this._lastWidth = 0;
    this._lastHeight = 0;
  }

  /**
   * Initialize WebGL on an existing canvas element
   * @param {HTMLCanvasElement} canvas - DOM canvas to render into
   * @returns {boolean} True if init succeeded
   */
  init(canvas) {
    this.canvas = canvas;
    this.gl = canvas.getContext('webgl2', { premultipliedAlpha: false })
           || canvas.getContext('webgl', { premultipliedAlpha: false });

    if (!this.gl) {
      console.error('PerspectiveWarp: WebGL not available');
      return false;
    }

    const gl = this.gl;

    // Compile shaders
    this.program = this._createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!this.program) return false;

    // Cache uniform locations
    this.uniforms = {
      texture: gl.getUniformLocation(this.program, 'u_texture'),
      homography: gl.getUniformLocation(this.program, 'u_homography'),
      showCheckerboard: gl.getUniformLocation(this.program, 'u_showCheckerboard'),
      checkerSize: gl.getUniformLocation(this.program, 'u_checkerSize'),
    };

    // Create fullscreen quad (two triangles)
    this.positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1,  1, -1,  -1, 1,
       1, -1,  1,  1,  -1, 1,
    ]), gl.STATIC_DRAW);

    // Create texture for source canvas uploads
    this.texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    // Set up viewport
    this._lastWidth = canvas.width;
    this._lastHeight = canvas.height;
    gl.viewport(0, 0, canvas.width, canvas.height);

    return true;
  }

  /**
   * Update viewport if canvas dimensions changed
   * @param {number} width
   * @param {number} height
   */
  resize(width, height) {
    if (this.gl && (width !== this._lastWidth || height !== this._lastHeight)) {
      this._lastWidth = width;
      this._lastHeight = height;
      this.gl.viewport(0, 0, width, height);
    }
  }

  /**
   * Set forward warp: rect → quad (for projector output)
   * The shader needs the inverse to map output pixels back to source texture.
   * @param {Array<{x: number, y: number}>} normalizedQuad - 4 corners in 0-1 coords [TL, TR, BR, BL]
   */
  setForwardWarp(normalizedQuad) {
    // Forward: unit rect → quad
    const unitRect = [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 1, y: 1 },
      { x: 0, y: 1 }
    ];

    // Compute forward homography (rect → quad)
    const H = Homography.computeHomography(unitRect, normalizedQuad);
    // Shader needs inverse (quad → rect) to look up source texture coords
    this.homography = Homography.inverse(H);
  }

  /**
   * Set inverse warp: quad → rect (for camera un-warping)
   * Maps the camera calibration quad to a full rectangle.
   * @param {Array<{x: number, y: number}>} pixelQuad - 4 corners in pixel coords [TL, TR, BR, BL]
   * @param {number} srcW - Source image width
   * @param {number} srcH - Source image height
   */
  setInverseWarp(pixelQuad, srcW, srcH) {
    // Normalize pixel quad to 0-1
    const normalizedQuad = pixelQuad.map(p => ({
      x: p.x / srcW,
      y: p.y / srcH
    }));

    // We want: for each output pixel (in 0-1), find source pixel in the quad
    // Forward would be: quad → rect. Inverse (for shader) is: rect → quad
    const unitRect = [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 1, y: 1 },
      { x: 0, y: 1 }
    ];

    // Compute: rect → quad (this maps output coords to source quad coords)
    // The shader uses this directly: output texCoord → source texCoord in quad
    this.homography = Homography.computeHomography(unitRect, normalizedQuad);
  }

  /**
   * Toggle checkerboard overlay
   * @param {boolean} show
   * @param {number} [size=10] - Number of checker squares per axis
   */
  setCheckerboard(show, size) {
    this.showCheckerboard = show;
    if (size !== undefined) {
      this.checkerSize = size;
    }
  }

  /**
   * Render source canvas onto this warp's canvas
   * @param {HTMLCanvasElement|HTMLVideoElement} source - Source to render
   */
  render(source) {
    const gl = this.gl;
    if (!gl || !this.program) return;

    // Auto-resize viewport if canvas dimensions changed
    const w = this.canvas.width;
    const h = this.canvas.height;
    if (w !== this._lastWidth || h !== this._lastHeight) {
      this.resize(w, h);
    }

    // Upload source to texture
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, source);

    // Clear
    gl.clearColor(0, 0, 0, 1);
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Use program
    gl.useProgram(this.program);

    // Bind position buffer
    const posLoc = gl.getAttribLocation(this.program, 'a_position');
    gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
    gl.enableVertexAttribArray(posLoc);
    gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 0, 0);

    // Set uniforms
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.texture);
    gl.uniform1i(this.uniforms.texture, 0);

    // Homography as column-major mat3 for GLSL
    // Our flat array is row-major [h00, h01, h02, h10, h11, h12, h20, h21, h22]
    // GLSL mat3 expects column-major
    const H = this.homography;
    gl.uniformMatrix3fv(this.uniforms.homography, false, [
      H[0], H[3], H[6],  // column 0
      H[1], H[4], H[7],  // column 1
      H[2], H[5], H[8],  // column 2
    ]);

    gl.uniform1i(this.uniforms.showCheckerboard, this.showCheckerboard ? 1 : 0);
    gl.uniform1f(this.uniforms.checkerSize, this.checkerSize);

    // Draw
    gl.drawArrays(gl.TRIANGLES, 0, 6);
  }

  /**
   * Clean up WebGL resources
   */
  dispose() {
    const gl = this.gl;
    if (!gl) return;

    if (this.texture) gl.deleteTexture(this.texture);
    if (this.positionBuffer) gl.deleteBuffer(this.positionBuffer);
    if (this.program) gl.deleteProgram(this.program);

    this.gl = null;
    this.canvas = null;
  }

  /**
   * Compile and link a shader program
   * @private
   */
  _createProgram(vertexSrc, fragmentSrc) {
    const gl = this.gl;

    const vs = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vs, vertexSrc);
    gl.compileShader(vs);
    if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
      console.error('PerspectiveWarp vertex shader:', gl.getShaderInfoLog(vs));
      return null;
    }

    const fs = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fs, fragmentSrc);
    gl.compileShader(fs);
    if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
      console.error('PerspectiveWarp fragment shader:', gl.getShaderInfoLog(fs));
      return null;
    }

    const prog = gl.createProgram();
    gl.attachShader(prog, vs);
    gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
      console.error('PerspectiveWarp link:', gl.getProgramInfoLog(prog));
      return null;
    }

    // Shaders can be detached after linking
    gl.deleteShader(vs);
    gl.deleteShader(fs);

    return prog;
  }
}
