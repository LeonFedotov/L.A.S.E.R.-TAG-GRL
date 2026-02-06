/**
 * LaserTracker - Detects and tracks laser pointer position using OpenCV.js
 * Enhanced with Kalman filtering, optical flow, and CAMShift adaptive tracking
 * Based on the original laserTracking.cpp from L.A.S.E.R. TAG
 */
import { KalmanFilter } from 'kalman-filter';

export class LaserTracker {
  constructor() {
    // Tracking state
    this.isTracking = false;
    this.lastPosition = null;
    this.currentPosition = null;
    this.predictedPosition = null;
    this.velocity = { x: 0, y: 0 };
    this.isNewStroke = false;
    this.framesSinceLastDetection = 0;

    // OpenCV matrices (lazy initialized)
    this.srcMat = null;
    this.hsvMat = null;
    this.maskMat = null;
    this.morphKernel = null;

    // Optical flow matrices
    this.prevGray = null;
    this.currGray = null;
    this.prevPoints = null;

    // CAMShift tracking
    this.trackWindow = null;
    this.roiHist = null;
    this.camshiftEnabled = false;

    // Kalman filter for position smoothing and prediction
    this.kalmanFilter = null;
    this.kalmanState = null;

    // Tracking parameters (configurable via GUI)
    this.params = {
      // HSV range for laser detection (default: green laser)
      hueMin: 35,
      hueMax: 85,
      satMin: 50,
      satMax: 255,
      valMin: 200,
      valMax: 255,

      // Blob detection parameters
      minBlobArea: 10,
      maxBlobArea: 5000,

      // Tracking parameters
      smoothing: 0.5,           // Position smoothing (0-1)
      newStrokeThreshold: 10,   // Frames without detection to trigger new stroke
      maxVelocity: 100,         // Max pixels per frame (filters noise)

      // Advanced tracking options
      useKalman: true,          // Use Kalman filter for smoothing
      useOpticalFlow: true,     // Use optical flow for prediction
      useCamshift: false,       // Use CAMShift for adaptive tracking

      // Debug
      showDebug: true
    };

    // ROI mask (limits detection to calibration quad area)
    this.roiMask = null;

    // Performance tracking
    this.processTime = 0;
  }

  /**
   * Initialize OpenCV matrices and Kalman filter
   * @param {number} width - Frame width
   * @param {number} height - Frame height
   */
  init(width, height) {
    if (typeof cv === 'undefined') {
      throw new Error('OpenCV.js not loaded');
    }

    // Check if this is a resolution change (reinit)
    const isReinit = this.width > 0 && this.height > 0;

    this.width = width;
    this.height = height;

    // Clean up old matrices if they exist
    if (isReinit) {
      this.cleanupMatrices();
    }

    // Create reusable matrices
    this.srcMat = new cv.Mat(height, width, cv.CV_8UC4);
    this.hsvMat = new cv.Mat();
    this.maskMat = new cv.Mat();
    this.morphKernel = cv.getStructuringElement(cv.MORPH_ELLIPSE, new cv.Size(5, 5));

    // Initialize optical flow matrices
    this.prevGray = new cv.Mat();
    this.currGray = new cv.Mat();

    // Reset all tracking state on resolution change
    this.resetTrackingState();

    // Initialize Kalman filter for 2D position + velocity tracking
    this.initKalmanFilter();

    console.log('LaserTracker initialized with Kalman filter, optical flow, and CAMShift support');
  }

  /**
   * Clean up OpenCV matrices
   */
  cleanupMatrices() {
    try {
      if (this.srcMat && !this.srcMat.isDeleted()) this.srcMat.delete();
      if (this.hsvMat && !this.hsvMat.isDeleted()) this.hsvMat.delete();
      if (this.maskMat && !this.maskMat.isDeleted()) this.maskMat.delete();
      if (this.morphKernel && !this.morphKernel.isDeleted()) this.morphKernel.delete();
      if (this.prevGray && !this.prevGray.isDeleted()) this.prevGray.delete();
      if (this.currGray && !this.currGray.isDeleted()) this.currGray.delete();
      if (this.roiHist && !this.roiHist.isDeleted()) this.roiHist.delete();
    } catch (e) {
      console.warn('Error cleaning up matrices:', e);
    }
  }

