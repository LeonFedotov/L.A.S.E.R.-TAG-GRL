/**
 * CameraCalibrationManager - Handles camera-to-projector coordinate calibration
 * Extracted from AppController to separate calibration concerns
 */
import { CoordWarping } from '../tracking/CoordWarping.js';

export class CameraCalibrationManager {
  constructor() {
    this.warping = new CoordWarping();
    this.isCalibrating = false;
    this.onStateChange = null;
  }

  /**
   * Initialize with camera and projector dimensions
   * @param {number} cameraWidth - Camera resolution width
   * @param {number} cameraHeight - Camera resolution height
   * @param {number} projectorWidth - Projector canvas width
   * @param {number} projectorHeight - Projector canvas height
   */
  init(cameraWidth, cameraHeight, projectorWidth, projectorHeight) {
    this.warping.setSourceDimensions(cameraWidth, cameraHeight);
    this.warping.setDestinationDimensions(projectorWidth, projectorHeight);
    this.warping.load();
  }

  /**
   * Update projector dimensions (e.g., on resize)
   * @param {number} width - New projector width
   * @param {number} height - New projector height
   */
  updateProjectorDimensions(width, height) {
    this.warping.setDestinationDimensions(width, height);
  }

  /**
   * Toggle calibration mode
   * @returns {boolean} New calibration state
   */
  toggle() {
    this.isCalibrating = !this.isCalibrating;
    this.notifyStateChange('calibrating', this.isCalibrating);
    return this.isCalibrating;
  }

  /**
   * Transform camera coordinates to projector coordinates
   * @param {number} x - Camera X coordinate
   * @param {number} y - Camera Y coordinate
   * @returns {{x: number, y: number}} Transformed coordinates
   */
  transform(x, y) {
    return this.warping.transform(x, y);
  }

  /**
   * Check if a point is inside the calibration area
   * @param {number} x - Camera X coordinate
   * @param {number} y - Camera Y coordinate
   * @returns {boolean} True if point is inside the calibrated quad
   */
  isInsideCalibrationArea(x, y) {
    return this.warping.isInsideSourceQuad(x, y);
  }

  /**
   * Find calibration point near given camera coordinates
   * @param {number} camX - Camera X coordinate
   * @param {number} camY - Camera Y coordinate
   * @param {number} threshold - Distance threshold in pixels
   * @returns {number} Point index or -1 if none found
   */
  findNearestPoint(camX, camY, threshold = 30) {
    if (!this.isCalibrating) return -1;
    return this.warping.findNearestPoint(camX, camY, threshold);
  }

  /**
   * Select calibration point from debug canvas click
   * @param {number} clickX - Click X in CSS pixels
   * @param {number} clickY - Click Y in CSS pixels
   * @param {HTMLCanvasElement} debugCanvas - Debug canvas element
   * @param {number} cameraWidth - Camera width
   * @param {number} cameraHeight - Camera height
   * @returns {number} Point index or -1 if none found
   */
  selectPoint(clickX, clickY, debugCanvas, cameraWidth, cameraHeight) {
    if (!this.isCalibrating) return -1;

    const rect = debugCanvas.getBoundingClientRect();
    const scaleX = cameraWidth / rect.width;
    const scaleY = cameraHeight / rect.height;

    const camX = clickX * scaleX;
    const camY = clickY * scaleY;

    return this.warping.findNearestPoint(camX, camY, 30);
  }

  /**
   * Move a calibration point
   * @param {number} pointIndex - Point index
   * @param {number} clickX - New X in CSS pixels
   * @param {number} clickY - New Y in CSS pixels
   * @param {HTMLCanvasElement} debugCanvas - Debug canvas element
   * @param {number} cameraWidth - Camera width
   * @param {number} cameraHeight - Camera height
   */
  movePoint(pointIndex, clickX, clickY, debugCanvas, cameraWidth, cameraHeight) {
    if (pointIndex < 0) return;

    const rect = debugCanvas.getBoundingClientRect();
    const scaleX = cameraWidth / rect.width;
    const scaleY = cameraHeight / rect.height;

    this.warping.setSourcePoint(pointIndex, clickX * scaleX, clickY * scaleY);

    // Autosave calibration on every move
    this.save();
  }

  /**
   * Draw calibration overlay on debug canvas
   * @param {CanvasRenderingContext2D} ctx - Debug canvas context
   * @param {number} cameraWidth - Camera width
   * @param {number} cameraHeight - Camera height
   * @param {number} canvasWidth - Debug canvas width
   * @param {number} canvasHeight - Debug canvas height
   */
  draw(ctx, cameraWidth, cameraHeight, canvasWidth, canvasHeight) {
    if (!this.isCalibrating) return;

    const scaleX = canvasWidth / cameraWidth;
    const scaleY = canvasHeight / cameraHeight;
    this.warping.draw(ctx, scaleX, scaleY);
  }

  /**
   * Get the source quad (camera calibration corners) in pixel coordinates
   * @returns {Array<{x: number, y: number}>} 4 corner points
   */
  getSourceQuad() {
    return this.warping.srcQuad;
  }

  /**
   * Save calibration to localStorage
   */
  save() {
    this.warping.save();
    console.log('Camera calibration saved');
  }

  /**
   * Reset calibration to default (identity mapping)
   * @param {number} cameraWidth - Camera width
   * @param {number} cameraHeight - Camera height
   */
  reset(cameraWidth, cameraHeight) {
    this.warping.setSourceDimensions(cameraWidth, cameraHeight);
    console.log('Camera calibration reset');
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
