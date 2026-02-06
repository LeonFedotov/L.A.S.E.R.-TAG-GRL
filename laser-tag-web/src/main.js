/**
 * L.A.S.E.R. TAG - Browser Edition
 * Main entry point
 *
 * Based on the original L.A.S.E.R. TAG by Graffiti Research Lab
 * Modernized for the web browser using JavaScript, WebRTC, and OpenCV.js
 */

import { AppController } from './app/AppController.js';
import { TweakpaneGui } from './app/TweakpaneGui.js';

// Global application state
let app = null;
let gui = null;

/**
 * Wait for OpenCV.js to load
 */
async function waitForOpenCV(timeout = 30000) {
  const startTime = Date.now();

  return new Promise((resolve, reject) => {
    const check = () => {
      if (typeof cv !== 'undefined' && cv.Mat) {
        console.log('OpenCV.js loaded successfully');
        resolve();
      } else if (Date.now() - startTime > timeout) {
        reject(new Error('OpenCV.js load timeout'));
      } else {
        setTimeout(check, 100);
      }
    };
    check();
  });
}

/**
 * Initialize the application
 */
async function initApp() {
  const loadingOverlay = document.getElementById('loading-overlay');
  const loadingStatus = document.getElementById('loading-status');
  const errorMessage = document.getElementById('error-message');

  try {
    // Update status
    loadingStatus.textContent = 'Loading OpenCV.js...';

    // Wait for OpenCV
    await waitForOpenCV();

    loadingStatus.textContent = 'Initializing camera...';

    // Get DOM elements
    const elements = {
      projectorCanvas: document.getElementById('projector-canvas'),
      projectorCloneCanvas: document.getElementById('projector-clone-canvas'),
      debugCanvas: document.getElementById('debug-canvas'),
      warpedCameraCanvas: document.getElementById('warped-camera-canvas'),
      videoElement: document.getElementById('video-element')
    };

    // Create app controller
    app = new AppController();

    // Initialize app
    await app.init(elements);

    loadingStatus.textContent = 'Setting up controls...';

    // Create GUI (Tweakpane)
    gui = new TweakpaneGui(app);
    gui.init(document.getElementById('gui-container'));

    // Set up event listeners
    setupEventListeners();

    // Set up state change callbacks
    app.onStateChange = handleStateChange;

    // Sync initial status bar state (settings loaded before callback was set)
    handleStateChange('mouseInput', app.useMouseInput);
    handleStateChange('mode', app.getMode());

    // Hide loading overlay
    loadingOverlay.classList.add('hidden');

    // Start the main loop
    app.start();

    console.log('L.A.S.E.R. TAG initialized successfully');

  } catch (error) {
    console.error('Initialization error:', error);
    errorMessage.textContent = `Error: ${error.message}. Make sure you allow camera access.`;
    loadingStatus.textContent = '';
  }
}

/**
 * Set up event listeners (called after app init)
 */
function setupEventListeners() {
  // Window resize
  window.addEventListener('resize', () => {
    if (app) {
      app.handleResize();
    }
  });

  // Keyboard shortcuts
  document.addEventListener('keydown', handleKeyDown);

  // Debug canvas mouse events for calibration
  const debugCanvas = document.getElementById('debug-canvas');
  let selectedPoint = -1;

  debugCanvas.addEventListener('mousedown', (e) => {
    if (!app || !app.isCalibrating) return;

    const rect = debugCanvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    selectedPoint = app.selectCalibrationPoint(x, y);
  });

  debugCanvas.addEventListener('mousemove', (e) => {
    if (!app || selectedPoint < 0) return;

    const rect = debugCanvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    app.moveCalibrationPoint(selectedPoint, x, y);
  });

  debugCanvas.addEventListener('mouseup', () => {
    selectedPoint = -1;
  });

  debugCanvas.addEventListener('mouseleave', () => {
    selectedPoint = -1;
  });

  // Color picker: click on camera preview to sample pixel and set HSV range
  debugCanvas.addEventListener('click', (e) => {
    if (!app || app.isCalibrating) return;

    const rect = debugCanvas.getBoundingClientRect();
    const scaleX = app.camera.width / rect.width;
    const scaleY = app.camera.height / rect.height;
    const camX = (e.clientX - rect.left) * scaleX;
    const camY = (e.clientY - rect.top) * scaleY;

    const hsv = app.samplePixelHSV(camX, camY);
    if (!hsv) return;

    // Set HSV range with tolerance around the sampled value
    const hueMargin = 15;
    const satMargin = 60;
    const valMargin = 60;

    const hueMin = Math.max(0, hsv.h - hueMargin);
    const hueMax = Math.min(180, hsv.h + hueMargin);
    const satMin = Math.max(0, hsv.s - satMargin);
    const satMax = Math.min(255, hsv.s + satMargin);
    const valMin = Math.max(0, hsv.v - valMargin);
    const valMax = Math.min(255, hsv.v + valMargin);

    if (gui) {
      gui.applyTrackerPreset(hueMin, hueMax, satMin, satMax, valMin, valMax);
    }

    console.log(`Color picked: HSV(${hsv.h}, ${hsv.s}, ${hsv.v}) → range H[${hueMin}-${hueMax}] S[${satMin}-${satMax}] V[${valMin}-${valMax}]`);
  });

  // Fullscreen change
  document.addEventListener('fullscreenchange', () => {
    if (app) {
      setTimeout(() => app.handleResize(), 100);
    }
  });

  // Projector canvas mouse events for mouse input mode and projector calibration
  const projectorCanvas = document.getElementById('projector-canvas');
  let projectorDragging = false;

  projectorCanvas.addEventListener('mousedown', (e) => {
    if (!app) return;
    // Check for projector calibration first
    if (app.isProjectorCalibrating) {
      const idx = app.selectProjectorPoint(e.clientX, e.clientY);
      if (idx >= 0) {
        projectorDragging = true;
      }
    } else {
      app.handleMouseDown(e);
    }
  });

  projectorCanvas.addEventListener('mousemove', (e) => {
    if (!app) return;
    if (app.isProjectorCalibrating && projectorDragging && app.projectorSelectedPoint >= 0) {
      app.moveProjectorPoint(app.projectorSelectedPoint, e.clientX, e.clientY);
    } else if (!app.isProjectorCalibrating) {
      app.handleMouseMove(e);
    }
  });

  projectorCanvas.addEventListener('mouseup', () => {
    if (!app) return;
    projectorDragging = false;
    app.projectorSelectedPoint = -1;
    if (!app.isProjectorCalibrating) {
      app.handleMouseUp();
    }
  });

  projectorCanvas.addEventListener('mouseleave', () => {
    if (!app) return;
    projectorDragging = false;
    if (!app.isProjectorCalibrating) {
      app.handleMouseUp();
    }
  });

  // Projector window button
  document.getElementById('projector-btn').addEventListener('click', () => {
    if (gui) {
      gui.openProjectorWindow();
    }
  });
}

