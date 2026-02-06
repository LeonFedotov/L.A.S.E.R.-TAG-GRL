/**
 * LaserTrackerWorker tests
 *
 * Since Web Workers are not available in jsdom, these tests verify:
 * 1. Fallback to main-thread LaserTracker works correctly
 * 2. Tracking state management (Kalman filtering, stroke detection)
 * 3. Public API parity with LaserTracker
 */
import { describe, it, expect, beforeEach, vi } from 'vitest';

// Mock LaserTracker (requires OpenCV which is not available in test env)
vi.mock('./LaserTracker.js', () => ({
  LaserTracker: vi.fn().mockImplementation(() => ({
    init: vi.fn(),
    processFrame: vi.fn().mockReturnValue(null),
    isTracking: false,
    currentPosition: null,
    lastPosition: null,
    predictedPosition: null,
    velocity: { x: 0, y: 0 },
    isNewStroke: false,
    framesSinceLastDetection: 0,
    processTime: 0,
    params: {
      hueMin: 35, hueMax: 85,
      satMin: 50, satMax: 255,
      valMin: 200, valMax: 255,
      minBlobArea: 10, maxBlobArea: 5000,
      smoothing: 0.5, newStrokeThreshold: 10,
      maxVelocity: 100, useKalman: true,
      useOpticalFlow: true, useCamshift: false,
      showDebug: true
    },
    setROI: vi.fn(),
    setParams: vi.fn(),
    drawDebug: vi.fn(),
    dispose: vi.fn(),
    getNormalizedPosition: vi.fn().mockReturnValue(null)
  }))
}));

import { LaserTrackerWorker } from './LaserTrackerWorker.js';

