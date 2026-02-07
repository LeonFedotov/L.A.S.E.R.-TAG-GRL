/**
 * PostProcessor - WebGL-based post-processing effects
 * Provides bloom/glow effects for the canvas output
 */

export class PostProcessor {
  constructor() {
    this.canvas = null;
    this.gl = null;
    this.programs = {};
    this.textures = {};
    this.framebuffers = {};
    this.enabled = false;

    // Effect parameters
    this.params = {
      bloomEnabled: false,  // Disabled by default
      bloomIntensity: 1.2,  // Strong bloom when enabled
      bloomThreshold: 0.1,  // Low threshold to catch more glow
      bloomBlurPasses: 4,
      bloomBlurSize: 3.0    // Wider blur spread
    };
  }

  /**
   * Initialize WebGL context and shaders
   * @param {number} width - Canvas width
   * @param {number} height - Canvas height
   */
  init(width, height) {
    this.width = width;
    this.height = height;

    // Create offscreen canvas for WebGL
    this.canvas = document.createElement('canvas');
    this.canvas.width = width;
    this.canvas.height = height;

    this.gl = this.canvas.getContext('webgl2') || this.canvas.getContext('webgl');
    if (!this.gl) {
      console.warn('WebGL not available, post-processing disabled');
      return false;
    }

    const gl = this.gl;

    // Enable required extensions
    gl.getExtension('OES_texture_float');
    gl.getExtension('OES_texture_float_linear');

    // Create shaders
    this.createShaders();

    // Create framebuffers for multi-pass rendering
    this.createFramebuffers();

    // Create fullscreen quad
    this.createQuad();

    this.enabled = true;
    return true;
  }

  /**
   * Create shader programs
   */
  createShaders() {
    const gl = this.gl;

    // Vertex shader (shared by all effects)
    const vertexShaderSource = `
      attribute vec2 a_position;
      attribute vec2 a_texCoord;
      varying vec2 v_texCoord;
      void main() {
        gl_Position = vec4(a_position, 0.0, 1.0);
        v_texCoord = a_texCoord;
      }
    `;

    // Passthrough shader (copy texture)
    const passthroughFragmentSource = `
      precision mediump float;
      varying vec2 v_texCoord;
      uniform sampler2D u_texture;
      void main() {
        gl_FragColor = texture2D(u_texture, v_texCoord);
      }
    `;

    // Brightness threshold shader (extract bright areas)
    // Uses max channel value instead of luminance for better glow on saturated colors
    const thresholdFragmentSource = `
      precision mediump float;
      varying vec2 v_texCoord;
      uniform sampler2D u_texture;
      uniform float u_threshold;
      void main() {
        vec4 color = texture2D(u_texture, v_texCoord);
        float brightness = max(max(color.r, color.g), color.b);
        if (brightness > u_threshold) {
          gl_FragColor = color;
        } else {
          gl_FragColor = vec4(0.0, 0.0, 0.0, color.a);
        }
      }
    `;

    // Gaussian blur shader (horizontal)
    const blurHFragmentSource = `
      precision mediump float;
      varying vec2 v_texCoord;
      uniform sampler2D u_texture;
      uniform vec2 u_texelSize;
      uniform float u_blurSize;

      void main() {
        vec4 color = vec4(0.0);
        float weights[5];
        weights[0] = 0.227027;
        weights[1] = 0.1945946;
        weights[2] = 0.1216216;
        weights[3] = 0.054054;
        weights[4] = 0.016216;

        color += texture2D(u_texture, v_texCoord) * weights[0];
        for (int i = 1; i < 5; i++) {
          vec2 offset = vec2(float(i) * u_blurSize * u_texelSize.x, 0.0);
          color += texture2D(u_texture, v_texCoord + offset) * weights[i];
          color += texture2D(u_texture, v_texCoord - offset) * weights[i];
        }
        gl_FragColor = color;
      }
    `;

    // Gaussian blur shader (vertical)
    const blurVFragmentSource = `
      precision mediump float;
      varying vec2 v_texCoord;
      uniform sampler2D u_texture;
      uniform vec2 u_texelSize;
      uniform float u_blurSize;

      void main() {
        vec4 color = vec4(0.0);
        float weights[5];
        weights[0] = 0.227027;
        weights[1] = 0.1945946;
        weights[2] = 0.1216216;
        weights[3] = 0.054054;
        weights[4] = 0.016216;

        color += texture2D(u_texture, v_texCoord) * weights[0];
        for (int i = 1; i < 5; i++) {
          vec2 offset = vec2(0.0, float(i) * u_blurSize * u_texelSize.y);
          color += texture2D(u_texture, v_texCoord + offset) * weights[i];
          color += texture2D(u_texture, v_texCoord - offset) * weights[i];
        }
        gl_FragColor = color;
      }
    `;

    // Combine shader (add bloom to original)
    const combineFragmentSource = `
      precision mediump float;
      varying vec2 v_texCoord;
      uniform sampler2D u_texture;
      uniform sampler2D u_bloomTexture;
      uniform float u_bloomIntensity;
      void main() {
        vec4 original = texture2D(u_texture, v_texCoord);
        vec4 bloom = texture2D(u_bloomTexture, v_texCoord);
        gl_FragColor = original + bloom * u_bloomIntensity;
      }
    `;

    // Compile shaders
    this.programs.passthrough = this.createProgram(vertexShaderSource, passthroughFragmentSource);
    this.programs.threshold = this.createProgram(vertexShaderSource, thresholdFragmentSource);
    this.programs.blurH = this.createProgram(vertexShaderSource, blurHFragmentSource);
    this.programs.blurV = this.createProgram(vertexShaderSource, blurVFragmentSource);
    this.programs.combine = this.createProgram(vertexShaderSource, combineFragmentSource);
  }