  /**
   * Reset all tracking state (call on resolution change)
   */
  resetTrackingState() {
    // Reset position tracking
    this.currentPosition = null;
    this.lastPosition = null;
    this.predictedPosition = null;
    this.velocity = { x: 0, y: 0 };
    this.isTracking = false;
    this.isNewStroke = true;
    this.framesSinceLastDetection = 0;

    // Reset Kalman state
    this.kalmanState = null;

    // Reset CAMShift state
    this.trackWindow = null;
    this.roiHist = null;
    this.camshiftEnabled = false;

    console.log('Tracking state reset');
  }

  /**
   * Initialize Kalman filter for position/velocity estimation
   */
  initKalmanFilter() {
    // State: [x, vx, y, vy] - position and velocity in x and y
    // Observation: [x, y] - we only observe position
    this.kalmanFilter = new KalmanFilter({
      observation: {
        dimension: 2,
        stateProjection: [
          [1, 0, 0, 0],  // x
          [0, 0, 1, 0]   // y
        ],
        covariance: [10, 10]  // Measurement noise
      },
      dynamic: {
        dimension: 4,
        transition: [
          [1, 1, 0, 0],  // x = x + vx
          [0, 1, 0, 0],  // vx = vx
          [0, 0, 1, 1],  // y = y + vy
          [0, 0, 0, 1]   // vy = vy
        ],
        covariance: [1, 1, 1, 1]  // Process noise
      }
    });

    this.kalmanState = null;
  }

  /**
   * Process a video frame and detect laser position
   * @param {ImageData} imageData - Frame from camera
   * @returns {Object|null} - Detected position {x, y} or null
   */
  processFrame(imageData) {
    if (!this.srcMat) {
      throw new Error('LaserTracker not initialized');
    }

    const startTime = performance.now();

    try {
      // Load image data into OpenCV matrix
      this.srcMat.data.set(imageData.data);

      // Convert RGBA to HSV
      cv.cvtColor(this.srcMat, this.hsvMat, cv.COLOR_RGBA2RGB);
      cv.cvtColor(this.hsvMat, this.hsvMat, cv.COLOR_RGB2HSV);

      // Store previous gray frame for optical flow
      if (this.params.useOpticalFlow && this.currGray && !this.currGray.empty()) {
        this.currGray.copyTo(this.prevGray);
      }

      // Convert to grayscale for optical flow
      if (this.params.useOpticalFlow) {
        const rgbMat = new cv.Mat();
        cv.cvtColor(this.srcMat, rgbMat, cv.COLOR_RGBA2GRAY);
        rgbMat.copyTo(this.currGray);
        rgbMat.delete();
      }

      // Primary detection: HSV color thresholding
      let detectedPosition = this.detectByColor();

      // Note: We intentionally do NOT use optical flow to predict position when
      // detection is lost. For laser tagging, when the artist turns off the laser,
      // drawing should stop immediately to allow non-continuous lines.
      // Optical flow is only used to refine tracking when detection IS present.

      // If using CAMShift and we have a track window, refine position
      if (detectedPosition && this.params.useCamshift && this.trackWindow) {
        detectedPosition = this.refineWithCamshift(detectedPosition);
      }

      // Update tracking state with Kalman filtering
      this.updateTrackingState(detectedPosition);

      this.processTime = performance.now() - startTime;

      return this.currentPosition;

    } catch (error) {
      console.error('LaserTracker processFrame error:', error);
      return null;
    }
  }

