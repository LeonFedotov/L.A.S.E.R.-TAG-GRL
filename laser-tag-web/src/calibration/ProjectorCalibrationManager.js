/**
 * ProjectorCalibrationManager - Handles projector output quad calibration
 * Extracted from AppController to separate calibration concerns
 *
 * This calibrates the OUTPUT projection area - adjusting corners to match
 * the physical projection surface (keystoning, irregular surfaces, etc.)
 */
import { Homography } from '../utils/Homography.js';
import { storageSave, storageLoad } from '../utils/StorageUtils.js';

const STORAGE_KEY = 'laserTag_projectorQuad';

export class ProjectorCalibrationManager {
  constructor() {
    this.isCalibrating = false;
    this.selectedPoint = -1;
    this.onStateChange = null;
    this.showCheckerboard = false;

    // Quad corners in normalized 0-1 coordinates
    // Default is full canvas
    this.quad = [
      { x: 0, y: 0 },      // Top-left
      { x: 1, y: 0 },      // Top-right
      { x: 1, y: 1 },      // Bottom-right
      { x: 0, y: 1 }       // Bottom-left
    ];

    // Load saved calibration if exists
    this.load();
  }

  /**
   * Set checkerboard visibility
   * @param {boolean} show
   */
  setCheckerboard(show) {
    this.showCheckerboard = show;
  }

  /**
   * Check if quad differs from default (full canvas)
   * @returns {boolean} True if warping is needed
   */
  isWarped() {
    return this.quad.some((p, i) => {
      const defaultX = (i === 0 || i === 3) ? 0 : 1;
      const defaultY = (i === 0 || i === 1) ? 0 : 1;
      return Math.abs(p.x - defaultX) > 0.001 || Math.abs(p.y - defaultY) > 0.001;
    });
  }

  /**
   * Get current quad (normalized 0-1 coordinates)
   * @returns {Array<{x: number, y: number}>}
   */
  getQuad() {
    return this.quad;
  }

  /**
   * Set quad (normalized 0-1 coordinates)
   * @param {Array<{x: number, y: number}>} quad
   */
  setQuad(quad) {
    this.quad = quad;
  }

  /**
   * Toggle calibration mode
   * @returns {boolean} New calibration state
   */
  toggle() {
    this.isCalibrating = !this.isCalibrating;
    if (!this.isCalibrating) {
      this.selectedPoint = -1;
    }
    this.notifyStateChange('projectorCalibrating', this.isCalibrating);
    return this.isCalibrating;
  }

  /**
   * Create CSS matrix3d transform for perspective warping
   * @param {number} width - Canvas width
   * @param {number} height - Canvas height
   * @param {boolean} [forceCompute=false] - Compute even during calibration (for live preview)
   * @returns {string} CSS transform string
   */
  getCssTransform(width, height, forceCompute = false) {
    if (!this.isWarped()) {
      return 'none';
    }

    // Skip during calibration unless force is requested (for live preview)
    if (this.isCalibrating && !forceCompute) {
      return 'none';
    }

    const H = Homography.createProjectionMapping(width, height, this.quad, width, height);
    return Homography.toMatrix3d(H, width, height);
  }

  /**
   * Select calibration point at given client coordinates
   * @param {number} clientX - Client X coordinate
   * @param {number} clientY - Client Y coordinate
   * @param {HTMLCanvasElement} canvas - Target canvas
   * @returns {number} Point index or -1 if none found
   */
  selectPoint(clientX, clientY, canvas) {
    if (!this.isCalibrating) return -1;

    const rect = canvas.getBoundingClientRect();
    const x = clientX - rect.left;
    const y = clientY - rect.top;
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    const canvasX = x * scaleX;
    const canvasY = y * scaleY;

    const w = canvas.width;
    const h = canvas.height;
    const handleRadius = 30;

    let closestDist = handleRadius;
    let closestIdx = -1;

    this.quad.forEach((p, i) => {
      const px = p.x * w;
      const py = p.y * h;
      const dist = Math.sqrt((canvasX - px) ** 2 + (canvasY - py) ** 2);
      if (dist < closestDist) {
        closestDist = dist;
        closestIdx = i;
      }
    });

    this.selectedPoint = closestIdx;
    return closestIdx;
  }

  /**
   * Move selected calibration point
   * @param {number} pointIndex - Point index (0-3)
   * @param {number} clientX - Client X coordinate
   * @param {number} clientY - Client Y coordinate
   * @param {HTMLCanvasElement} canvas - Target canvas
   */
  movePoint(pointIndex, clientX, clientY, canvas) {
    if (pointIndex < 0 || pointIndex >= 4) return;

    const rect = canvas.getBoundingClientRect();
    const x = clientX - rect.left;
    const y = clientY - rect.top;
    const scaleX = canvas.width / rect.width;
    const scaleY = canvas.height / rect.height;
    const canvasX = x * scaleX;
    const canvasY = y * scaleY;

    this.quad[pointIndex] = {
      x: Math.max(0, Math.min(1, canvasX / canvas.width)),
      y: Math.max(0, Math.min(1, canvasY / canvas.height))
    };

    // Autosave calibration on every move
    this.save();
  }

