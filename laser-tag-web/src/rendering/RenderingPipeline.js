/**
 * RenderingPipeline - Manages all rendering operations
 * Extracted from AppController to separate visual concerns
 *
 * Responsibilities:
 * - Main canvas rendering
 * - Post-processing (bloom effects)
 * - Popup window sync
 * - Calibration overlay drawing
 * - Warped camera preview (WebGL)
 * - Clone/popup perspective warp (WebGL)
 */
import { PerspectiveWarp } from '../effects/PerspectiveWarp.js';

export class RenderingPipeline {
  constructor() {
    // Canvas references
    this.projectorCanvas = null;
    this.projectorCtx = null;

    // Clone canvas (mirrors popup in main window) - WebGL warp
    this.cloneCanvas = null;
    this.cloneOverlayCanvas = null;
    this.cloneOverlayCtx = null;

    // Warped camera canvas (perspective-corrected camera view) - WebGL warp
    this.warpedCameraCanvas = null;

    // Debug canvas reference (for reading camera frame)
    this.debugCanvas = null;

    // Capture canvas reference (raw camera frame without overlays)
    this.captureCanvas = null;

    // WebGL warp instances
    this.cameraWarp = null;
    this.projectorWarp = null;
    this.popupWarp = null;

    // Dependencies (set via configure)
    this.brushManager = null;
    this.postProcessor = null;
    this.cameraCalibration = null;
    this.projectorCalibration = null;
    this.tracker = null;

    // Popup window reference
    this.projectorPopup = null;

    // Settings reference
    this.settings = null;

    // Throttle secondary preview canvases (every N frames)
    this.frameCount = 0;
    this.secondaryRenderInterval = 3; // Render clone/warped camera every 3rd frame
  }

  /**
   * Configure the pipeline with dependencies
   * @param {Object} config - Configuration object
   */
  configure(config) {
    this.projectorCanvas = config.projectorCanvas;
    this.projectorCtx = config.projectorCtx;
    this.brushManager = config.brushManager;
    this.postProcessor = config.postProcessor;
    this.cameraCalibration = config.cameraCalibration;
    this.projectorCalibration = config.projectorCalibration;
    this.tracker = config.tracker;
    this.settings = config.settings;
  }

  /**
   * Set clone canvas reference (mirrors popup in main window)
   * Uses WebGL for perspective warp rendering.
   * @param {HTMLCanvasElement} canvas
   * @param {HTMLCanvasElement} [overlayCanvas] - Overlay for calibration handles (2D)
   */
  setCloneCanvas(canvas, overlayCanvas) {
    this.cloneCanvas = canvas;
    if (canvas) {
      this.projectorWarp = new PerspectiveWarp();
      this.projectorWarp.init(canvas);
    } else {
      if (this.projectorWarp) this.projectorWarp.dispose();
      this.projectorWarp = null;
    }

    this.cloneOverlayCanvas = overlayCanvas || null;
    this.cloneOverlayCtx = overlayCanvas ? overlayCanvas.getContext('2d') : null;
  }

  /**
   * Set warped camera canvas reference
   * Uses WebGL for perspective warp rendering.
   * @param {HTMLCanvasElement} canvas
   */
  setWarpedCameraCanvas(canvas) {
    this.warpedCameraCanvas = canvas;
    if (canvas) {
      this.cameraWarp = new PerspectiveWarp();
      this.cameraWarp.init(canvas);
    } else {
      if (this.cameraWarp) this.cameraWarp.dispose();
      this.cameraWarp = null;
    }
  }

  /**
   * Set debug canvas reference (camera feed with overlays)
   * @param {HTMLCanvasElement} canvas
   */
  setDebugCanvas(canvas) {
    this.debugCanvas = canvas;
  }

  /**
   * Set capture canvas reference (raw camera frame without overlays)
   * @param {HTMLCanvasElement} canvas
   */
  setCaptureCanvas(canvas) {
    this.captureCanvas = canvas;
  }

  /**
   * Set projector popup window reference
   * Initializes WebGL warp on the popup's content canvas.
   * @param {Object|null} popup - {window, canvas, overlayCanvas, overlayCtx, container}
   */
  setProjectorPopup(popup) {
    // Clean up previous popup warp
    if (this.popupWarp) {
      this.popupWarp.dispose();
      this.popupWarp = null;
    }

    this.projectorPopup = popup;

    if (popup && popup.canvas) {
      this.popupWarp = new PerspectiveWarp();
      this.popupWarp.init(popup.canvas);
    }
  }

  /**
   * Get projector popup reference
   * @returns {Object|null}
   */
  getProjectorPopup() {
    return this.projectorPopup;
  }

  /**
   * Render the main output canvas
   */
  render() {
    this.renderMainCanvas();

    // Popup is the primary projector output - render every frame
    this.renderPopupWindow();

    // Throttle secondary preview canvases for performance
    // But always render during calibration for responsive UI
    this.frameCount++;
    const isCalibrating = this.projectorCalibration.isCalibrating;
    const isCameraCalibrating = this.cameraCalibration.isCalibrating;
    if (isCalibrating || isCameraCalibrating || this.frameCount % this.secondaryRenderInterval === 0) {
      this.renderWarpedCamera();
      this.renderCloneCanvas();
    }
  }

