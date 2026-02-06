/**
 * AppController - Main application orchestrator
 * Based on appController.cpp from the original L.A.S.E.R. TAG
 */
import { Camera } from '../tracking/Camera.js';
import { LaserTracker } from '../tracking/LaserTracker.js';
import { PostProcessor } from '../effects/PostProcessor.js';
import { CameraCalibrationManager } from '../calibration/CameraCalibrationManager.js';
import { ProjectorCalibrationManager } from '../calibration/ProjectorCalibrationManager.js';
import { BrushManager } from '../brushes/BrushManager.js';
import { RenderingPipeline } from '../rendering/RenderingPipeline.js';

export class AppController {
  constructor() {
    // Core components
    this.camera = new Camera();
    this.tracker = new LaserTracker();

    // Calibration managers
    this.cameraCalibration = new CameraCalibrationManager();
    this.projectorCalibration = new ProjectorCalibrationManager();

    // Brush manager
    this.brushManager = new BrushManager();

    // Rendering pipeline
    this.renderingPipeline = new RenderingPipeline();

    // Canvas elements
    this.projectorCanvas = null;
    this.projectorCtx = null;
    this.debugCanvas = null;
    this.debugCtx = null;

    // Internal canvas for camera capture
    this.captureCanvas = null;
    this.captureCtx = null;

    // State
    this.isRunning = false;
    this.frameCount = 0;
    this.fps = 0;
    this.lastFpsUpdate = 0;

    // Mouse input mode (for testing without laser)
    this.useMouseInput = false;
    this.mousePosition = null;
    this.mouseIsDown = false;

    // Animation frame ID
    this.animationFrameId = null;

    // Post-processing (WebGL bloom/glow effects)
    this.postProcessor = null;

    // Settings
    this.settings = {
      projectorWidth: 1280,
      projectorHeight: 720,
      showDebug: true,
      backgroundColor: '#000000',
      // Erase zone settings (in normalized 0-1 coordinates)
      eraseZoneEnabled: false,
      eraseZoneX: 0.0,       // Left edge (0-1)
      eraseZoneY: 0.0,       // Top edge (0-1)
      eraseZoneWidth: 0.15,  // Width (0-1)
      eraseZoneHeight: 0.15  // Height (0-1)
    };

    // Callbacks for UI updates
    this.onStateChange = null;
  }

