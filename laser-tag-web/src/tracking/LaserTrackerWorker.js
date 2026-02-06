/**
 * LaserTrackerWorker - Offloads OpenCV laser detection to a Web Worker
 *
 * Provides the same public interface as LaserTracker so it can be used
 * as a drop-in replacement in AppController.  Heavy OpenCV work runs in
 * a background thread; Kalman filtering and state management remain on
 * the main thread for low-latency access by the rendering loop.
 *
 * Falls back to the original main-thread LaserTracker if the worker
 * cannot be created (e.g. browser restrictions, missing OpenCV.js).
 */
import { KalmanFilter } from 'kalman-filter';
import { LaserTracker } from './LaserTracker.js';

export class LaserTrackerWorker {
  constructor() {
    // ---- Worker management ----
    this.worker = null;
    this.workerReady = false;
    this.fallbackTracker = null; // used when worker is unavailable

    // ---- Tracking state (same fields as LaserTracker) ----
    this.isTracking = false;
    this.lastPosition = null;
    this.currentPosition = null;
    this.predictedPosition = null;
    this.velocity = { x: 0, y: 0 };
    this.isNewStroke = false;
    this.framesSinceLastDetection = 0;

    // ---- Kalman filter (runs on main thread) ----
    this.kalmanFilter = null;
    this.kalmanState = null;

    // ---- Parameters (shared with worker) ----
    this.params = {
      hueMin: 35,
      hueMax: 85,
      satMin: 50,
      satMax: 255,
      valMin: 200,
      valMax: 255,
      minBlobArea: 10,
      maxBlobArea: 5000,
      smoothing: 0.5,
      newStrokeThreshold: 10,
      maxVelocity: 100,
      useKalman: true,
      useOpticalFlow: true,
      useCamshift: false,
      showDebug: true
    };

    // ---- Dimensions ----
    this.width = 0;
    this.height = 0;

    // ---- Performance ----
    this.processTime = 0;

    // ---- ROI ----
    this.roiMask = null; // kept for interface compat; actual mask lives in worker

    // ---- Debug ----
    this._lastImageData = null; // cached for drawDebug
  }

  // -----------------------------------------------------------
  //  Initialization
  // -----------------------------------------------------------

  /**
   * Initialize the tracker.
   * Tries to create a Web Worker; falls back to main-thread LaserTracker.
   * @param {number} width - Frame width
   * @param {number} height - Frame height
   */
  init(width, height) {
    this.width = width;
    this.height = height;
    this.initKalmanFilter();

    try {
      this.worker = new Worker(
        new URL('./trackingWorker.js', import.meta.url)
      );

      this.worker.onmessage = (e) => this._onWorkerMessage(e);
      this.worker.onerror = (e) => {
        console.warn('Tracking worker error, falling back to main thread:', e.message);
        this._activateFallback();
      };

      // Determine OpenCV URL for the worker
      const opencvUrl = new URL('/lib/opencv.js', self.location.origin).href;

      this.worker.postMessage({
        type: 'init',
        opencvUrl,
        width,
        height,
        params: this._getWorkerParams()
      });

      console.log('LaserTrackerWorker: Web Worker created, waiting for OpenCV…');
    } catch (err) {
      console.warn('Failed to create tracking worker:', err.message);
      this._activateFallback();
    }
  }

  /**
   * Activate the main-thread fallback tracker.
   */
  _activateFallback() {
    if (this.worker) {
      this.worker.terminate();
      this.worker = null;
    }
    this.workerReady = false;
    this.fallbackTracker = new LaserTracker();
    this.fallbackTracker.init(this.width, this.height);
    console.log('LaserTrackerWorker: using main-thread fallback');
  }

  // -----------------------------------------------------------
  //  Kalman filter (identical to LaserTracker)
  // -----------------------------------------------------------

