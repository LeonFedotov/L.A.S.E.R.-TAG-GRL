/**
 * AppGuiAdapter - Facade for GUI interactions with AppController
 * Decouples TweakpaneGui from AppController internals
 *
 * This adapter provides a clean interface for the GUI to interact with
 * the application, hiding implementation details and reducing coupling.
 */
export class AppGuiAdapter {
  constructor(app) {
    this.app = app;
  }

  // =========================================
  // Brush Interface
  // =========================================

  /**
   * Get list of available brushes
   * @returns {Array<{index: number, name: string, active: boolean}>}
   */
  getBrushList() {
    return this.app.getBrushList();
  }

  /**
   * Get currently active brush
   * @returns {BaseBrush}
   */
  getActiveBrush() {
    return this.app.getActiveBrush();
  }

  /**
   * Set active brush by index
   * @param {number} index
   */
  setActiveBrush(index) {
    this.app.setActiveBrush(index);
  }

  /**
   * Set brush color
   * @param {string} hexColor - Hex color string (e.g., '#FF0000')
   */
  setBrushColor(hexColor) {
    this.app.setBrushColor(hexColor);
  }

  /**
   * Set brush width
   * @param {number} width - Width in pixels
   */
  setBrushWidth(width) {
    this.app.setBrushWidth(width);
  }

  /**
   * Clear all brush canvases
   */
  clearCanvas() {
    this.app.clearCanvas();
  }

  /**
   * Undo last stroke
   */
  undo() {
    this.app.undo();
  }

  // =========================================
  // Camera Interface
  // =========================================

  /**
   * Set camera horizontal flip
   * @param {boolean} flip
   */
  setCameraFlipH(flip) {
    if (this.app.camera) {
      this.app.camera.setFlipH(flip);
    }
  }

  /**
   * Set camera vertical flip
   * @param {boolean} flip
   */
  setCameraFlipV(flip) {
    if (this.app.camera) {
      this.app.camera.setFlipV(flip);
    }
  }

  /**
   * Set camera rotation
   * @param {number} degrees - Rotation in degrees (0, 90, 180, 270)
   */
  setCameraRotation(degrees) {
    if (this.app.camera) {
      this.app.camera.setRotation(degrees);
    }
  }

  /**
   * Get current camera device ID from stream
   * @returns {string|null}
   */
  getCurrentCameraDeviceId() {
    if (!this.app.camera?.stream) return null;
    const videoTrack = this.app.camera.stream.getVideoTracks()[0];
    if (!videoTrack) return null;
    const settings = videoTrack.getSettings();
    return settings.deviceId || null;
  }

  /**
   * Get available cameras (static method access)
   * @returns {Promise<Array>}
   */
  async getAvailableCameras() {
    if (!this.app.camera) return [];
    return this.app.camera.constructor.getAvailableCameras();
  }

  /**
   * Change camera resolution and update related canvases/tracker
   * @param {number} width
   * @param {number} height
   */
  async setCameraResolution(width, height) {
    if (!this.app.camera) return;

    await this.app.camera.setResolution(width, height);

    // Update canvases to match new resolution
    const newWidth = this.app.camera.width;
    const newHeight = this.app.camera.height;

    if (this.app.debugCanvas) {
      this.app.debugCanvas.width = newWidth;
      this.app.debugCanvas.height = newHeight;
    }
    if (this.app.captureCanvas) {
      this.app.captureCanvas.width = newWidth;
      this.app.captureCanvas.height = newHeight;
    }

    // Reinitialize tracker with new dimensions
    if (this.app.tracker) {
      this.app.tracker.init(newWidth, newHeight);
    }

    console.log('Resolution changed to:', newWidth, 'x', newHeight);
  }

  /**
   * Switch camera and update related canvases
   * @param {string} deviceId
   */
  async switchCameraDevice(deviceId) {
    if (!this.app.camera) return;

    await this.app.camera.switchCamera(deviceId);

    // Update canvases to match camera dimensions
    const width = this.app.camera.width;
    const height = this.app.camera.height;

    if (this.app.debugCanvas) {
      this.app.debugCanvas.width = width;
      this.app.debugCanvas.height = height;
    }
    if (this.app.captureCanvas) {
      this.app.captureCanvas.width = width;
      this.app.captureCanvas.height = height;
    }
  }

  // =========================================
  // Tracker Interface
  // =========================================

  /**
   * Get tracker parameters
   * @returns {Object}
   */
  getTrackerParams() {
    return this.app.getTrackerParams();
  }

  /**
   * Set tracker parameters
   * @param {Object} params
   */
  setTrackerParams(params) {
    this.app.setTrackerParams(params);
  }

  // =========================================
  // Effects Interface (Post-Processing)
  // =========================================

  /**
   * Set bloom enabled
   * @param {boolean} enabled
   */
  setBloomEnabled(enabled) {
    if (this.app.postProcessor) {
      this.app.postProcessor.params.bloomEnabled = enabled;
    }
  }