  /**
   * Render the main projector canvas
   */
  renderMainCanvas() {
    const ctx = this.projectorCtx;
    const width = this.projectorCanvas.width;
    const height = this.projectorCanvas.height;

    // Clear with background color
    ctx.fillStyle = this.settings.backgroundColor;
    ctx.fillRect(0, 0, width, height);

    // Render and composite all brushes
    this.brushManager.render();
    this.brushManager.draw(ctx);

    // Note: Laser position indicator removed from projection output
    // It's only shown on debug canvas for calibration purposes

    // Apply WebGL post-processing (bloom effect)
    this.applyPostProcessing(ctx, width, height);

    // Note: Projector calibration overlay is drawn separately via drawProjectorCalibrationOverlay()
    // to avoid duplicating it when the popup copies the main canvas
  }

  /**
   * Draw projector calibration overlay on main canvas (called after render)
   * This is separate from render() so the popup can copy content without the overlay
   */
  drawProjectorCalibrationOverlay() {
    if (this.projectorCalibration.isCalibrating) {
      this.projectorCalibration.draw(this.projectorCtx);
    }
  }

  /**
   * Render to clone canvas (mirrors popup in main window)
   * Uses WebGL perspective warp via PerspectiveWarp.
   */
  renderCloneCanvas() {
    if (!this.cloneCanvas || !this.projectorWarp) {
      return;
    }

    const srcCanvas = this.projectorCanvas;
    const w = this.cloneCanvas.width;
    const h = this.cloneCanvas.height;

    // Clear clone overlay
    if (this.cloneOverlayCtx && this.cloneOverlayCanvas) {
      this.cloneOverlayCtx.clearRect(0, 0, this.cloneOverlayCanvas.width, this.cloneOverlayCanvas.height);
    }

    // Reset any CSS transform (WebGL handles warping now)
    this.cloneCanvas.style.transform = 'none';

    const isWarped = this.projectorCalibration.isWarped();
    const isCalibrating = this.projectorCalibration.isCalibrating;

    if (isWarped && !isCalibrating) {
      // Warped: render with perspective warp
      this.projectorWarp.setForwardWarp(this.projectorCalibration.getQuad());
      this.projectorWarp.setCheckerboard(false);
      this.projectorWarp.render(srcCanvas);
    } else if (isCalibrating) {
      // Calibrating: warped preview with optional checkerboard
      this.projectorWarp.setForwardWarp(this.projectorCalibration.getQuad());
      this.projectorWarp.setCheckerboard(
        this.projectorCalibration.showCheckerboard, 10
      );
      this.projectorWarp.render(srcCanvas);

      // Draw calibration overlay (frame and handles) on the overlay canvas
      if (this.cloneOverlayCtx && this.cloneOverlayCanvas) {
        const ow = this.cloneOverlayCanvas.width;
        const oh = this.cloneOverlayCanvas.height;
        this.projectorCalibration.draw(this.cloneOverlayCtx, ow, oh, true);
      }
    } else {
      // No warping - identity warp (passthrough)
      this.projectorWarp.setForwardWarp([
        { x: 0, y: 0 }, { x: 1, y: 0 },
        { x: 1, y: 1 }, { x: 0, y: 1 }
      ]);
      this.projectorWarp.setCheckerboard(false);
      this.projectorWarp.render(srcCanvas);
    }
  }

  /**
   * Render warped camera preview (perspective-corrected camera view)
   * Uses WebGL for single-pass perspective un-warping of the camera quad.
   */
  renderWarpedCamera() {
    // Use captureCanvas (raw camera frame) to avoid debug overlays in preview
    const sourceCanvas = this.captureCanvas || this.debugCanvas;
    if (!this.warpedCameraCanvas || !this.cameraWarp || !sourceCanvas) {
      return;
    }

    const w = this.warpedCameraCanvas.width;
    const h = this.warpedCameraCanvas.height;
    if (w === 0 || h === 0) return;

    // Reset any CSS transform
    this.warpedCameraCanvas.style.transform = 'none';

    // Get camera calibration source quad (pixel coordinates)
    const srcQuad = this.cameraCalibration.getSourceQuad();
    if (!srcQuad || srcQuad.length !== 4) {
      // No calibration — identity warp (show full camera frame)
      this.cameraWarp.setForwardWarp([
        { x: 0, y: 0 }, { x: 1, y: 0 },
        { x: 1, y: 1 }, { x: 0, y: 1 }
      ]);
      this.cameraWarp.setCheckerboard(false);
      this.cameraWarp.render(sourceCanvas);
      return;
    }

    const camW = sourceCanvas.width;
    const camH = sourceCanvas.height;
    if (camW === 0 || camH === 0) return;

    // Set inverse warp: maps camera quad → full rectangle
    this.cameraWarp.setInverseWarp(srcQuad, camW, camH);
    this.cameraWarp.setCheckerboard(false);
    this.cameraWarp.render(sourceCanvas);
  }