  /**
   * Initialize the application
   * @param {Object} elements - DOM element references
   */
  async init(elements) {
    console.log('AppController initializing...');

    // Store canvas references
    this.projectorCanvas = elements.projectorCanvas;
    this.debugCanvas = elements.debugCanvas;
    this.videoElement = elements.videoElement;

    // Get canvas contexts
    this.projectorCtx = this.projectorCanvas.getContext('2d');
    this.debugCtx = this.debugCanvas.getContext('2d');

    // Set canvas sizes
    this.resizeCanvases();

    // Create capture canvas for camera frames
    this.captureCanvas = document.createElement('canvas');
    this.captureCtx = this.captureCanvas.getContext('2d', { willReadFrequently: true });

    // Initialize camera
    await this.camera.init(this.videoElement, {
      width: 640,
      height: 480,
      frameRate: 60
    });

    // Set capture canvas to camera dimensions
    this.captureCanvas.width = this.camera.width;
    this.captureCanvas.height = this.camera.height;

    // Set debug canvas to match camera resolution
    this.debugCanvas.width = this.camera.width;
    this.debugCanvas.height = this.camera.height;

    console.log(`Camera resolution: ${this.camera.width}x${this.camera.height}`);

    // Initialize tracker
    this.tracker.init(this.camera.width, this.camera.height);

    // Initialize calibration managers
    this.cameraCalibration.init(
      this.camera.width,
      this.camera.height,
      this.projectorCanvas.width,
      this.projectorCanvas.height
    );
    this.cameraCalibration.onStateChange = (key, value) => this.notifyStateChange(key, value);
    this.projectorCalibration.onStateChange = (key, value) => this.notifyStateChange(key, value);

    // Initialize brush manager
    this.brushManager.init(this.projectorCanvas.width, this.projectorCanvas.height);
    this.brushManager.onStateChange = (key, value) => this.notifyStateChange(key, value);

    // Initialize post-processor for bloom/glow effects
    this.postProcessor = new PostProcessor();
    const ppInitialized = this.postProcessor.init(
      this.projectorCanvas.width,
      this.projectorCanvas.height
    );
    if (ppInitialized) {
      console.log('WebGL post-processor initialized');
    } else {
      console.warn('WebGL post-processor unavailable, bloom effects disabled');
      this.postProcessor = null;
    }

    // Configure rendering pipeline
    this.renderingPipeline.configure({
      projectorCanvas: this.projectorCanvas,
      projectorCtx: this.projectorCtx,
      brushManager: this.brushManager,
      postProcessor: this.postProcessor,
      cameraCalibration: this.cameraCalibration,
      projectorCalibration: this.projectorCalibration,
      tracker: this.tracker,
      settings: this.settings
    });

    // Set up clone canvas if available (mirrors popup in main window)
    if (elements.projectorCloneCanvas) {
      // Set clone canvas size to match projector canvas
      elements.projectorCloneCanvas.width = this.projectorCanvas.width;
      elements.projectorCloneCanvas.height = this.projectorCanvas.height;
      this.renderingPipeline.setCloneCanvas(elements.projectorCloneCanvas);
      this.projectorCloneCanvas = elements.projectorCloneCanvas;
    }

    // Set up warped camera canvas if available (perspective-corrected camera preview)
    if (elements.warpedCameraCanvas) {
      elements.warpedCameraCanvas.width = this.camera.width;
      elements.warpedCameraCanvas.height = this.camera.height;
      this.renderingPipeline.setWarpedCameraCanvas(elements.warpedCameraCanvas);
      this.warpedCameraCanvas = elements.warpedCameraCanvas;
    }

    // Give rendering pipeline access to debug canvas for warped camera preview
    this.renderingPipeline.setDebugCanvas(this.debugCanvas);

    console.log('AppController initialized');
    return true;
  }

  /**
   * Get the currently active brush
   * @returns {BaseBrush}
   */
  getActiveBrush() {
    return this.brushManager.getActiveBrush();
  }

  /**
   * Set active brush by index
   * @param {number} index - Brush index
   */
  setActiveBrush(index) {
    this.brushManager.setActiveBrush(index);
  }

  /**
   * Get list of available brushes
   * @returns {Array}
   */
  getBrushList() {
    return this.brushManager.getBrushList();
  }

  /**
   * Get brushes array (for backwards compatibility)
   * @returns {Array}
   */
  get brushes() {
    return this.brushManager.brushes;
  }

  /**
   * Get projector popup reference (delegates to rendering pipeline)
   * @returns {Object|null}
   */
  get projectorPopup() {
    return this.renderingPipeline.getProjectorPopup();
  }

  /**
   * Set projector popup reference (delegates to rendering pipeline)
   * @param {Object|null} popup
   */
  set projectorPopup(popup) {
    this.renderingPipeline.setProjectorPopup(popup);
  }