  /**
   * Detect laser by HSV color thresholding
   * @returns {Object|null} - Detected position or null
   */
  detectByColor() {
    // Create mask for laser color range
    const lowerBound = new cv.Mat(this.hsvMat.rows, this.hsvMat.cols, this.hsvMat.type(), [
      this.params.hueMin, this.params.satMin, this.params.valMin, 0
    ]);
    const upperBound = new cv.Mat(this.hsvMat.rows, this.hsvMat.cols, this.hsvMat.type(), [
      this.params.hueMax, this.params.satMax, this.params.valMax, 255
    ]);

    cv.inRange(this.hsvMat, lowerBound, upperBound, this.maskMat);

    // Clean up temporary matrices
    lowerBound.delete();
    upperBound.delete();

    // Morphological operations to clean up noise
    cv.morphologyEx(this.maskMat, this.maskMat, cv.MORPH_OPEN, this.morphKernel);
    cv.morphologyEx(this.maskMat, this.maskMat, cv.MORPH_CLOSE, this.morphKernel);

    // Apply ROI mask to limit detection to calibration area
    if (this.roiMask && !this.roiMask.isDeleted()) {
      cv.bitwise_and(this.maskMat, this.roiMask, this.maskMat);
    }

    // Find contours
    const contours = new cv.MatVector();
    const hierarchy = new cv.Mat();
    cv.findContours(this.maskMat, contours, hierarchy, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);

    // Find largest blob
    let maxArea = 0;
    let maxContourIdx = -1;

    for (let i = 0; i < contours.size(); i++) {
      const area = cv.contourArea(contours.get(i));
      if (area > this.params.minBlobArea &&
          area < this.params.maxBlobArea &&
          area > maxArea) {
        maxArea = area;
        maxContourIdx = i;
      }
    }

    let detectedPosition = null;

    if (maxContourIdx >= 0) {
      // Calculate centroid of largest contour
      const moments = cv.moments(contours.get(maxContourIdx));
      if (moments.m00 !== 0) {
        const cx = moments.m10 / moments.m00;
        const cy = moments.m01 / moments.m00;
        detectedPosition = { x: cx, y: cy };

        // Initialize CAMShift track window if enabled
        if (this.params.useCamshift) {
          const rect = cv.boundingRect(contours.get(maxContourIdx));
          this.initCamshift(rect);
        }
      }
    }

    // Clean up
    contours.delete();
    hierarchy.delete();

    return detectedPosition;
  }

  /**
   * Predict position using Lucas-Kanade optical flow
   * @returns {Object|null} - Predicted position or null
   */
  predictWithOpticalFlow() {
    if (!this.prevGray || this.prevGray.empty() ||
        !this.currGray || this.currGray.empty() ||
        !this.lastPosition) {
      return null;
    }

    try {
      // Create point to track from last known position
      const prevPts = cv.matFromArray(1, 1, cv.CV_32FC2, [
        this.lastPosition.x, this.lastPosition.y
      ]);
      const nextPts = new cv.Mat();
      const status = new cv.Mat();
      const err = new cv.Mat();

      // Calculate optical flow
      cv.calcOpticalFlowPyrLK(
        this.prevGray,
        this.currGray,
        prevPts,
        nextPts,
        status,
        err,
        new cv.Size(21, 21),
        3,
        new cv.TermCriteria(cv.TermCriteria_COUNT + cv.TermCriteria_EPS, 30, 0.01)
      );

      let predictedPos = null;

      // Check if tracking succeeded
      if (status.data[0] === 1) {
        const x = nextPts.data32F[0];
        const y = nextPts.data32F[1];

        // Validate the prediction is within bounds and reasonable
        if (x >= 0 && x < this.width && y >= 0 && y < this.height) {
          const dx = x - this.lastPosition.x;
          const dy = y - this.lastPosition.y;
          const dist = Math.sqrt(dx * dx + dy * dy);

          if (dist < this.params.maxVelocity) {
            predictedPos = { x, y, predicted: true };
          }
        }
      }

      // Clean up
      prevPts.delete();
      nextPts.delete();
      status.delete();
      err.delete();

      return predictedPos;
    } catch (e) {
      console.warn('Optical flow prediction failed:', e);
      return null;
    }
  }

  /**
   * Initialize CAMShift tracking with detected region
   * @param {Object} rect - Bounding rectangle of detected object
   */
  initCamshift(rect) {
    try {
      // Expand the rect slightly for better tracking
      const padding = 10;
      const x = Math.max(0, rect.x - padding);
      const y = Math.max(0, rect.y - padding);
      const w = Math.min(this.width - x, rect.width + padding * 2);
      const h = Math.min(this.height - y, rect.height + padding * 2);

      this.trackWindow = new cv.Rect(x, y, w, h);

      // Calculate histogram for the region
      const roi = this.hsvMat.roi(this.trackWindow);
      const mask = this.maskMat.roi(this.trackWindow);

      if (this.roiHist) this.roiHist.delete();
      this.roiHist = new cv.Mat();

      const hsvChannels = new cv.MatVector();
      hsvChannels.push_back(roi);

      cv.calcHist(hsvChannels, [0], mask, this.roiHist, [180], [0, 180]);
      cv.normalize(this.roiHist, this.roiHist, 0, 255, cv.NORM_MINMAX);

      hsvChannels.delete();
      this.camshiftEnabled = true;
    } catch (e) {
      console.warn('CAMShift init failed:', e);
      this.camshiftEnabled = false;
    }
  }

