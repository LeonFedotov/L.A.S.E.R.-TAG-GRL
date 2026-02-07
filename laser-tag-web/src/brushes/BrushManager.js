/**
 * BrushManager - Manages brush instances and operations
 * Extracted from AppController to separate brush concerns
 */
import { VectorBrush } from './VectorBrush.js';
import { getAvailableModes } from './modes/index.js';
import { hexToRgb } from '../utils/ColorUtils.js';

export class BrushManager {
  constructor() {
    this.brushes = [];
    this.activeBrushIndex = 0;
    this.onStateChange = null;
    this.modes = getAvailableModes();
  }

  /**
   * Initialize brushes with canvas dimensions
   * @param {number} width - Canvas width
   * @param {number} height - Canvas height
   */
  init(width, height) {
    // Vector brush (only mode for now)
    const vectorBrush = new VectorBrush();
    vectorBrush.init(width, height);
    this.brushes.push(vectorBrush);

    console.log(`Initialized ${this.brushes.length} brush(es)`);
  }

  /**
   * Resize all brush canvases
   * @param {number} width - New width
   * @param {number} height - New height
   */
  resize(width, height) {
    for (const brush of this.brushes) {
      brush.init(width, height);
    }
  }

  /**
   * Get the currently active brush
   * @returns {BaseBrush}
   */
  getActiveBrush() {
    return this.brushes[this.activeBrushIndex];
  }

  /**
   * Set active brush by index (deprecated - use setMode instead)
   * @param {number} index - Brush index
   */
  setActiveBrush(index) {
    // Now switches modes instead of brushes
    this.setModeByIndex(index);
  }

  /**
   * Set brush mode by index (1-based for keyboard shortcuts)
   * @param {number} index - Mode index (0-based)
   */
  setModeByIndex(index) {
    if (index >= 0 && index < this.modes.length) {
      const modeName = this.modes[index];
      this.setMode(modeName);
    }
  }

  /**
   * Set brush mode by name
   * @param {string} modeName - Mode name (smooth, glow, basic, dope, arrow, arrowFat)
   */
  setMode(modeName) {
    const brush = this.getActiveBrush();
    if (brush && this.modes.includes(modeName)) {
      brush.params.mode = modeName;
      this.notifyStateChange('mode', modeName);
    }
  }

  /**
   * Get current mode name
   * @returns {string}
   */
  getMode() {
    const brush = this.getActiveBrush();
    return brush ? brush.params.mode : 'smooth';
  }

  /**
   * Get list of available modes
   * @returns {string[]}
   */
  getModeList() {
    return this.modes;
  }

  /**
   * Get list of available brushes
   * @returns {Array<{index: number, name: string, active: boolean}>}
   */
  getBrushList() {
    return this.brushes.map((b, i) => ({
      index: i,
      name: b.name,
      active: i === this.activeBrushIndex
    }));
  }

  /**
   * Clear all brush canvases
   */
  clearAll() {
    for (const brush of this.brushes) {
      brush.clear();
    }
  }

  /**
   * Undo last stroke on active brush
   */
  undo() {
    this.getActiveBrush().undo();
  }

  /**
   * Set color for all brushes
   * @param {string} hexColor - Hex color string (e.g., '#FF0000')
   */
  setColor(hexColor) {
    const rgb = hexToRgb(hexColor);
    if (!rgb) return;

    for (const brush of this.brushes) {
      brush.setColor(rgb.r, rgb.g, rgb.b);
    }
  }

  /**
   * Set width for all brushes
   * @param {number} width - Width in pixels
   */
  setWidth(width) {
    for (const brush of this.brushes) {
      brush.setBrushWidth(width);
    }
  }

  /**
   * Add point to active brush
   * @param {number} x - Normalized X (0-1)
   * @param {number} y - Normalized Y (0-1)
   * @param {boolean} isNewStroke - Whether this starts a new stroke
   */
  addPoint(x, y, isNewStroke) {
    this.getActiveBrush().addPoint(x, y, isNewStroke);
  }

  /**
   * End current stroke on active brush
   */
  endStroke() {
    const brush = this.getActiveBrush();
    if (brush.isDrawing) {
      brush.endStroke();
    }
  }

  /**
   * Check if active brush is currently drawing
   * @returns {boolean}
   */
  isDrawing() {
    return this.getActiveBrush().isDrawing;
  }

  /**
   * Render all brushes (update drips, etc.)
   */
  render() {
    for (const brush of this.brushes) {
      brush.render();
    }
  }

  /**
   * Draw all brushes to a context
   * @param {CanvasRenderingContext2D} ctx - Target context
   */
  draw(ctx) {
    for (const brush of this.brushes) {
      brush.draw(ctx);
    }
  }

  /**
   * Dispose all brush resources
   */
  dispose() {
    for (const brush of this.brushes) {
      brush.dispose();
    }
    this.brushes = [];
  }

  /**
   * Notify listener of state changes
   * @param {string} key - State key
   * @param {*} value - New value
   */
  notifyStateChange(key, value) {
    if (this.onStateChange) {
      this.onStateChange(key, value);
    }
  }
}