  /**
   * Resize canvases to fit container
   */
  resizeCanvases() {
    const container = this.projectorCanvas.parentElement;
    if (!container) return;

    // Get container dimensions
    const rect = container.getBoundingClientRect();

    // Skip if container has no dimensions yet
    if (rect.width === 0 || rect.height === 0) return;

    // Set projector canvas to fill container
    this.projectorCanvas.width = rect.width;
    this.projectorCanvas.height = rect.height;

    // Set debug canvas to match camera aspect ratio
    // CSS sizes it to 320x240, but internal resolution should match camera
    if (this.camera && this.camera.width) {
      this.debugCanvas.width = this.camera.width;
      this.debugCanvas.height = this.camera.height;
    } else {
      // Fallback to CSS size if camera not ready
      const debugRect = this.debugCanvas.getBoundingClientRect();
      this.debugCanvas.width = debugRect.width * window.devicePixelRatio;
      this.debugCanvas.height = debugRect.height * window.devicePixelRatio;
    }

    // Update brush canvas sizes
    this.brushManager.resize(rect.width, rect.height);

    // Update calibration manager dimensions
    this.cameraCalibration.updateProjectorDimensions(rect.width, rect.height);

    // Update rendering pipeline (handles post-processor resize)
    this.renderingPipeline.resize(rect.width, rect.height);

    // Resize clone canvas to match projector canvas
    if (this.projectorCloneCanvas) {
      const cloneContainer = this.projectorCloneCanvas.parentElement;
      if (cloneContainer) {
        const cloneRect = cloneContainer.getBoundingClientRect();
        if (cloneRect.width > 0 && cloneRect.height > 0) {
          this.projectorCloneCanvas.width = cloneRect.width;
          this.projectorCloneCanvas.height = cloneRect.height;
        }
      }
    }

    // Resize warped camera canvas
    if (this.warpedCameraCanvas) {
      const warpedContainer = this.warpedCameraCanvas.parentElement;
      if (warpedContainer) {
        const warpedRect = warpedContainer.getBoundingClientRect();
        if (warpedRect.width > 0 && warpedRect.height > 0) {
          this.warpedCameraCanvas.width = warpedRect.width;
          this.warpedCameraCanvas.height = warpedRect.height;
        }
      }
    }
  }

  /**
   * Start the main loop
   */
  start() {
    if (this.isRunning) return;

    this.isRunning = true;
    this.lastFpsUpdate = performance.now();
    this.frameCount = 0;

    this.mainLoop();
    console.log('AppController started');
  }

  /**
   * Stop the main loop
   */
  stop() {
    this.isRunning = false;

    if (this.animationFrameId) {
      cancelAnimationFrame(this.animationFrameId);
      this.animationFrameId = null;
    }

    console.log('AppController stopped');
  }

  /**
   * Main application loop
   */
  mainLoop() {
    if (!this.isRunning) return;

    // Process tracking
    this.processTracking();

    // Update painting
    this.updatePainting();

    // Render output
    this.render();

    // Update FPS
    this.updateFps();

    // Schedule next frame
    this.animationFrameId = requestAnimationFrame(() => this.mainLoop());
  }

  /**
   * Process camera frame and track laser
   */
  processTracking() {
    if (!this.camera.isReady) return;

    // Capture frame from camera
    const imageData = this.camera.getFrame(this.captureCtx);
    if (!imageData) return;

    // Update tracker ROI from camera calibration quad
    // During calibration, update every frame (quad corners are being dragged).
    // Otherwise, update only once after init or when calibration ends.
    if (this.cameraCalibration.isCalibrating || !this._roiInitialized) {
      this.tracker.setROI(this.cameraCalibration.getSourceQuad());
      this._roiInitialized = true;
      this._roiNeedsUpdate = true;
    } else if (this._roiNeedsUpdate) {
      // One final update after calibration mode exits
      this.tracker.setROI(this.cameraCalibration.getSourceQuad());
      this._roiNeedsUpdate = false;
    }

    // Process frame with tracker
    this.tracker.processFrame(imageData);

    // Check erase zone if enabled and tracking
    if (this.settings.eraseZoneEnabled && this.tracker.isTracking && this.tracker.currentPosition) {
      const normX = this.tracker.currentPosition.x / this.camera.width;
      const normY = this.tracker.currentPosition.y / this.camera.height;

      // Check if laser is in erase zone
      const ez = this.settings;
      if (normX >= ez.eraseZoneX && normX <= ez.eraseZoneX + ez.eraseZoneWidth &&
          normY >= ez.eraseZoneY && normY <= ez.eraseZoneY + ez.eraseZoneHeight) {
        this.clearCanvas();
        console.log('Erase zone triggered!');
      }
    }

    // Draw debug view if enabled
    if (this.settings.showDebug && this.debugCtx) {
      this.tracker.drawDebug(this.debugCtx, imageData);

      // Draw calibration quad on debug canvas
      this.cameraCalibration.draw(
        this.debugCtx,
        this.camera.width,
        this.camera.height,
        this.debugCanvas.width,
        this.debugCanvas.height
      );

      // Draw erase zone rectangle if enabled
      if (this.settings.eraseZoneEnabled) {
        const ctx = this.debugCtx;
        const ez = this.settings;
        const x = ez.eraseZoneX * this.debugCanvas.width;
        const y = ez.eraseZoneY * this.debugCanvas.height;
        const w = ez.eraseZoneWidth * this.debugCanvas.width;
        const h = ez.eraseZoneHeight * this.debugCanvas.height;

        ctx.strokeStyle = '#ff0000';
        ctx.lineWidth = 2;
        ctx.strokeRect(x, y, w, h);

        ctx.fillStyle = 'rgba(255, 0, 0, 0.1)';
        ctx.fillRect(x, y, w, h);

        ctx.fillStyle = '#ff0000';
        ctx.font = '12px monospace';
        ctx.fillText('ERASE', x + 4, y + 14);
      }
    }

    // Update status
    this.notifyStateChange('tracking', this.tracker.isTracking);
    if (this.tracker.currentPosition) {
      this.notifyStateChange('position', {
        x: Math.round(this.tracker.currentPosition.x),
        y: Math.round(this.tracker.currentPosition.y)
      });
    }
  }