describe('LaserTrackerWorker', () => {
  let tracker;

  beforeEach(() => {
    vi.clearAllMocks();
    tracker = new LaserTrackerWorker();
  });

  describe('constructor', () => {
    it('initializes with default tracking state', () => {
      expect(tracker.isTracking).toBe(false);
      expect(tracker.currentPosition).toBe(null);
      expect(tracker.isNewStroke).toBe(false);
      expect(tracker.framesSinceLastDetection).toBe(0);
      expect(tracker.processTime).toBe(0);
    });

    it('initializes with default HSV parameters', () => {
      expect(tracker.params.hueMin).toBe(35);
      expect(tracker.params.hueMax).toBe(85);
      expect(tracker.params.valMin).toBe(200);
      expect(tracker.params.useKalman).toBe(true);
    });
  });

  describe('init (fallback mode)', () => {
    it('falls back to LaserTracker when Worker is unavailable', () => {
      // jsdom does not support real Workers, so it will fall back
      tracker.init(640, 480);

      expect(tracker.width).toBe(640);
      expect(tracker.height).toBe(480);
      expect(tracker.fallbackTracker).not.toBe(null);
      expect(tracker.fallbackTracker.init).toHaveBeenCalledWith(640, 480);
    });
  });

  describe('processFrame (fallback mode)', () => {
    beforeEach(() => {
      tracker.init(640, 480);
    });

    it('delegates to fallback tracker', () => {
      const fakeImageData = { data: new Uint8ClampedArray(4), width: 640, height: 480 };
      tracker.processFrame(fakeImageData);

      expect(tracker.fallbackTracker.processFrame).toHaveBeenCalledWith(fakeImageData);
    });

    it('syncs tracking state from fallback', () => {
      tracker.fallbackTracker.isTracking = true;
      tracker.fallbackTracker.currentPosition = { x: 100, y: 200 };
      tracker.fallbackTracker.processTime = 1.5;

      const fakeImageData = { data: new Uint8ClampedArray(4), width: 640, height: 480 };
      tracker.processFrame(fakeImageData);

      expect(tracker.isTracking).toBe(true);
      expect(tracker.currentPosition).toEqual({ x: 100, y: 200 });
      expect(tracker.processTime).toBe(1.5);
    });
  });

  describe('setROI (fallback mode)', () => {
    beforeEach(() => {
      tracker.init(640, 480);
    });

    it('delegates to fallback tracker', () => {
      const quad = [
        { x: 0, y: 0 }, { x: 640, y: 0 },
        { x: 640, y: 480 }, { x: 0, y: 480 }
      ];
      tracker.setROI(quad);
      expect(tracker.fallbackTracker.setROI).toHaveBeenCalledWith(quad);
    });
  });

  describe('setParams (fallback mode)', () => {
    beforeEach(() => {
      tracker.init(640, 480);
    });

    it('updates local params and delegates to fallback', () => {
      tracker.setParams({ hueMin: 50, hueMax: 100 });
      expect(tracker.params.hueMin).toBe(50);
      expect(tracker.params.hueMax).toBe(100);
      expect(tracker.fallbackTracker.setParams).toHaveBeenCalledWith({ hueMin: 50, hueMax: 100 });
    });
  });

  describe('getNormalizedPosition', () => {
    it('returns null when no position', () => {
      tracker.init(640, 480);
      expect(tracker.getNormalizedPosition()).toBe(null);
    });

    it('normalizes position to 0-1 range', () => {
      tracker.width = 640;
      tracker.height = 480;
      tracker.currentPosition = { x: 320, y: 240 };

      const norm = tracker.getNormalizedPosition();
      expect(norm.x).toBe(0.5);
      expect(norm.y).toBe(0.5);
    });
  });

  describe('_applyDetectionResult', () => {
    beforeEach(() => {
      tracker.width = 640;
      tracker.height = 480;
      tracker.initKalmanFilter();
    });

    it('sets tracking state on detection', () => {
      tracker._applyDetectionResult({ x: 100, y: 200 }, 1.0);

      expect(tracker.isTracking).toBe(true);
      expect(tracker.currentPosition).not.toBe(null);
      expect(tracker.framesSinceLastDetection).toBe(0);
      expect(tracker.processTime).toBe(1.0);
    });

    it('increments framesSinceLastDetection on no detection', () => {
      tracker._applyDetectionResult(null, 0.5);

      expect(tracker.framesSinceLastDetection).toBe(1);
    });

    it('marks new stroke after threshold', () => {
      // Simulate many frames without detection
      for (let i = 0; i <= tracker.params.newStrokeThreshold; i++) {
        tracker._applyDetectionResult(null, 0.5);
      }
      expect(tracker.isTracking).toBe(false);

      // New detection should be a new stroke
      tracker._applyDetectionResult({ x: 300, y: 150 }, 1.0);
      expect(tracker.isNewStroke).toBe(true);
      expect(tracker.isTracking).toBe(true);
    });

    it('filters noise (large jumps)', () => {
      // First detection
      tracker._applyDetectionResult({ x: 100, y: 100 }, 1.0);
      expect(tracker.isTracking).toBe(true);

      // Large jump (> maxVelocity) should be filtered
      tracker._applyDetectionResult({ x: 500, y: 500 }, 1.0);
      // Still tracking from first position, frame count incremented
      expect(tracker.framesSinceLastDetection).toBe(1);
    });

    it('uses simple smoothing when Kalman is disabled', () => {
      tracker.params.useKalman = false;

      tracker._applyDetectionResult({ x: 100, y: 100 }, 1.0);
      expect(tracker.currentPosition).toEqual({ x: 100, y: 100 });

      tracker._applyDetectionResult({ x: 110, y: 110 }, 1.0);
      // Smoothed position should be between old and new
      expect(tracker.currentPosition.x).toBeGreaterThan(100);
      expect(tracker.currentPosition.x).toBeLessThanOrEqual(110);
    });
  });

  describe('dispose', () => {
    it('cleans up fallback tracker', () => {
      tracker.init(640, 480);
      tracker.dispose();

      expect(tracker.fallbackTracker).toBe(null);
      expect(tracker.workerReady).toBe(false);
    });
  });

  describe('_getWorkerParams', () => {
    it('extracts only worker-relevant params', () => {
      const workerParams = tracker._getWorkerParams();

      expect(workerParams).toHaveProperty('hueMin');
      expect(workerParams).toHaveProperty('hueMax');
      expect(workerParams).toHaveProperty('minBlobArea');
      expect(workerParams).not.toHaveProperty('useKalman');
      expect(workerParams).not.toHaveProperty('smoothing');
    });
  });

  describe('back-pressure (worker mode simulation)', () => {
    it('skips frames while worker is busy', () => {
      tracker.workerReady = true;
      tracker._workerBusy = false;
      tracker.worker = { postMessage: vi.fn() };
      tracker.fallbackTracker = null;

      const fakeImageData = { data: new Uint8ClampedArray(16), width: 2, height: 2 };

      // First frame should be sent
      tracker.processFrame(fakeImageData);
      expect(tracker.worker.postMessage).toHaveBeenCalledTimes(1);
      expect(tracker._workerBusy).toBe(true);

      // Second frame should be skipped (worker still busy)
      tracker.processFrame(fakeImageData);
      expect(tracker.worker.postMessage).toHaveBeenCalledTimes(1);
    });

    it('clears busy flag on worker result', () => {
      tracker.workerReady = true;
      tracker._workerBusy = true;
      tracker.initKalmanFilter();
      tracker.width = 640;
      tracker.height = 480;

      tracker._onWorkerMessage({ data: { type: 'result', position: { x: 100, y: 200 }, processTime: 1.0 } });
      expect(tracker._workerBusy).toBe(false);
    });
  });

  describe('pending command queue', () => {
    it('queues setROI when worker is not ready', () => {
      tracker.worker = { postMessage: vi.fn() };
      tracker.workerReady = false;
      tracker.fallbackTracker = null;

      const quad = [
        { x: 0, y: 0 }, { x: 640, y: 0 },
        { x: 640, y: 480 }, { x: 0, y: 480 }
      ];
      tracker.setROI(quad);

      expect(tracker.worker.postMessage).not.toHaveBeenCalled();
      expect(tracker._pendingROI).toEqual(quad);
    });

    it('queues setParams when worker is not ready', () => {
      tracker.worker = { postMessage: vi.fn() };
      tracker.workerReady = false;
      tracker.fallbackTracker = null;

      tracker.setParams({ hueMin: 50 });

      expect(tracker.worker.postMessage).not.toHaveBeenCalled();
      expect(tracker._pendingParams).not.toBe(null);
      expect(tracker._pendingParams.hueMin).toBe(50);
    });

    it('flushes pending commands on worker ready', () => {
      tracker.worker = { postMessage: vi.fn() };
      tracker.workerReady = false;
      tracker.fallbackTracker = null;

      const quad = [
        { x: 10, y: 10 }, { x: 600, y: 10 },
        { x: 600, y: 400 }, { x: 10, y: 400 }
      ];
      tracker.setROI(quad);
      tracker.setParams({ hueMin: 60 });

      // Simulate worker ready message
      tracker._onWorkerMessage({ data: { type: 'ready' } });

      expect(tracker.workerReady).toBe(true);
      expect(tracker.worker.postMessage).toHaveBeenCalledWith(
        expect.objectContaining({ type: 'setParams' })
      );
      expect(tracker.worker.postMessage).toHaveBeenCalledWith(
        expect.objectContaining({ type: 'setROI', quad })
      );
      expect(tracker._pendingROI).toBe(null);
      expect(tracker._pendingParams).toBe(null);
    });
  });

  describe('resetTrackingState', () => {
    it('resets all tracking state', () => {
      tracker.isTracking = true;
      tracker.currentPosition = { x: 100, y: 200 };
      tracker.lastPosition = { x: 90, y: 190 };
      tracker.framesSinceLastDetection = 5;
      tracker.kalmanState = {};

      tracker.resetTrackingState();

      expect(tracker.isTracking).toBe(false);
      expect(tracker.currentPosition).toBe(null);
      expect(tracker.lastPosition).toBe(null);
      expect(tracker.predictedPosition).toBe(null);
      expect(tracker.isNewStroke).toBe(true);
      expect(tracker.framesSinceLastDetection).toBe(0);
      expect(tracker.kalmanState).toBe(null);
    });
  });
});