  initKalmanFilter() {
    this.kalmanFilter = new KalmanFilter({
      observation: {
        dimension: 2,
        stateProjection: [
          [1, 0, 0, 0],
          [0, 0, 1, 0]
        ],
        covariance: [10, 10]
      },
      dynamic: {
        dimension: 4,
        transition: [
          [1, 1, 0, 0],
          [0, 1, 0, 0],
          [0, 0, 1, 1],
          [0, 0, 0, 1]
        ],
        covariance: [1, 1, 1, 1]
      }
    });
    this.kalmanState = null;
  }

  // -----------------------------------------------------------
  //  Frame processing
  // -----------------------------------------------------------

  /**
   * Process a video frame.
   * If using the worker, posts the frame data and returns immediately.
   * The tracking state is updated asynchronously when the worker responds.
   *
   * @param {ImageData} imageData - Camera frame
   * @returns {Object|null} Latest currentPosition (may be from previous frame)
   */
  processFrame(imageData) {
    // Store for debug drawing
    this._lastImageData = imageData;

    // --- Fallback: delegate entirely to main-thread tracker ---
    if (this.fallbackTracker) {
      const pos = this.fallbackTracker.processFrame(imageData);
      this._syncFromFallback();
      return pos;
    }

    // --- Worker path ---
    if (!this.workerReady) return this.currentPosition;

    // Post frame data to worker (structured clone copies the TypedArray)
    this.worker.postMessage({
      type: 'processFrame',
      data: imageData.data,
      width: imageData.width,
      height: imageData.height
    });

    return this.currentPosition;
  }

  // -----------------------------------------------------------
  //  Worker message handler
  // -----------------------------------------------------------

  _onWorkerMessage(e) {
    const msg = e.data;

    switch (msg.type) {
      case 'ready':
        this.workerReady = true;
        console.log('LaserTrackerWorker: Web Worker ready (OpenCV loaded)');
        break;

      case 'result':
        this._applyDetectionResult(msg.position, msg.processTime);
        break;

      case 'error':
        console.warn('Tracking worker reported error:', msg.message);
        this._activateFallback();
        break;
    }
  }