  /**
   * Update painting based on tracking or mouse input
   */
  updatePainting() {
    const brush = this.getActiveBrush();

    // Mouse input mode (when mouse is down)
    if (this.useMouseInput && this.mouseIsDown && this.mousePosition) {
      // Use mouse position directly (already normalized 0-1)
      brush.addPoint(this.mousePosition.x, this.mousePosition.y, this._mouseIsNewStroke);
      this._mouseIsNewStroke = false;
      return;  // Mouse takes priority when actively drawing
    }

    // Laser tracking mode (works alongside mouse when mouse is not pressed)
    if (!this.tracker.isTracking) {
      // Only end stroke if we're not waiting for mouse input
      if (brush.isDrawing && !(this.useMouseInput && !this.mouseIsDown)) {
        brush.endStroke();
      }
      return;
    }

    // Get normalized position from tracker
    const normPos = this.tracker.getNormalizedPosition();
    if (!normPos) return;

    // Ignore detections outside the camera calibration area
    if (!this.cameraCalibration.isInsideCalibrationArea(
      this.tracker.currentPosition.x,
      this.tracker.currentPosition.y
    )) {
      return;
    }

    // Transform through calibration
    const transformed = this.cameraCalibration.transform(
      this.tracker.currentPosition.x,
      this.tracker.currentPosition.y
    );

    // Normalize to 0-1 range for brush
    const finalPos = {
      x: transformed.x / this.projectorCanvas.width,
      y: transformed.y / this.projectorCanvas.height
    };

    // Clamp to valid range
    finalPos.x = Math.max(0, Math.min(1, finalPos.x));
    finalPos.y = Math.max(0, Math.min(1, finalPos.y));

    brush.addPoint(finalPos.x, finalPos.y, this.tracker.isNewStroke);
  }

  /**
   * Handle mouse down on projector canvas
   */
  handleMouseDown(e) {
    if (!this.useMouseInput) return;

    const rect = this.projectorCanvas.getBoundingClientRect();
    this.mousePosition = {
      x: (e.clientX - rect.left) / rect.width,
      y: (e.clientY - rect.top) / rect.height
    };
    this.mouseIsDown = true;
    this._mouseIsNewStroke = true;
  }

  /**
   * Handle mouse move on projector canvas
   */
  handleMouseMove(e) {
    if (!this.useMouseInput || !this.mouseIsDown) return;

    const rect = this.projectorCanvas.getBoundingClientRect();
    this.mousePosition = {
      x: (e.clientX - rect.left) / rect.width,
      y: (e.clientY - rect.top) / rect.height
    };
  }

  /**
   * Handle mouse up
   */
  handleMouseUp() {
    this.mouseIsDown = false;
  }

  /**
   * Toggle mouse input mode
   */
  toggleMouseInput() {
    this.useMouseInput = !this.useMouseInput;
    this.notifyStateChange('mouseInput', this.useMouseInput);
    return this.useMouseInput;
  }