  /**
   * Refine position using CAMShift
   * @param {Object} detectedPos - Initially detected position
   * @returns {Object} - Refined position
   */
  refineWithCamshift(detectedPos) {
    if (!this.camshiftEnabled || !this.roiHist || !this.trackWindow) {
      return detectedPos;
    }

    try {
      // Back projection
      const dst = new cv.Mat();
      const hsvChannels = new cv.MatVector();
      hsvChannels.push_back(this.hsvMat);

      cv.calcBackProject(hsvChannels, [0], this.roiHist, dst, [0, 180], 1);

      // Apply CAMShift
      const termCrit = new cv.TermCriteria(
        cv.TermCriteria_EPS | cv.TermCriteria_COUNT, 10, 1
      );

      const [rotatedRect, newWindow] = cv.CamShift(dst, this.trackWindow, termCrit);

      // Update track window
      this.trackWindow = newWindow;

      // Get center from rotated rect
      const center = rotatedRect.center;

      hsvChannels.delete();
      dst.delete();

      // Blend CAMShift result with color detection
      return {
        x: detectedPos.x * 0.7 + center.x * 0.3,
        y: detectedPos.y * 0.7 + center.y * 0.3
      };
    } catch (e) {
      // CAMShift failed, reset tracking
      this.camshiftEnabled = false;
      return detectedPos;
    }
  }

  /**
   * Update tracking state with new detection, applying Kalman filtering
   * @param {Object|null} detectedPosition - Raw detected position
   */
  updateTrackingState(detectedPosition) {
    if (detectedPosition) {
      // Check if this is a new stroke (laser was off for a while)
      this.isNewStroke = this.framesSinceLastDetection > this.params.newStrokeThreshold;

      // Check for valid velocity (filter out noise jumps)
      // BUT: Skip this check if it's a new stroke - allow jumping to new position
      if (this.lastPosition && !detectedPosition.predicted && !this.isNewStroke) {
        const dx = detectedPosition.x - this.lastPosition.x;
        const dy = detectedPosition.y - this.lastPosition.y;
        const dist = Math.sqrt(dx * dx + dy * dy);

        if (dist > this.params.maxVelocity) {
          // Too fast - likely noise, ignore
          this.framesSinceLastDetection++;

          // But use Kalman prediction if available
          if (this.params.useKalman && this.kalmanState) {
            const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
            this.predictedPosition = {
              x: predicted.mean[0],
              y: predicted.mean[2]
            };
          }
          return;
        }

        this.velocity = { x: dx, y: dy };
      }

      // Apply Kalman filter for smooth tracking
      if (this.params.useKalman) {
        try {
          if (this.isNewStroke || !this.kalmanState) {
            // Initialize Kalman state with first observation
            // First predict with null state, then correct
            const predicted = this.kalmanFilter.predict({ previousCorrected: null });
            this.kalmanState = this.kalmanFilter.correct({
              predicted,
              observation: [detectedPosition.x, detectedPosition.y]
            });
          } else {
            // Predict and correct
            const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
            this.kalmanState = this.kalmanFilter.correct({
              predicted,
              observation: [detectedPosition.x, detectedPosition.y]
            });
          }

          // Use Kalman-filtered position
          this.currentPosition = {
            x: this.kalmanState.mean[0],
            y: this.kalmanState.mean[2]
          };

          // Store velocity from Kalman state
          this.velocity = {
            x: this.kalmanState.mean[1],
            y: this.kalmanState.mean[3]
          };
        } catch (e) {
          console.warn('Kalman filter error, falling back to simple smoothing:', e);
          // Fallback to simple smoothing on error
          if (this.currentPosition && !this.isNewStroke) {
            const s = this.params.smoothing;
            this.currentPosition = {
              x: this.currentPosition.x * s + detectedPosition.x * (1 - s),
              y: this.currentPosition.y * s + detectedPosition.y * (1 - s)
            };
          } else {
            this.currentPosition = { ...detectedPosition };
          }
        }
      } else {
        // Fallback to simple smoothing
        if (this.currentPosition && !this.isNewStroke) {
          const s = this.params.smoothing;
          this.currentPosition = {
            x: this.currentPosition.x * s + detectedPosition.x * (1 - s),
            y: this.currentPosition.y * s + detectedPosition.y * (1 - s)
          };
        } else {
          this.currentPosition = { ...detectedPosition };
        }
      }

      this.lastPosition = { ...detectedPosition };
      this.isTracking = true;
      this.framesSinceLastDetection = 0;

    } else {
      // No detection
      this.framesSinceLastDetection++;

      // Use Kalman prediction to maintain tracking briefly
      if (this.params.useKalman && this.kalmanState &&
          this.framesSinceLastDetection <= this.params.newStrokeThreshold) {
        try {
          const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
          // Don't update kalmanState without a correction - just use prediction for display
          this.predictedPosition = {
            x: predicted.mean[0],
            y: predicted.mean[2]
          };
          this.currentPosition = { ...this.predictedPosition };
        } catch (e) {
          // Prediction failed, ignore
        }
      }

      if (this.framesSinceLastDetection > this.params.newStrokeThreshold) {
        this.isTracking = false;
        this.camshiftEnabled = false;
        this.kalmanState = null;  // Reset Kalman state for new stroke
      }
    }
  }