  /**
   * Draw laser position indicator on canvas
   * @param {CanvasRenderingContext2D} ctx
   */
  drawLaserIndicator(ctx) {
    if (!this.tracker.isTracking || !this.tracker.currentPosition) {
      return;
    }

    const transformed = this.cameraCalibration.transform(
      this.tracker.currentPosition.x,
      this.tracker.currentPosition.y
    );

    ctx.beginPath();
    ctx.arc(transformed.x, transformed.y, 5, 0, Math.PI * 2);
    ctx.fillStyle = 'rgba(255, 0, 0, 0.5)';
    ctx.fill();
  }

  /**
   * Apply WebGL post-processing (bloom effect)
   * @param {CanvasRenderingContext2D} ctx
   * @param {number} width
   * @param {number} height
   */
  applyPostProcessing(ctx, width, height) {
    if (!this.postProcessor ||
        !this.postProcessor.enabled ||
        !this.postProcessor.params.bloomEnabled) {
      return;
    }

    const processedCanvas = this.postProcessor.process(this.projectorCanvas);

    // Clear and draw processed result (flip Y to correct WebGL orientation)
    ctx.fillStyle = this.settings.backgroundColor;
    ctx.fillRect(0, 0, width, height);
    ctx.save();
    ctx.translate(0, height);
    ctx.scale(1, -1);
    ctx.drawImage(processedCanvas, 0, 0);
    ctx.restore();
  }

  /**
   * Render to popup projector window if open
   */
  renderPopupWindow() {
    if (!this.projectorPopup ||
        !this.projectorPopup.window ||
        this.projectorPopup.window.closed) {
      return;
    }

    const popupCanvas = this.projectorPopup.canvas;
    const overlayCanvas = this.projectorPopup.overlayCanvas;
    const srcCanvas = this.projectorCanvas;
    const overlayCtx = this.projectorPopup.overlayCtx || (overlayCanvas ? overlayCanvas.getContext('2d') : null);

    const w = popupCanvas.width;
    const h = popupCanvas.height;

    // Clear overlay canvas
    if (overlayCtx && overlayCanvas) {
      overlayCtx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
    }

    // Reset CSS transform (WebGL handles warping)
    popupCanvas.style.transform = 'none';

    if (!this.popupWarp) return;

    const isWarped = this.projectorCalibration.isWarped();
    const isCalibrating = this.projectorCalibration.isCalibrating;

    if (isWarped && !isCalibrating) {
      this.renderPopupWarped(popupCanvas, srcCanvas, w, h);
    } else if (isCalibrating) {
      this.renderPopupCalibrating(popupCanvas, srcCanvas, w, h);
      // Draw calibration overlay (frame + handles) on the non-transformed overlay canvas
      if (overlayCtx && overlayCanvas) {
        const ow = overlayCanvas.width;
        const oh = overlayCanvas.height;
        this.projectorCalibration.draw(overlayCtx, ow, oh, true);
      }
    } else {
      this.renderPopupNormal(popupCanvas, srcCanvas, w, h);
    }
  }

  /**
   * Render popup with perspective warp via WebGL
   */
  renderPopupWarped(popupCanvas, srcCanvas, w, h) {
    this.popupWarp.setForwardWarp(this.projectorCalibration.getQuad());
    this.popupWarp.setCheckerboard(false);
    this.popupWarp.render(srcCanvas);
  }

  /**
   * Render popup during calibration (with live warp preview + checkerboard)
   */
  renderPopupCalibrating(popupCanvas, srcCanvas, w, h) {
    this.popupWarp.setForwardWarp(this.projectorCalibration.getQuad());
    this.popupWarp.setCheckerboard(
      this.projectorCalibration.showCheckerboard, 10
    );
    this.popupWarp.render(srcCanvas);
  }

  /**
   * Render popup normally (no warp, identity passthrough)
   */
  renderPopupNormal(popupCanvas, srcCanvas, w, h) {
    // Identity warp - just renders source to fill canvas
    this.popupWarp.setForwardWarp([
      { x: 0, y: 0 }, { x: 1, y: 0 },
      { x: 1, y: 1 }, { x: 0, y: 1 }
    ]);
    this.popupWarp.setCheckerboard(false);
    this.popupWarp.render(srcCanvas);
  }

  /**
   * Resize post-processor if needed
   * @param {number} width
   * @param {number} height
   */
  resize(width, height) {
    if (this.postProcessor && this.postProcessor.enabled) {
      this.postProcessor.resize(width, height);
    }
  }

  /**
   * Dispose resources
   */
  dispose() {
    if (this.cameraWarp) { this.cameraWarp.dispose(); this.cameraWarp = null; }
    if (this.projectorWarp) { this.projectorWarp.dispose(); this.projectorWarp = null; }
    if (this.popupWarp) { this.popupWarp.dispose(); this.popupWarp = null; }
    this.projectorPopup = null;
  }
}
