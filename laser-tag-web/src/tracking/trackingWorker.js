/**
 * trackingWorker.js - Web Worker for laser detection using OpenCV.js
 *
 * Offloads the heavy OpenCV processing (HSV conversion, thresholding,
 * morphological operations, contour finding) from the main thread so the
 * UI stays responsive during drawing.
 *
 * Communication protocol:
 *   Main → Worker:
 *     { type: 'init',         opencvUrl, width, height }
 *     { type: 'processFrame', data: ArrayBuffer, width, height }  (transferred)
 *     { type: 'setParams',    params }
 *     { type: 'setROI',       quad }
 *     { type: 'dispose' }
 *
 *   Worker → Main:
 *     { type: 'ready' }
 *     { type: 'result', position: {x,y}|null, processTime }
 *     { type: 'error',  message }
 */

/* eslint-disable no-restricted-globals */

// OpenCV matrices (reused across frames)
let srcMat = null;
let hsvMat = null;
let maskMat = null;
let morphKernel = null;
let roiMask = null;
let lowerBound = null;
let upperBound = null;
let width = 0;
let height = 0;
let cvReady = false;
let boundsNeedUpdate = true; // rebuild bound Mats when params change

// Tracking parameters (mirrors LaserTracker.params)
let params = {
  hueMin: 35,
  hueMax: 85,
  satMin: 50,
  satMax: 255,
  valMin: 200,
  valMax: 255,
  minBlobArea: 10,
  maxBlobArea: 5000
};

/**
 * Load OpenCV.js in the worker context
 */
function loadOpenCV(url) {
  var INIT_TIMEOUT = 30000; // 30 s

  return new Promise((resolve, reject) => {
    try {
      self.importScripts(url);
    } catch (e) {
      reject(new Error('Failed to load OpenCV.js: ' + e.message));
      return;
    }

    // cv may need WASM runtime initialization
    if (typeof cv !== 'undefined') {
      if (cv.Mat) {
        resolve();
      } else if (typeof cv === 'function') {
        // OpenCV.js factory pattern (common in newer builds)
        cv().then(function (instance) {
          self.cv = instance;
          resolve();
        }).catch(reject);
      } else {
        var timer = setTimeout(function () {
          reject(new Error('OpenCV.js runtime initialization timed out'));
        }, INIT_TIMEOUT);
        cv.onRuntimeInitialized = function () {
          clearTimeout(timer);
          resolve();
        };
      }
    } else {
      reject(new Error('OpenCV.js did not define cv global'));
    }
  });
}

/**
 * Initialize OpenCV matrices for the given frame dimensions
 */
function initMatrices(w, h) {
  cleanupMatrices();

  width = w;
  height = h;

  srcMat = new cv.Mat(h, w, cv.CV_8UC4);
  hsvMat = new cv.Mat();
  maskMat = new cv.Mat();
  morphKernel = cv.getStructuringElement(cv.MORPH_ELLIPSE, new cv.Size(5, 5));

  boundsNeedUpdate = true;
}

/**
 * Rebuild the HSV lower/upper bound Mats (called when params or dimensions change)
 */
function rebuildBounds() {
  if (lowerBound && !lowerBound.isDeleted()) lowerBound.delete();
  if (upperBound && !upperBound.isDeleted()) upperBound.delete();

  lowerBound = new cv.Mat(height, width, cv.CV_8UC3, [
    params.hueMin, params.satMin, params.valMin, 0
  ]);
  upperBound = new cv.Mat(height, width, cv.CV_8UC3, [
    params.hueMax, params.satMax, params.valMax, 255
  ]);
  boundsNeedUpdate = false;
}

/**
 * Clean up OpenCV matrices
 */
function cleanupMatrices() {
  try {
    if (srcMat && !srcMat.isDeleted()) srcMat.delete();
    if (hsvMat && !hsvMat.isDeleted()) hsvMat.delete();
    if (maskMat && !maskMat.isDeleted()) maskMat.delete();
    if (morphKernel && !morphKernel.isDeleted()) morphKernel.delete();
    if (roiMask && !roiMask.isDeleted()) roiMask.delete();
    if (lowerBound && !lowerBound.isDeleted()) lowerBound.delete();
    if (upperBound && !upperBound.isDeleted()) upperBound.delete();
  } catch (e) {
    // Ignore cleanup errors
  }
  srcMat = null;
  hsvMat = null;
  maskMat = null;
  morphKernel = null;
  roiMask = null;
  lowerBound = null;
  upperBound = null;
}

