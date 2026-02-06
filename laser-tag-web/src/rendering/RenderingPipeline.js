/**
 * RenderingPipeline - Manages all rendering operations
 * Extracted from AppController to separate visual concerns
 *
 * Responsibilities:
 * - Main canvas rendering
 * - Post-processing (bloom effects)
 * - Popup window sync
 * - Calibration overlay drawing
 * - Warped camera preview
 */
export class RenderingPipeline {
  constructor() {
    // Canvas references
    this.projectorCanvas = null;
    this.projectorCtx = null;

    // Clone canvas (mirrors popup in main window)
    this.cloneCanvas = null;
    this.cloneCtx = null;

    // Warped camera canvas (perspective-corrected camera view)
    this.warpedCameraCanvas = null;
    this.warpedCameraCtx = null;

    // Debug canvas reference (for reading camera frame)
    this.debugCanvas = null;

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

    // Throttle secondary canvas rendering (every N frames)
    this.frameCount = 0;
    this.secondaryRenderInterval = 2; // Render clone/popup every 2nd frame
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
   * @param {HTMLCanvasElement} canvas
   */
  setCloneCanvas(canvas) {
    this.cloneCanvas = canvas;
    this.cloneCtx = canvas ? canvas.getContext('2d') : null;
  }

  /**
   * Set warped camera canvas reference
   * @param {HTMLCanvasElement} canvas
   */
  setWarpedCameraCanvas(canvas) {
    this.warpedCameraCanvas = canvas;
    this.warpedCameraCtx = canvas ? canvas.getContext('2d') : null;
  }

  /**
   * Set debug canvas reference (camera feed source for warped preview)
   * @param {HTMLCanvasElement} canvas
   */
  setDebugCanvas(canvas) {
    this.debugCanvas = canvas;
  }

  /**
   * Set projector popup window reference
   * @param {Object|null} popup - {window, canvas, ctx}
   */
  setProjectorPopup(popup) {
    this.projectorPopup = popup;
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

    // Throttle secondary canvas rendering for performance
    // But always render during calibration for responsive UI
    this.frameCount++;
    const isCalibrating = this.projectorCalibration.isCalibrating;
    const isCameraCalibrating = this.cameraCalibration.isCalibrating;
    if (isCalibrating || isCameraCalibrating || this.frameCount % this.secondaryRenderInterval === 0) {
      this.renderWarpedCamera();
      this.renderCloneCanvas();
      this.renderPopupWindow();
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
   * Shows the warped projection output with calibration overlay
   */
  renderCloneCanvas() {
    if (!this.cloneCanvas || !this.cloneCtx) {
      return;
    }

    const ctx = this.cloneCtx;
    const srcCanvas = this.projectorCanvas;
    const w = this.cloneCanvas.width;
    const h = this.cloneCanvas.height;

    // Clear the clone canvas
    ctx.fillStyle = this.settings.backgroundColor;
    ctx.fillRect(0, 0, w, h);

    const isWarped = this.projectorCalibration.isWarped();
    const isCalibrating = this.projectorCalibration.isCalibrating;

    if (isWarped && !isCalibrating) {
      // Apply CSS transform for warping
      const cssMatrix = this.projectorCalibration.getCssTransform(w, h);
      this.cloneCanvas.style.transformOrigin = '0 0';
      this.cloneCanvas.style.transform = cssMatrix;
      ctx.drawImage(srcCanvas, 0, 0, w, h);
    } else if (isCalibrating) {
      // During calibration, show warped preview with checkerboard
      const cssMatrix = this.projectorCalibration.getCssTransform(w, h, true);
      this.cloneCanvas.style.transformOrigin = '0 0';
      this.cloneCanvas.style.transform = cssMatrix !== 'none' ? cssMatrix : 'none';

      ctx.drawImage(srcCanvas, 0, 0, w, h);

      // Draw checkerboard AFTER content so it's visible on top
      // Use clipped version to constrain to calibration quad
      if (this.projectorCalibration.showCheckerboard) {
        const quad = this.projectorCalibration.getQuad();
        const scaledPoints = quad.map(p => ({ x: p.x * w, y: p.y * h }));
        this.projectorCalibration.drawCheckerboard(ctx, w, h, scaledPoints);
      }
    } else {
      // No warping - reset transform and draw normally
      this.cloneCanvas.style.transform = 'none';
      ctx.drawImage(srcCanvas, 0, 0, w, h);
    }

    // Draw calibration overlay (frame and handles) when calibrating
    // Note: This will be visually transformed by CSS along with the content,
    // showing how the calibration frame appears on the actual projection
    if (isCalibrating) {
      this.projectorCalibration.draw(ctx, w, h, true); // skipCheckerboard=true (already drawn above)
    }
  }

  /**
   * Render warped camera preview (perspective-corrected camera view)
   * Uses grid-based subdivision to un-warp the calibration quad into a rectangle,
   * replicating the C++ warpIntoMe() pattern.
   *
   * Canvas 2D doesn't support perspective transforms natively, so we subdivide
   * the quad into a fine grid of small cells, each drawn with an affine transform.
   * At sufficient subdivision (N=8), the piecewise-affine approximation is visually
   * indistinguishable from a true perspective warp.
   */
  renderWarpedCamera() {
    if (!this.warpedCameraCanvas || !this.warpedCameraCtx || !this.debugCanvas) {
      return;
    }

    const ctx = this.warpedCameraCtx;
    const w = this.warpedCameraCanvas.width;
    const h = this.warpedCameraCanvas.height;

    if (w === 0 || h === 0) return;

    // Ensure no CSS transform is applied (we do everything in canvas)
    this.warpedCameraCanvas.style.transform = 'none';

    // Clear
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, w, h);

    // Get camera calibration source quad (pixel coordinates)
    const srcQuad = this.cameraCalibration.getSourceQuad();
    if (!srcQuad || srcQuad.length !== 4) {
      // No calibration — just draw camera frame as-is
      ctx.drawImage(this.debugCanvas, 0, 0, w, h);
      return;
    }

    const camW = this.debugCanvas.width;
    const camH = this.debugCanvas.height;
    if (camW === 0 || camH === 0) return;

    // Subdivide the quad into NxN cells and draw each with an affine transform
    // srcQuad order: [TL, TR, BR, BL]
    const N = 8;
    const q = srcQuad;

    for (let j = 0; j < N; j++) {
      for (let i = 0; i < N; i++) {
        const u0 = i / N, u1 = (i + 1) / N;
        const v0 = j / N, v1 = (j + 1) / N;

        // Source corners: bilinear interpolation within the calibration quad
        // Maps (u, v) in [0,1]² to camera pixel coordinates
        const s00 = this._bilinearQuad(q, u0, v0);
        const s10 = this._bilinearQuad(q, u1, v0);
        const s01 = this._bilinearQuad(q, u0, v1);
        const s11 = this._bilinearQuad(q, u1, v1);

        // Destination corners: regular grid on output canvas
        const d00x = u0 * w, d00y = v0 * h;
        const d10x = u1 * w, d10y = v0 * h;
        const d01x = u0 * w, d01y = v1 * h;
        const d11x = u1 * w, d11y = v1 * h;

        // Draw two triangles per cell
        this._drawTexturedTriangle(ctx, this.debugCanvas,
          s00.x, s00.y, s10.x, s10.y, s11.x, s11.y,
          d00x, d00y, d10x, d10y, d11x, d11y);
        this._drawTexturedTriangle(ctx, this.debugCanvas,
          s00.x, s00.y, s11.x, s11.y, s01.x, s01.y,
          d00x, d00y, d11x, d11y, d01x, d01y);
      }
    }
  }

  /**
   * Bilinear interpolation within a quad
   * @param {Array} quad - [TL, TR, BR, BL] corner points
   * @param {number} u - Horizontal parameter (0-1)
   * @param {number} v - Vertical parameter (0-1)
   * @returns {{x: number, y: number}}
   */
  _bilinearQuad(quad, u, v) {
    // Top edge: TL → TR
    const tx = quad[0].x + (quad[1].x - quad[0].x) * u;
    const ty = quad[0].y + (quad[1].y - quad[0].y) * u;
    // Bottom edge: BL → BR
    const bx = quad[3].x + (quad[2].x - quad[3].x) * u;
    const by = quad[3].y + (quad[2].y - quad[3].y) * u;
    // Interpolate vertically
    return {
      x: tx + (bx - tx) * v,
      y: ty + (by - ty) * v
    };
  }

  /**
   * Draw a textured triangle from source image to destination canvas
   * Computes the affine transform mapping the source triangle to the dest triangle,
   * then clips and draws.
   */
  _drawTexturedTriangle(ctx, img,
    s0x, s0y, s1x, s1y, s2x, s2y,
    d0x, d0y, d1x, d1y, d2x, d2y) {

    // Compute affine transform: source triangle → dest triangle
    const denom = (s0x * (s1y - s2y) + s1x * (s2y - s0y) + s2x * (s0y - s1y));
    if (Math.abs(denom) < 1e-10) return; // Degenerate triangle

    const invD = 1 / denom;
    const a = (d0x * (s1y - s2y) + d1x * (s2y - s0y) + d2x * (s0y - s1y)) * invD;
    const b = (d0x * (s2x - s1x) + d1x * (s0x - s2x) + d2x * (s1x - s0x)) * invD;
    const c = (d0y * (s1y - s2y) + d1y * (s2y - s0y) + d2y * (s0y - s1y)) * invD;
    const d = (d0y * (s2x - s1x) + d1y * (s0x - s2x) + d2y * (s1x - s0x)) * invD;
    const tx = d0x - a * s0x - b * s0y;
    const ty = d0y - c * s0x - d * s0y;

    ctx.save();

    // Clip to destination triangle
    ctx.beginPath();
    ctx.moveTo(d0x, d0y);
    ctx.lineTo(d1x, d1y);
    ctx.lineTo(d2x, d2y);
    ctx.closePath();
    ctx.clip();

    // Set transform: maps source image coords → dest canvas coords
    ctx.setTransform(a, c, b, d, tx, ty);
    ctx.drawImage(img, 0, 0);

    ctx.restore();
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

    // Get fresh context references (helps after fullscreen changes)
    const popupCtx = popupCanvas.getContext('2d');

    const w = popupCanvas.width;
    const h = popupCanvas.height;

    // Clear the popup canvas
    popupCtx.fillStyle = this.settings.backgroundColor;
    popupCtx.fillRect(0, 0, w, h);

    // Clear and prepare overlay canvas
    let overlayCtx = null;
    if (overlayCanvas) {
      overlayCtx = overlayCanvas.getContext('2d');
      overlayCtx.clearRect(0, 0, overlayCanvas.width, overlayCanvas.height);
    }

    const isWarped = this.projectorCalibration.isWarped();
    const isCalibrating = this.projectorCalibration.isCalibrating;

    if (isWarped && !isCalibrating) {
      this.renderPopupWarped(popupCtx, popupCanvas, srcCanvas, w, h);
    } else if (isCalibrating) {
      // During calibration, always show warped preview + overlay on separate canvas
      this.renderPopupCalibrating(popupCtx, popupCanvas, srcCanvas, w, h);
      // Draw calibration overlay (frame + handles) on the non-transformed overlay canvas
      if (overlayCtx && overlayCanvas) {
        const ow = overlayCanvas.width;
        const oh = overlayCanvas.height;
        // Draw only frame and handles on overlay (skipCheckerboard=true)
        this.projectorCalibration.draw(overlayCtx, ow, oh, true);
      }
    } else {
      this.renderPopupNormal(popupCtx, popupCanvas, srcCanvas, w, h);
    }
  }

  /**
   * Render popup with perspective warp applied via CSS
   */
  renderPopupWarped(popupCtx, popupCanvas, srcCanvas, w, h) {
    const cssMatrix = this.projectorCalibration.getCssTransform(w, h);

    // Apply CSS transform to the canvas element
    popupCanvas.style.transformOrigin = '0 0';
    popupCanvas.style.transform = cssMatrix;

    // Draw content at full size (CSS will warp it)
    popupCtx.drawImage(srcCanvas, 0, 0, w, h);
  }

  /**
   * Render popup during calibration (with live warp preview)
   * Note: Calibration frame/handles drawn separately on overlay canvas (not transformed)
   */
  renderPopupCalibrating(popupCtx, popupCanvas, srcCanvas, w, h) {
    // Apply CSS transform to show live warp preview (force=true to get transform during calibration)
    const cssMatrix = this.projectorCalibration.getCssTransform(w, h, true);
    popupCanvas.style.transformOrigin = '0 0';
    popupCanvas.style.transform = cssMatrix !== 'none' ? cssMatrix : 'none';

    // Draw content at full size (CSS will warp it)
    popupCtx.drawImage(srcCanvas, 0, 0, w, h);

    // Draw checkerboard AFTER content so it's visible on top
    if (this.projectorCalibration.showCheckerboard) {
      this.projectorCalibration.drawCheckerboardFullscreen(popupCtx, w, h);
    }
  }

  /**
   * Render popup normally with aspect ratio preservation
   * Note: Calibration overlay is drawn separately on overlay canvas
   */
  renderPopupNormal(popupCtx, popupCanvas, srcCanvas, w, h) {
    // Reset any existing transform
    popupCanvas.style.transform = 'none';

    // Calculate aspect-ratio-preserving scale (letterbox/pillarbox)
    const srcAspect = srcCanvas.width / srcCanvas.height;
    const dstAspect = w / h;

    let drawWidth, drawHeight, offsetX, offsetY;

    if (srcAspect > dstAspect) {
      // Source is wider - fit to width, letterbox top/bottom
      drawWidth = w;
      drawHeight = w / srcAspect;
      offsetX = 0;
      offsetY = (h - drawHeight) / 2;
    } else {
      // Source is taller - fit to height, pillarbox left/right
      drawHeight = h;
      drawWidth = h * srcAspect;
      offsetX = (w - drawWidth) / 2;
      offsetY = 0;
    }

    popupCtx.drawImage(srcCanvas, offsetX, offsetY, drawWidth, drawHeight);
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
    this.projectorPopup = null;
  }
}