  /**
   * Render the output
   */
  render() {
    this.renderingPipeline.render();
    // Draw projector calibration overlay after popup has copied content
    this.renderingPipeline.drawProjectorCalibrationOverlay();
  }

  /**
   * Update FPS counter
   */
  updateFps() {
    this.frameCount++;
    const now = performance.now();
    const elapsed = now - this.lastFpsUpdate;

    if (elapsed >= 1000) {
      this.fps = Math.round(this.frameCount * 1000 / elapsed);
      this.frameCount = 0;
      this.lastFpsUpdate = now;
      this.notifyStateChange('fps', this.fps);
    }
  }

  /**
   * Toggle calibration mode
   */
  toggleCalibration() {
    // Turn off projector calibration if on
    if (this.projectorCalibration.isCalibrating) {
      this.projectorCalibration.toggle();
    }
    return this.cameraCalibration.toggle();
  }

  /**
   * Check if camera calibration is active
   * @returns {boolean}
   */
  get isCalibrating() {
    return this.cameraCalibration.isCalibrating;
  }

  /**
   * Handle calibration point selection
   * @param {number} x - Click X in debug canvas coordinates
   * @param {number} y - Click Y in debug canvas coordinates
   */
  selectCalibrationPoint(x, y) {
    return this.cameraCalibration.selectPoint(
      x, y, this.debugCanvas, this.camera.width, this.camera.height
    );
  }

  /**
   * Move a calibration point
   * @param {number} pointIndex - Point index
   * @param {number} x - New X in debug canvas CSS coordinates
   * @param {number} y - New Y in debug canvas CSS coordinates
   */
  moveCalibrationPoint(pointIndex, x, y) {
    this.cameraCalibration.movePoint(
      pointIndex, x, y, this.debugCanvas, this.camera.width, this.camera.height
    );
  }

  /**
   * Save current camera calibration
   */
  saveCalibration() {
    this.cameraCalibration.save();
  }

  /**
   * Reset camera calibration to defaults
   */
  resetCalibration() {
    this.cameraCalibration.reset(this.camera.width, this.camera.height);
  }

  // =========================================
  // Projector Calibration (Output Warping)
  // =========================================

  /**
   * Toggle projector calibration mode
   */
  toggleProjectorCalibration() {
    // Turn off camera calibration if on
    if (this.cameraCalibration.isCalibrating) {
      this.cameraCalibration.toggle();
    }
    return this.projectorCalibration.toggle();
  }

  /**
   * Check if projector calibration is active
   * @returns {boolean}
   */
  get isProjectorCalibrating() {
    return this.projectorCalibration.isCalibrating;
  }

  /**
   * Get projector quad (for GUI compatibility)
   * @returns {Array<{x: number, y: number}>}
   */
  get projectorQuad() {
    return this.projectorCalibration.getQuad();
  }

  /**
   * Get selected projector point index
   * @returns {number}
   */
  get projectorSelectedPoint() {
    return this.projectorCalibration.selectedPoint;
  }

  /**
   * Set selected projector point index
   * @param {number} value
   */
  set projectorSelectedPoint(value) {
    this.projectorCalibration.selectedPoint = value;
  }

  /**
   * Select projector calibration point at given coordinates
   * @param {number} clientX - Client X coordinate
   * @param {number} clientY - Client Y coordinate
   * @param {HTMLCanvasElement} [canvas] - Optional canvas
   */
  selectProjectorPoint(clientX, clientY, canvas) {
    const targetCanvas = canvas || this.projectorCanvas;
    return this.projectorCalibration.selectPoint(clientX, clientY, targetCanvas);
  }

  /**
   * Move selected projector calibration point
   * @param {number} pointIndex - Point index (0-3)
   * @param {number} clientX - Client X coordinate
   * @param {number} clientY - Client Y coordinate
   * @param {HTMLCanvasElement} [canvas] - Optional canvas
   */
  moveProjectorPoint(pointIndex, clientX, clientY, canvas) {
    const targetCanvas = canvas || this.projectorCanvas;
    this.projectorCalibration.movePoint(pointIndex, clientX, clientY, targetCanvas);
  }