/**
 * Set Region of Interest from calibration quad
 */
function setROI(quad) {
  if (!quad || quad.length !== 4 || !width || !height) {
    if (roiMask && !roiMask.isDeleted()) roiMask.delete();
    roiMask = null;
    return;
  }

  const mask = cv.Mat.zeros(height, width, cv.CV_8UC1);
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

  if (roiMask && !roiMask.isDeleted()) roiMask.delete();
  roiMask = mask;
}

/**
 * Detect laser position via HSV color thresholding + contour analysis
 * @param {ArrayBuffer} data - RGBA pixel data (transferred from main thread)
 * @param {number} w - Frame width
 * @param {number} h - Frame height
 * @returns {{ position: {x: number, y: number}|null, processTime: number }}
 */
function detectLaser(data, w, h) {
  const startTime = performance.now();

  // Reinitialize if dimensions changed
  if (w !== width || h !== height) {
    initMatrices(w, h);
  }

  try {
    // Load pixel data into OpenCV matrix (data is an ArrayBuffer via Transferable)
    srcMat.data.set(new Uint8ClampedArray(data));

    // Convert RGBA → HSV
    cv.cvtColor(srcMat, hsvMat, cv.COLOR_RGBA2RGB);
    cv.cvtColor(hsvMat, hsvMat, cv.COLOR_RGB2HSV);

    // Rebuild bound Mats lazily (only when params or dimensions change)
    if (boundsNeedUpdate) {
      rebuildBounds();
    }

    // HSV thresholding (reuses pre-allocated bound Mats)
    cv.inRange(hsvMat, lowerBound, upperBound, maskMat);

    // Morphological operations (noise cleanup)
    cv.morphologyEx(maskMat, maskMat, cv.MORPH_OPEN, morphKernel);
    cv.morphologyEx(maskMat, maskMat, cv.MORPH_CLOSE, morphKernel);

    // Apply ROI mask
    if (roiMask && !roiMask.isDeleted()) {
      cv.bitwise_and(maskMat, roiMask, maskMat);
    }

    // Find contours
    const contours = new cv.MatVector();
    const hierarchy = new cv.Mat();
    cv.findContours(maskMat, contours, hierarchy, cv.RETR_EXTERNAL, cv.CHAIN_APPROX_SIMPLE);

    // Find brightest valid blob (brightness at centroid discriminates laser from ambient light)
    let bestScore = 0;
    let position = null;

    for (let i = 0; i < contours.size(); i++) {
      const area = cv.contourArea(contours.get(i));
      if (area > params.minBlobArea && area < params.maxBlobArea) {
        const moments = cv.moments(contours.get(i));
        if (moments.m00 !== 0) {
          const cx = moments.m10 / moments.m00;
          const cy = moments.m01 / moments.m00;
          // Read V (brightness) channel at centroid — HSV is 3-channel, V is index 2
          const vIdx = (Math.round(cy) * width + Math.round(cx)) * 3 + 2;
          const brightness = hsvMat.data[vIdx] || 0;
          // Primary: brightness, secondary: area (tiebreaker)
          const score = brightness * 10000 + area;
          if (score > bestScore) {
            bestScore = score;
            position = { x: cx, y: cy };
          }
        }
      }
    }

    contours.delete();
    hierarchy.delete();

    return { position, processTime: performance.now() - startTime };

  } catch (error) {
    return { position: null, processTime: performance.now() - startTime, error: error.message };
  }
}

/**
 * Message handler
 */
self.onmessage = function (e) {
  const msg = e.data;

  switch (msg.type) {
    case 'init': {
      loadOpenCV(msg.opencvUrl)
        .then(function () {
          cvReady = true;
          initMatrices(msg.width, msg.height);
          if (msg.params) Object.assign(params, msg.params);
          self.postMessage({ type: 'ready' });
        })
        .catch(function (err) {
          self.postMessage({ type: 'error', message: err.message });
        });
      break;
    }

    case 'processFrame': {
      if (!cvReady) return;
      const result = detectLaser(msg.data, msg.width, msg.height);
      self.postMessage({ type: 'result', position: result.position, processTime: result.processTime });
      break;
    }

    case 'setParams': {
      Object.assign(params, msg.params);
      boundsNeedUpdate = true;
      break;
    }

    case 'setROI': {
      if (cvReady) {
        setROI(msg.quad);
      }
      break;
    }

    case 'dispose': {
      cleanupMatrices();
      cvReady = false;
      break;
    }
  }
};