  /**
   * Create a shader program
   */
  createProgram(vertexSource, fragmentSource) {
    const gl = this.gl;

    const vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexSource);
    gl.compileShader(vertexShader);
    if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
      console.error('Vertex shader error:', gl.getShaderInfoLog(vertexShader));
      return null;
    }

    const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentSource);
    gl.compileShader(fragmentShader);
    if (!gl.getShaderParameter(fragmentShader, gl.COMPILE_STATUS)) {
      console.error('Fragment shader error:', gl.getShaderInfoLog(fragmentShader));
      return null;
    }

    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.error('Program link error:', gl.getProgramInfoLog(program));
      return null;
    }

    return program;
  }

  /**
   * Create framebuffers for multi-pass rendering
   */
  createFramebuffers() {
    const gl = this.gl;

    // Helper to create framebuffer with texture
    const createFB = (width, height) => {
      const texture = gl.createTexture();
      gl.bindTexture(gl.TEXTURE_2D, texture);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

      const fb = gl.createFramebuffer();
      gl.bindFramebuffer(gl.FRAMEBUFFER, fb);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);

      return { framebuffer: fb, texture };
    };

    // Full resolution buffers for quality (no downsampling)
    this.framebuffers.scene = createFB(this.width, this.height);
    this.framebuffers.bright = createFB(this.width, this.height);
    this.framebuffers.blurA = createFB(this.width, this.height);
    this.framebuffers.blurB = createFB(this.width, this.height);

    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  /**
   * Create fullscreen quad geometry
   */
  createQuad() {
    const gl = this.gl;

    // Positions (clip space: -1 to 1)
    const positions = new Float32Array([
      -1, -1,
       1, -1,
      -1,  1,
       1,  1
    ]);

    // Texture coordinates (standard - flip handled via UNPACK_FLIP_Y_WEBGL)
    const texCoords = new Float32Array([
      0, 0,
      1, 0,
      0, 1,
      1, 1
    ]);

    this.positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, positions, gl.STATIC_DRAW);

    this.texCoordBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, texCoords, gl.STATIC_DRAW);
  }

  /**
   * Set up vertex attributes for a program
   */
  setupAttributes(program) {
    const gl = this.gl;

    const posLoc = gl.getAttribLocation(program, 'a_position');
    gl.bindBuffer(gl.ARRAY_BUFFER, this.positionBuffer);
    gl.enableVertexAttribArray(posLoc);
    gl.vertexAttribPointer(posLoc, 2, gl.FLOAT, false, 0, 0);

    const texLoc = gl.getAttribLocation(program, 'a_texCoord');
    gl.bindBuffer(gl.ARRAY_BUFFER, this.texCoordBuffer);
    gl.enableVertexAttribArray(texLoc);
    gl.vertexAttribPointer(texLoc, 2, gl.FLOAT, false, 0, 0);
  }

  /**
   * Process a canvas with bloom effect
   * @param {HTMLCanvasElement} sourceCanvas - Input canvas
   * @returns {HTMLCanvasElement} - Processed canvas
   */
  process(sourceCanvas) {
    if (!this.enabled || !this.params.bloomEnabled) {
      return sourceCanvas;
    }

    const gl = this.gl;

    // Upload source canvas to texture
    if (!this.textures.source) {
      this.textures.source = gl.createTexture();
    }
    gl.bindTexture(gl.TEXTURE_2D, this.textures.source);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, sourceCanvas);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);

    // Pass 1: Extract bright areas
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffers.bright.framebuffer);
    gl.viewport(0, 0, this.width, this.height);
    gl.useProgram(this.programs.threshold);
    this.setupAttributes(this.programs.threshold);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.textures.source);
    gl.uniform1i(gl.getUniformLocation(this.programs.threshold, 'u_texture'), 0);
    gl.uniform1f(gl.getUniformLocation(this.programs.threshold, 'u_threshold'), this.params.bloomThreshold);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    // Pass 2+: Blur the bright areas (ping-pong between buffers)
    // Use full resolution for quality
    const blurW = this.width;
    const blurH = this.height;
    let readBuffer = this.framebuffers.bright;
    let writeBuffer = this.framebuffers.blurA;

    for (let i = 0; i < this.params.bloomBlurPasses; i++) {
      // Horizontal blur
      gl.bindFramebuffer(gl.FRAMEBUFFER, writeBuffer.framebuffer);
      gl.viewport(0, 0, blurW, blurH);
      gl.useProgram(this.programs.blurH);
      this.setupAttributes(this.programs.blurH);

      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, readBuffer.texture);
      gl.uniform1i(gl.getUniformLocation(this.programs.blurH, 'u_texture'), 0);
      gl.uniform2f(gl.getUniformLocation(this.programs.blurH, 'u_texelSize'), 1.0 / blurW, 1.0 / blurH);
      gl.uniform1f(gl.getUniformLocation(this.programs.blurH, 'u_blurSize'), this.params.bloomBlurSize);

      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

      // Swap buffers
      readBuffer = writeBuffer;
      writeBuffer = (writeBuffer === this.framebuffers.blurA) ? this.framebuffers.blurB : this.framebuffers.blurA;

      // Vertical blur
      gl.bindFramebuffer(gl.FRAMEBUFFER, writeBuffer.framebuffer);
      gl.viewport(0, 0, blurW, blurH);
      gl.useProgram(this.programs.blurV);
      this.setupAttributes(this.programs.blurV);

      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, readBuffer.texture);
      gl.uniform1i(gl.getUniformLocation(this.programs.blurV, 'u_texture'), 0);
      gl.uniform2f(gl.getUniformLocation(this.programs.blurV, 'u_texelSize'), 1.0 / blurW, 1.0 / blurH);
      gl.uniform1f(gl.getUniformLocation(this.programs.blurV, 'u_blurSize'), this.params.bloomBlurSize);

      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

      // Swap for next iteration
      readBuffer = writeBuffer;
      writeBuffer = (writeBuffer === this.framebuffers.blurA) ? this.framebuffers.blurB : this.framebuffers.blurA;
    }

    // Final pass: Combine original + bloom
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.viewport(0, 0, this.width, this.height);
    gl.useProgram(this.programs.combine);
    this.setupAttributes(this.programs.combine);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, this.textures.source);
    gl.uniform1i(gl.getUniformLocation(this.programs.combine, 'u_texture'), 0);

    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, readBuffer.texture);
    gl.uniform1i(gl.getUniformLocation(this.programs.combine, 'u_bloomTexture'), 1);

    gl.uniform1f(gl.getUniformLocation(this.programs.combine, 'u_bloomIntensity'), this.params.bloomIntensity);

    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

    return this.canvas;
  }

  /**
   * Resize the post-processor
   */
  resize(width, height) {
    this.width = width;
    this.height = height;
    this.canvas.width = width;
    this.canvas.height = height;

    // Recreate framebuffers at new size
    if (this.enabled) {
      this.createFramebuffers();
    }
  }

  /**
   * Clean up WebGL resources
   */
  dispose() {
    if (!this.gl) return;

    const gl = this.gl;

    // Delete textures
    for (const tex of Object.values(this.textures)) {
      gl.deleteTexture(tex);
    }

    // Delete framebuffers
    for (const fb of Object.values(this.framebuffers)) {
      gl.deleteFramebuffer(fb.framebuffer);
      gl.deleteTexture(fb.texture);
    }

    // Delete programs
    for (const prog of Object.values(this.programs)) {
      gl.deleteProgram(prog);
    }

    // Delete buffers
    gl.deleteBuffer(this.positionBuffer);
    gl.deleteBuffer(this.texCoordBuffer);

    this.enabled = false;
  }
}
