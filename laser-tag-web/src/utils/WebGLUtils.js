/**
 * WebGLUtils - Shared WebGL shader compilation utilities
 */

/**
 * Compile a single shader
 * @param {WebGLRenderingContext} gl
 * @param {number} type - gl.VERTEX_SHADER or gl.FRAGMENT_SHADER
 * @param {string} source - GLSL source
 * @param {string} debugName - Name for error messages
 * @returns {WebGLShader|null}
 */
function compileShader(gl, type, source, debugName) {
  const shader = gl.createShader(type);
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    console.error(`${debugName} shader error:`, gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
    return null;
  }
  return shader;
}

/**
 * Compile and link a vertex + fragment shader into a program
 * @param {WebGLRenderingContext} gl
 * @param {string} vertexSrc - Vertex shader GLSL
 * @param {string} fragmentSrc - Fragment shader GLSL
 * @param {string} [debugName='Shader'] - Name for error messages
 * @returns {WebGLProgram|null}
 */
export function createProgram(gl, vertexSrc, fragmentSrc, debugName = 'Shader') {
  const vs = compileShader(gl, gl.VERTEX_SHADER, vertexSrc, `${debugName} vertex`);
  if (!vs) return null;

  const fs = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSrc, `${debugName} fragment`);
  if (!fs) {
    gl.deleteShader(vs);
    return null;
  }

  const program = gl.createProgram();
  gl.attachShader(program, vs);
  gl.attachShader(program, fs);
  gl.linkProgram(program);

  // Shaders can be detached after linking
  gl.deleteShader(vs);
  gl.deleteShader(fs);

  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    console.error(`${debugName} link error:`, gl.getProgramInfoLog(program));
    gl.deleteProgram(program);
    return null;
  }

  return program;
}