  /**
   * Draw debug visualization
   * @param {CanvasRenderingContext2D} ctx - Canvas context to draw on
   * @param {ImageData} originalFrame - Original camera frame
   */
  drawDebug(ctx, originalFrame) {
    if (!this.params.showDebug) return;

    const canvas = ctx.canvas;

    // Draw original frame
    ctx.putImageData(originalFrame, 0, 0);

    // Draw mask overlay if available
    if (this.maskMat && !this.maskMat.isDeleted()) {
      try {
        const maskData = new ImageData(
          new Uint8ClampedArray(this.maskMat.data),
          this.maskMat.cols,
          this.maskMat.rows
        );

        // Create overlay
        const overlayCanvas = document.createElement('canvas');
        overlayCanvas.width = this.maskMat.cols;
        overlayCanvas.height = this.maskMat.rows;
        const overlayCtx = overlayCanvas.getContext('2d');

        // Draw mask as colored overlay
        const tempImageData = overlayCtx.createImageData(this.maskMat.cols, this.maskMat.rows);
        for (let i = 0; i < this.maskMat.data.length; i++) {
          const idx = i * 4;
          if (this.maskMat.data[i] > 0) {
            tempImageData.data[idx] = 0;      // R
            tempImageData.data[idx + 1] = 255; // G
            tempImageData.data[idx + 2] = 0;   // B
            tempImageData.data[idx + 3] = 128; // A
          } else {
            tempImageData.data[idx + 3] = 0;   // Transparent
          }
        }
        overlayCtx.putImageData(tempImageData, 0, 0);
        ctx.drawImage(overlayCanvas, 0, 0, canvas.width, canvas.height);
      } catch (e) {
        // Mask not ready yet
      }
    }

    // Draw predicted position (if using prediction)
    if (this.predictedPosition && this.framesSinceLastDetection > 0) {
      const scaleX = canvas.width / this.width;
      const scaleY = canvas.height / this.height;

      ctx.beginPath();
      ctx.arc(
        this.predictedPosition.x * scaleX,
        this.predictedPosition.y * scaleY,
        8, 0, Math.PI * 2
      );
      ctx.strokeStyle = '#ff0';
      ctx.lineWidth = 2;
      ctx.setLineDash([4, 4]);
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // Draw detected position
    if (this.currentPosition) {
      const scaleX = canvas.width / this.width;
      const scaleY = canvas.height / this.height;

      ctx.beginPath();
      ctx.arc(
        this.currentPosition.x * scaleX,
        this.currentPosition.y * scaleY,
        10, 0, Math.PI * 2
      );
      ctx.strokeStyle = this.isTracking ? '#0f0' : '#f00';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Crosshair
      ctx.beginPath();
      ctx.moveTo(this.currentPosition.x * scaleX - 15, this.currentPosition.y * scaleY);
      ctx.lineTo(this.currentPosition.x * scaleX + 15, this.currentPosition.y * scaleY);
      ctx.moveTo(this.currentPosition.x * scaleX, this.currentPosition.y * scaleY - 15);
      ctx.lineTo(this.currentPosition.x * scaleX, this.currentPosition.y * scaleY + 15);
      ctx.stroke();

      // Draw velocity vector
      if (this.velocity && (Math.abs(this.velocity.x) > 1 || Math.abs(this.velocity.y) > 1)) {
        ctx.beginPath();
        ctx.moveTo(this.currentPosition.x * scaleX, this.currentPosition.y * scaleY);
        ctx.lineTo(
          (this.currentPosition.x + this.velocity.x * 3) * scaleX,
          (this.currentPosition.y + this.velocity.y * 3) * scaleY
        );
        ctx.strokeStyle = '#0ff';
        ctx.lineWidth = 2;
        ctx.stroke();
      }
    }

    // Draw CAMShift track window
    if (this.params.useCamshift && this.trackWindow && this.camshiftEnabled) {
      const scaleX = canvas.width / this.width;
      const scaleY = canvas.height / this.height;

      ctx.strokeStyle = '#f0f';
      ctx.lineWidth = 1;
      ctx.strokeRect(
        this.trackWindow.x * scaleX,
        this.trackWindow.y * scaleY,
        this.trackWindow.width * scaleX,
        this.trackWindow.height * scaleY
      );
    }

    // Draw status text
    ctx.fillStyle = '#fff';
    ctx.font = '12px monospace';
    ctx.fillText(`Tracking: ${this.isTracking ? 'ON' : 'OFF'}`, 5, 15);
    ctx.fillText(`Process: ${this.processTime.toFixed(1)}ms`, 5, 30);

    const features = [];
    if (this.params.useKalman) features.push('Kalman');
    if (this.params.useOpticalFlow) features.push('OptFlow');
    if (this.params.useCamshift) features.push('CAMShift');
    if (features.length > 0) {
      ctx.fillText(`Features: ${features.join(', ')}`, 5, 45);
    }
  }

  /**
   * Get normalized position (0-1 range)
   * @returns {Object|null} - Normalized position {x, y} or null
   */
  getNormalizedPosition() {
    if (!this.currentPosition) return null;

    return {
      x: this.currentPosition.x / this.width,
      y: this.currentPosition.y / this.height
    };
  }

  /**
   * Set ROI (Region of Interest) from calibration quad
   * Only detections within this polygon will be considered
   * @param {Array<{x: number, y: number}>} quad - 4 corner points in camera pixel coords
   */
  setROI(quad) {
    if (!quad || quad.length !== 4 || !this.width || !this.height) {
      this.roiMask = null;
      return;
    }

    // Create a filled polygon mask from the quad
    const mask = cv.Mat.zeros(this.height, this.width, cv.CV_8UC1);
    const pts = cv.matFromArray(4, 1, cv.CV_32SC2, [
      Math.round(quad[0].x), Math.round(quad[0].y),
      Math.round(quad[1].x), Math.round(quad[1].y),
      Math.round(quad[2].x), Math.round(quad[2].y),
      Math.round(quad[3].x), Math.round(quad[3].y)
    ]);
    const contours = new cv.MatVector();
    contours.push_back(pts);
    cv.fillPoly(mask, contours, new cv.Scalar(255));
    pts.delete();
    contours.delete();

    // Replace old mask
    if (this.roiMask && !this.roiMask.isDeleted()) {
      this.roiMask.delete();
    }
    this.roiMask = mask;
  }

  /**
   * Update tracking parameters
   * @param {Object} newParams - New parameter values
   */
  setParams(newParams) {
    Object.assign(this.params, newParams);
  }

  /**
   * Clean up OpenCV matrices
   */
  dispose() {
    if (this.srcMat) this.srcMat.delete();
    if (this.hsvMat) this.hsvMat.delete();
    if (this.maskMat) this.maskMat.delete();
    if (this.morphKernel) this.morphKernel.delete();
    if (this.prevGray) this.prevGray.delete();
    if (this.currGray) this.currGray.delete();
    if (this.roiHist) this.roiHist.delete();
    if (this.roiMask) this.roiMask.delete();

    this.srcMat = null;
    this.hsvMat = null;
    this.maskMat = null;
    this.morphKernel = null;
    this.prevGray = null;
    this.currGray = null;
    this.roiHist = null;
  }
}