  /**
   * Draw calibration overlay
   * @param {CanvasRenderingContext2D} ctx - Canvas context
   * @param {number} [width] - Canvas width
   * @param {number} [height] - Canvas height
   * @param {boolean} [skipCheckerboard=false] - Skip drawing checkerboard (drawn separately on warped canvas)
   */
  draw(ctx, width, height, skipCheckerboard = false) {
    const w = width || ctx.canvas.width;
    const h = height || ctx.canvas.height;

    // Convert normalized coords to pixels
    const points = this.quad.map(p => ({
      x: p.x * w,
      y: p.y * h
    }));

    // Checkerboard is drawn separately on warped canvas, not here
    // This is controlled by skipCheckerboard parameter

    // Draw the quad outline (white)
    ctx.beginPath();
    ctx.moveTo(points[0].x, points[0].y);
    ctx.lineTo(points[1].x, points[1].y);
    ctx.lineTo(points[2].x, points[2].y);
    ctx.lineTo(points[3].x, points[3].y);
    ctx.closePath();
    ctx.strokeStyle = '#FFFFFF';
    ctx.lineWidth = 3;
    ctx.stroke();

    // Draw diagonal lines for reference
    ctx.beginPath();
    ctx.moveTo(points[0].x, points[0].y);
    ctx.lineTo(points[2].x, points[2].y);
    ctx.moveTo(points[1].x, points[1].y);
    ctx.lineTo(points[3].x, points[3].y);
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
    ctx.lineWidth = 1;
    ctx.stroke();

    // Draw corner handles with colors (like original C++)
    const colors = ['#FF0000', '#00FF00', '#0000FF', '#00FFFF']; // TL, TR, BR, BL
    const labels = ['TL', 'TR', 'BR', 'BL'];
    const handleRadius = 15;

    points.forEach((p, i) => {
      // Outer circle
      ctx.beginPath();
      ctx.arc(p.x, p.y, handleRadius, 0, Math.PI * 2);
      ctx.fillStyle = this.selectedPoint === i ? '#FFFF00' : colors[i];
      ctx.fill();
      ctx.strokeStyle = '#FFFFFF';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Label
      ctx.fillStyle = '#000000';
      ctx.font = 'bold 10px monospace';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(labels[i], p.x, p.y);
    });
  }

  /**
   * Draw checkerboard pattern for calibration (clipped to quad, semi-transparent)
   * @param {CanvasRenderingContext2D} ctx
   * @param {number} w - Width
   * @param {number} h - Height
   * @param {Array<{x: number, y: number}>} points - Quad corner points in pixels
   */
  drawCheckerboard(ctx, w, h, points) {
    ctx.save();

    // Clip to quad shape
    ctx.beginPath();
    ctx.moveTo(points[0].x, points[0].y);
    ctx.lineTo(points[1].x, points[1].y);
    ctx.lineTo(points[2].x, points[2].y);
    ctx.lineTo(points[3].x, points[3].y);
    ctx.closePath();
    ctx.clip();

    // Draw semi-transparent checkerboard
    const tileSize = 40;
    const cols = Math.ceil(w / tileSize);
    const rows = Math.ceil(h / tileSize);

    ctx.globalAlpha = 0.5;
    for (let row = 0; row < rows; row++) {
      for (let col = 0; col < cols; col++) {
        const isWhite = (row + col) % 2 === 0;
        ctx.fillStyle = isWhite ? '#FFFFFF' : '#000000';
        ctx.fillRect(col * tileSize, row * tileSize, tileSize, tileSize);
      }
    }

    ctx.restore();
  }

  /**
   * Draw fullscreen checkerboard (for warped canvas - no clipping needed)
   * @param {CanvasRenderingContext2D} ctx
   * @param {number} w - Width
   * @param {number} h - Height
   */
  drawCheckerboardFullscreen(ctx, w, h) {
    const tileSize = 40;
    const cols = Math.ceil(w / tileSize);
    const rows = Math.ceil(h / tileSize);

    ctx.save();
    ctx.globalAlpha = 0.5;
    for (let row = 0; row < rows; row++) {
      for (let col = 0; col < cols; col++) {
        const isWhite = (row + col) % 2 === 0;
        ctx.fillStyle = isWhite ? '#FFFFFF' : '#000000';
        ctx.fillRect(col * tileSize, row * tileSize, tileSize, tileSize);
      }
    }
    ctx.restore();
  }

  /**
   * Save calibration to localStorage
   */
  save() {
    storageSave(STORAGE_KEY, this.quad, 'Projector calibration');
  }

  /**
   * Load calibration from localStorage
   */
  load() {
    const saved = storageLoad(STORAGE_KEY);
    if (saved) {
      this.quad = saved;
      console.log('Projector calibration loaded');
    }
  }

  /**
   * Reset calibration to full canvas
   */
  reset() {
    this.quad = [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 1, y: 1 },
      { x: 0, y: 1 }
    ];
    this.selectedPoint = -1;
    this.save();
    console.log('Projector calibration reset');
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