  /**
   * Save projector calibration to localStorage
   */
  saveProjectorCalibration() {
    this.projectorCalibration.save();
  }

  /**
   * Reset projector calibration to full canvas
   */
  resetProjectorCalibration() {
    this.projectorCalibration.reset();
  }

  /**
   * Clear the canvas
   */
  clearCanvas() {
    this.brushManager.clearAll();
  }

  /**
   * Undo last stroke on active brush
   */
  undo() {
    this.brushManager.undo();
  }

  /**
   * Set brush color
   * @param {string} hexColor - Hex color string
   */
  setBrushColor(hexColor) {
    this.brushManager.setColor(hexColor);
  }

  /**
   * Set brush width
   * @param {number} width - Width in pixels
   */
  setBrushWidth(width) {
    this.brushManager.setWidth(width);
  }

  /**
   * Get the active brush
   * @returns {BaseBrush}
   */
  getActiveBrush() {
    return this.brushManager.getActiveBrush();
  }

  /**
   * Set active brush/mode by index
   * @param {number} index - Mode index (0-based)
   */
  setActiveBrush(index) {
    this.brushManager.setActiveBrush(index);
  }

  /**
   * Set mode by index
   * @param {number} index - Mode index (0-based)
   */
  setModeByIndex(index) {
    this.brushManager.setModeByIndex(index);
  }

  /**
   * Set mode by name
   * @param {string} modeName - Mode name
   */
  setMode(modeName) {
    this.brushManager.setMode(modeName);
  }

  /**
   * Get current mode name
   * @returns {string}
   */
  getMode() {
    return this.brushManager.getMode();
  }

  /**
   * Get tracker parameters
   * @returns {Object}
   */
  getTrackerParams() {
    return { ...this.tracker.params };
  }

  /**
   * Update tracker parameters
   * @param {Object} params - New parameters
   */
  setTrackerParams(params) {
    this.tracker.setParams(params);
  }

  /**
   * Notify UI of state changes
   */
  notifyStateChange(key, value) {
    if (this.onStateChange) {
      this.onStateChange(key, value);
    }
  }

  /**
   * Sample a pixel from the camera capture and return its HSV value (OpenCV format)
   * @param {number} x - X in camera pixel coordinates
   * @param {number} y - Y in camera pixel coordinates
   * @returns {{h: number, s: number, v: number}|null} HSV values (H: 0-180, S: 0-255, V: 0-255)
   */
  samplePixelHSV(x, y) {
    if (!this.captureCanvas || !this.captureCtx) return null;

    const cx = Math.round(Math.max(0, Math.min(x, this.captureCanvas.width - 1)));
    const cy = Math.round(Math.max(0, Math.min(y, this.captureCanvas.height - 1)));

    const pixel = this.captureCtx.getImageData(cx, cy, 1, 1).data;
    const r = pixel[0], g = pixel[1], b = pixel[2];

    // Convert RGB to HSV (OpenCV convention: H 0-180, S 0-255, V 0-255)
    const rn = r / 255, gn = g / 255, bn = b / 255;
    const max = Math.max(rn, gn, bn);
    const min = Math.min(rn, gn, bn);
    const delta = max - min;

    let h = 0;
    if (delta > 0) {
      if (max === rn) h = 60 * (((gn - bn) / delta) % 6);
      else if (max === gn) h = 60 * ((bn - rn) / delta + 2);
      else h = 60 * ((rn - gn) / delta + 4);
    }
    if (h < 0) h += 360;

    const s = max === 0 ? 0 : delta / max;
    const v = max;

    return {
      h: Math.round(h / 2),     // 0-360 → 0-180 (OpenCV)
      s: Math.round(s * 255),
      v: Math.round(v * 255)
    };
  }

  /**
   * Handle window resize
   */
  handleResize() {
    this.resizeCanvases();
  }

  /**
   * Clean up resources
   */
  dispose() {
    this.stop();
    this.camera.stop();
    this.tracker.dispose();
    this.brushManager.dispose();
  }
}