  /**
   * Apply a detection result from the worker, running Kalman filtering
   * on the main thread.
   */
  _applyDetectionResult(detectedPosition, workerProcessTime) {
    this.processTime = workerProcessTime;

    if (detectedPosition) {
      this.isNewStroke = this.framesSinceLastDetection > this.params.newStrokeThreshold;

      // Velocity validation (skip for new strokes)
      if (this.lastPosition && !this.isNewStroke) {
        const dx = detectedPosition.x - this.lastPosition.x;
        const dy = detectedPosition.y - this.lastPosition.y;
        const dist = Math.sqrt(dx * dx + dy * dy);

        if (dist > this.params.maxVelocity) {
          this.framesSinceLastDetection++;
          if (this.params.useKalman && this.kalmanState) {
            const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
            this.predictedPosition = { x: predicted.mean[0], y: predicted.mean[2] };
          }
          return;
        }
        this.velocity = { x: dx, y: dy };
      }

      // Kalman filter
      if (this.params.useKalman) {
        try {
          if (this.isNewStroke || !this.kalmanState) {
            const predicted = this.kalmanFilter.predict({ previousCorrected: null });
            this.kalmanState = this.kalmanFilter.correct({
              predicted,
              observation: [detectedPosition.x, detectedPosition.y]
            });
          } else {
            const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
            this.kalmanState = this.kalmanFilter.correct({
              predicted,
              observation: [detectedPosition.x, detectedPosition.y]
            });
          }
          this.currentPosition = {
            x: this.kalmanState.mean[0],
            y: this.kalmanState.mean[2]
          };
          this.velocity = {
            x: this.kalmanState.mean[1],
            y: this.kalmanState.mean[3]
          };
        } catch (e) {
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
      } else {
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

      if (this.params.useKalman && this.kalmanState &&
          this.framesSinceLastDetection <= this.params.newStrokeThreshold) {
        try {
          const predicted = this.kalmanFilter.predict({ previousCorrected: this.kalmanState });
          this.predictedPosition = { x: predicted.mean[0], y: predicted.mean[2] };
          this.currentPosition = { ...this.predictedPosition };
        } catch (e) {
          // Prediction failed
        }
      }

      if (this.framesSinceLastDetection > this.params.newStrokeThreshold) {
        this.isTracking = false;
        this.kalmanState = null;
      }
    }
  }

  // -----------------------------------------------------------
  //  Public API (same as LaserTracker)
  // -----------------------------------------------------------

  /**
   * Get normalized position (0-1 range)
   */
  getNormalizedPosition() {
    if (!this.currentPosition) return null;
    return {
      x: this.currentPosition.x / this.width,
      y: this.currentPosition.y / this.height
    };
  }

  /**
   * Set ROI from calibration quad
   */
  setROI(quad) {
    if (this.fallbackTracker) {
      this.fallbackTracker.setROI(quad);
      return;
    }
    if (this.worker && this.workerReady) {
      this.worker.postMessage({ type: 'setROI', quad });
    }
  }

  /**
   * Update tracking parameters
   */
  setParams(newParams) {
    Object.assign(this.params, newParams);
    if (this.fallbackTracker) {
      this.fallbackTracker.setParams(newParams);
      return;
    }
    if (this.worker) {
      this.worker.postMessage({ type: 'setParams', params: this._getWorkerParams() });
    }
  }

  /**
   * Draw debug visualisation.
   * In worker mode the mask overlay is not available; everything else
   * (camera frame, position crosshair, velocity, status) is drawn.
   */
  drawDebug(ctx, originalFrame) {
    if (!this.params.showDebug) return;

    if (this.fallbackTracker) {
      this.fallbackTracker.drawDebug(ctx, originalFrame);
      return;
    }

    const canvas = ctx.canvas;
    const imageData = originalFrame || this._lastImageData;

    // Draw camera frame
    if (imageData) {
      ctx.putImageData(imageData, 0, 0);
    }

    // Draw predicted position
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

    // Draw current position
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

      // Velocity vector
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

    // Status text
    ctx.fillStyle = '#fff';
    ctx.font = '12px monospace';
    ctx.fillText(`Tracking: ${this.isTracking ? 'ON' : 'OFF'}`, 5, 15);
    ctx.fillText(`Process: ${this.processTime.toFixed(1)}ms (worker)`, 5, 30);

    const features = [];
    if (this.params.useKalman) features.push('Kalman');
    if (this.params.useOpticalFlow) features.push('OptFlow');
    if (this.params.useCamshift) features.push('CAMShift');
    features.push('WebWorker');
    ctx.fillText(`Features: ${features.join(', ')}`, 5, 45);
  }

  /**
   * Clean up resources
   */
  dispose() {
    if (this.worker) {
      this.worker.postMessage({ type: 'dispose' });
      this.worker.terminate();
      this.worker = null;
    }
    if (this.fallbackTracker) {
      this.fallbackTracker.dispose();
      this.fallbackTracker = null;
    }
    this.workerReady = false;
    this._lastImageData = null;
  }

  // -----------------------------------------------------------
  //  Private helpers
  // -----------------------------------------------------------

  /**
   * Copy tracking state from fallback tracker to our own fields
   * so AppController can read them uniformly.
   */
  _syncFromFallback() {
    const t = this.fallbackTracker;
    this.isTracking = t.isTracking;
    this.currentPosition = t.currentPosition;
    this.lastPosition = t.lastPosition;
    this.predictedPosition = t.predictedPosition;
    this.velocity = t.velocity;
    this.isNewStroke = t.isNewStroke;
    this.framesSinceLastDetection = t.framesSinceLastDetection;
    this.processTime = t.processTime;
  }

  /**
   * Extract worker-relevant params (only HSV + blob settings)
   */
  _getWorkerParams() {
    return {
      hueMin: this.params.hueMin,
      hueMax: this.params.hueMax,
      satMin: this.params.satMin,
      satMax: this.params.satMax,
      valMin: this.params.valMin,
      valMax: this.params.valMax,
      minBlobArea: this.params.minBlobArea,
      maxBlobArea: this.params.maxBlobArea
    };
  }
}