/**
 * Handle keyboard shortcuts
 */
function handleKeyDown(e) {
  if (!app) return;

  switch (e.key.toLowerCase()) {
    case 'c':
      // Clear canvas
      app.clearCanvas();
      break;

    case 'z':
      if (e.ctrlKey || e.metaKey) {
        // Undo
        app.undo();
      }
      break;

    case ' ':
      // Toggle camera calibration
      app.toggleCalibration();
      e.preventDefault();
      break;

    case 'p':
      // Toggle projector calibration
      app.toggleProjectorCalibration();
      break;

    case 's':
      if (e.ctrlKey || e.metaKey) {
        // Save both calibrations
        app.saveCalibration();
        app.saveProjectorCalibration();
        e.preventDefault();
      }
      break;

    case 'f':
      // Toggle fullscreen
      if (gui) {
        gui.toggleFullscreen();
      }
      break;

    case 'd':
      // Toggle debug view
      const debugCanvas = document.getElementById('debug-canvas');
      if (debugCanvas) {
        const isVisible = debugCanvas.style.display !== 'none';
        debugCanvas.style.display = isVisible ? 'none' : 'block';
        app.settings.showDebug = !isVisible;
      }
      break;

    case 'm':
      // Toggle mouse input mode
      const mouseMode = app.toggleMouseInput();
      console.log('Mouse input mode:', mouseMode ? 'ON' : 'OFF');
      // Sync GUI checkbox
      if (gui && gui.setMouseInputState) {
        gui.setMouseInputState(mouseMode);
      } else if (gui && gui.state) {
        gui.state.useMouseInput = mouseMode;
        if (gui.pane) gui.pane.refresh();
      }
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
      // Switch mode
      const modeIndex = parseInt(e.key) - 1;
      app.setModeByIndex(modeIndex);
      break;

    case 'arrowup':
      // Increase brush size
      app.setBrushWidth(app.getActiveBrush().params.brushWidth + 2);
      break;

    case 'arrowdown':
      // Decrease brush size
      app.setBrushWidth(Math.max(1, app.getActiveBrush().params.brushWidth - 2));
      break;
  }
}

/**
 * Handle state changes from app
 */
function handleStateChange(key, value) {
  switch (key) {
    case 'tracking':
      const trackingEl = document.querySelector('#status-tracking span');
      if (trackingEl) {
        trackingEl.textContent = value ? 'ON' : 'OFF';
        trackingEl.className = value ? 'active' : 'warning';
      }
      break;

    case 'fps':
      const fpsEl = document.querySelector('#status-fps span');
      if (fpsEl) {
        fpsEl.textContent = value;
      }
      break;

    case 'position':
      const posEl = document.querySelector('#status-position span');
      if (posEl) {
        posEl.textContent = `${value.x}, ${value.y}`;
      }
      break;

    case 'mode':
      const modeEl = document.querySelector('#status-mode span');
      if (modeEl) {
        modeEl.textContent = value;
      }
      break;

    case 'mouseInput':
      const mouseEl = document.querySelector('#status-mouse span');
      if (mouseEl) {
        mouseEl.textContent = value ? 'ON' : 'OFF';
        mouseEl.className = value ? 'active' : '';
      }
      break;
  }
}

/**
 * Clean up on page unload
 */
window.addEventListener('beforeunload', () => {
  if (app) {
    app.dispose();
  }
  if (gui) {
    gui.dispose();
  }
});

// Export for debugging
window.laserTag = {
  get app() { return app; },
  get gui() { return gui; }
};

console.log('L.A.S.E.R. TAG - Browser Edition');
console.log('Press START to begin');
console.log('Keyboard shortcuts: C=Clear, Z=Undo, Space=Calibrate, F=Fullscreen, D=Debug, 1-4=Brush');

// Attach start button listener immediately on module load
document.getElementById('start-button').addEventListener('click', initApp);