  /**
   * Set bloom intensity
   * @param {number} intensity
   */
  setBloomIntensity(intensity) {
    if (this.app.postProcessor) {
      this.app.postProcessor.params.bloomIntensity = intensity;
    }
  }

  /**
   * Set bloom threshold
   * @param {number} threshold
   */
  setBloomThreshold(threshold) {
    if (this.app.postProcessor) {
      this.app.postProcessor.params.bloomThreshold = threshold;
    }
  }

  // =========================================
  // Calibration Interface
  // =========================================

  /**
   * Toggle camera calibration mode
   * @returns {boolean} New state
   */
  toggleCalibration() {
    return this.app.toggleCalibration();
  }

  /**
   * Reset camera calibration
   */
  resetCalibration() {
    this.app.resetCalibration();
  }

  /**
   * Toggle projector calibration mode
   * @returns {boolean} New state
   */
  toggleProjectorCalibration() {
    return this.app.toggleProjectorCalibration();
  }

  /**
   * Check if projector calibration is active
   * @returns {boolean}
   */
  isProjectorCalibrating() {
    return this.app.isProjectorCalibrating;
  }

  /**
   * Get selected projector calibration point
   * @returns {number}
   */
  getProjectorSelectedPoint() {
    return this.app.projectorSelectedPoint;
  }

  /**
   * Set selected projector calibration point
   * @param {number} index
   */
  setProjectorSelectedPoint(index) {
    this.app.projectorSelectedPoint = index;
  }

  /**
   * Select projector point at coordinates
   * @param {number} clientX
   * @param {number} clientY
   * @param {HTMLCanvasElement} canvas
   * @returns {number} Point index
   */
  selectProjectorPoint(clientX, clientY, canvas) {
    return this.app.selectProjectorPoint(clientX, clientY, canvas);
  }

  /**
   * Move projector point
   * @param {number} pointIndex
   * @param {number} clientX
   * @param {number} clientY
   * @param {HTMLCanvasElement} canvas
   */
  moveProjectorPoint(pointIndex, clientX, clientY, canvas) {
    this.app.moveProjectorPoint(pointIndex, clientX, clientY, canvas);
  }

  /**
   * Reset projector calibration
   */
  resetProjectorCalibration() {
    this.app.resetProjectorCalibration();
  }

  /**
   * Set projector calibration checkerboard visibility
   * @param {boolean} show
   */
  setProjectorCheckerboard(show) {
    this.app.projectorCalibration.setCheckerboard(show);
  }

  /**
   * Reload both calibrations from localStorage (after preset load)
   */
  reloadCalibration() {
    this.app.cameraCalibration.warping.load();
    // Re-apply current projector dimensions — load() may restore an outdated dstQuad
    this.app.cameraCalibration.updateProjectorDimensions(
      this.app.projectorCanvas.width,
      this.app.projectorCanvas.height
    );
    this.app.projectorCalibration.load();
  }

  // =========================================
  // Settings Interface
  // =========================================

  /**
   * Set background color
   * @param {string} hexColor
   */
  setBackgroundColor(hexColor) {
    this.app.settings.backgroundColor = hexColor;
  }

  /**
   * Set erase zone settings
   * @param {Object} zone - {enabled, x, y, width, height}
   */
  setEraseZone(zone) {
    if (zone.enabled !== undefined) this.app.settings.eraseZoneEnabled = zone.enabled;
    if (zone.x !== undefined) this.app.settings.eraseZoneX = zone.x;
    if (zone.y !== undefined) this.app.settings.eraseZoneY = zone.y;
    if (zone.width !== undefined) this.app.settings.eraseZoneWidth = zone.width;
    if (zone.height !== undefined) this.app.settings.eraseZoneHeight = zone.height;
  }

  /**
   * Set mouse input mode
   * @param {boolean} enabled
   */
  setMouseInputEnabled(enabled) {
    this.app.useMouseInput = enabled;
    this.app.notifyStateChange('mouseInput', enabled);
  }

  // =========================================
  // Brushes Interface
  // =========================================

  /**
   * Get all brush instances for iteration
   * @returns {Array<BaseBrush>}
   */
  getBrushes() {
    return this.app.brushes;
  }

  // =========================================
  // Canvas References (for event handlers)
  // =========================================

  /**
   * Get projector canvas element
   * @returns {HTMLCanvasElement}
   */
  getProjectorCanvas() {
    return this.app.projectorCanvas;
  }

  // =========================================
  // Projector Popup Interface
  // =========================================

  /**
   * Set projector popup reference
   * @param {Object} popup
   */
  setProjectorPopup(popup) {
    this.app.projectorPopup = popup;
  }

  /**
   * Set projector resolution from popup window dimensions.
   * Pass null to clear (reverts to container-based sizing).
   * @param {number|null} width
   * @param {number|null} height
   */
  setProjectorResolution(width, height) {
    this.app.setProjectorResolution(width, height);
  }
}
