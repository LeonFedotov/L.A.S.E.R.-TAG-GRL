/**
 * TweakpaneGui - Creates and manages GUI controls using Tweakpane
 * Modern, clean UI replacement for lil-gui
 */
import { Pane } from 'tweakpane';
import * as EssentialsPlugin from '@tweakpane/plugin-essentials';
import { SettingsManager } from './SettingsManager.js';

export class TweakpaneGui {
  constructor(app) {
    this.app = app;
    this.pane = null;
    this.folders = {};
    this.bindings = [];

    // Settings persistence manager
    this.settingsManager = new SettingsManager();

    // Color palette from original L.A.S.E.R. TAG (9 colors)
    this.colorPalette = [
      { name: 'White', hex: '#FFFFFF' },
      { name: 'Pink', hex: '#FF0A47' },
      { name: 'Green', hex: '#47FF0A' },
      { name: 'Magenta', hex: '#FF0AC2' },
      { name: 'Cyan', hex: '#0AC2FF' },
      { name: 'Yellow-Green', hex: '#C2FF0A' },
      { name: 'Blue', hex: '#001CFF' },
      { name: 'Orange', hex: '#FF9E15' },
      { name: 'Black', hex: '#000000' }
    ];

    // GUI state (bound to controls) - load from autosave or use defaults
    const autosaved = this.settingsManager.loadAutosave();
    this.state = autosaved
      ? this.settingsManager.mergeWithDefaults(autosaved)
      : this.settingsManager.getDefaults();

    // Current preset name (for display)
    this.currentPresetName = '';
  }

  /**
   * Initialize the GUI
   * @param {HTMLElement} container - Container element for GUI
   */
  init(container) {
    this.pane = new Pane({
      container,
      title: 'L.A.S.E.R. TAG'
    });

    // Register essentials plugin for additional controls
    this.pane.registerPlugin(EssentialsPlugin);

    // Inject custom styles for color/mode mosaics
    this.injectCustomStyles();

    this.createColorsFolder();
    this.createStylesFolder();
    this.createEffectsFolder();  // Now includes Drips
    this.createCameraFolder();
    this.createTrackingFolder();
    this.createCalibrationFolder();
    this.createDisplayFolder();
    this.createActionsFolder();
    this.createEraseZoneFolder();
    this.createSettingsFolder();  // Settings persistence UI

    // Sync initial state from app
    this.syncFromApp();

    // Push settings to tracker to ensure it starts with correct values
    this.updateTrackerParams();

    // Apply initial drip settings
    this.updateDripParams();

    // Apply loaded settings to app (important when loading autosave)
    this.applyStateToApp();
  }

  /**
   * Apply current GUI state to app components
   */
  applyStateToApp() {
    // Brush settings
    this.app.setBrushColor(this.state.brushColor);
    this.app.setBrushWidth(this.state.brushWidth);
    this.app.setActiveBrush(this.state.brushIndex);

    const brush = this.app.getActiveBrush();
    if (brush.params.mode !== undefined) {
      brush.params.mode = this.state.brushMode;
    }
    if (brush.params.glowIntensity !== undefined) {
      brush.params.glowIntensity = this.state.glowIntensity;
    }
    if (brush.params.shadowOffset !== undefined) {
      brush.params.shadowOffset = this.state.shadowOffset;
    }
    if (brush.params.shadowColor !== undefined) {
      brush.params.shadowColor = this.state.shadowColor;
    }

    // Drips
    this.updateDripParams();

    // Bloom
    if (this.app.postProcessor) {
      this.app.postProcessor.params.bloomEnabled = this.state.bloomEnabled;
      this.app.postProcessor.params.bloomIntensity = this.state.bloomIntensity;
      this.app.postProcessor.params.bloomThreshold = this.state.bloomThreshold;
    }

    // Tracker
    this.updateTrackerParams();

    // Display
    this.app.settings.backgroundColor = this.state.backgroundColor;
    this.app.useMouseInput = this.state.useMouseInput;

    // Camera
    if (this.app.camera) {
      this.app.camera.setFlipH(this.state.flipH);
      this.app.camera.setFlipV(this.state.flipV);
    }

    // Erase zone
    this.app.settings.eraseZoneEnabled = this.state.eraseZoneEnabled;
    this.app.settings.eraseZoneX = this.state.eraseZoneX / 100;
    this.app.settings.eraseZoneY = this.state.eraseZoneY / 100;
    this.app.settings.eraseZoneWidth = this.state.eraseZoneWidth / 100;
    this.app.settings.eraseZoneHeight = this.state.eraseZoneHeight / 100;
  }

  /**
   * Trigger autosave of current state
   */
  triggerAutosave() {
    this.settingsManager.autosave(this.state);
  }

  /**
   * Inject custom CSS for colorful mosaics and compact UI
   */
  injectCustomStyles() {
    const style = document.createElement('style');
    style.textContent = `
      /* Fully floating UI - no sidebar background */
      .tp-dfwv {
        width: 220px !important;
        background: transparent !important;
      }
      .tp-rotv {
        background: transparent !important;
        border: none !important;
      }
      .tp-rotv_t {
        padding: 6px 8px !important;
      }
      /* Individual folders have background */
      .tp-fldv {
        background: rgba(30, 30, 30, 0.85) !important;
        backdrop-filter: blur(8px);
        border-radius: 6px !important;
        margin-bottom: 4px !important;
        box-shadow: 0 2px 10px rgba(0,0,0,0.3) !important;
      }
      .tp-fldv_c {
        background: transparent !important;
      }

      /* Color palette mosaic - smaller */
      .tp-color-mosaic .tp-btnv {
        min-width: 22px !important;
        max-width: 22px !important;
        min-height: 22px !important;
        max-height: 22px !important;
        border-radius: 3px !important;
        border: 2px solid transparent !important;
        transition: border-color 0.15s, transform 0.1s !important;
      }
      .tp-color-mosaic .tp-btnv:hover {
        transform: scale(1.1);
        z-index: 10;
      }
      .tp-color-mosaic .tp-btnv.selected {
        border-color: #fff !important;
        box-shadow: 0 0 6px rgba(255,255,255,0.5);
      }
      .tp-color-mosaic .tp-btnv_b {
        background: var(--btn-color) !important;
        color: transparent !important;
      }
      .tp-color-mosaic .tp-bggv {
        gap: 3px !important;
      }

      /* Mode mosaic - 2 rows of 4, compact */
      .tp-mode-mosaic .tp-btnv {
        min-width: 38px !important;
        max-width: 38px !important;
        min-height: 22px !important;
        max-height: 22px !important;
        border-radius: 3px !important;
        border: 2px solid transparent !important;
        transition: border-color 0.15s !important;
      }
      .tp-mode-mosaic .tp-btnv.selected {
        border-color: #0AC2FF !important;
        box-shadow: 0 0 4px rgba(10,194,255,0.5);
      }
      .tp-mode-mosaic .tp-btnv_b {
        font-size: 8px !important;
        padding: 1px 2px !important;
      }
      .tp-mode-mosaic .tp-bggv {
        gap: 2px !important;
      }

      /* Make debug canvas larger */
      #debug-canvas {
        width: 320px !important;
        height: auto !important;
      }
    `;
    document.head.appendChild(style);
  }

  /**
   * Create colors folder (brush color + shadow color)
   */
  createColorsFolder() {
    const folder = this.pane.addFolder({ title: 'Colors', expanded: false });
    this.folders.colors = folder;

    // Brush color palette
    this.colorPaletteBlade = folder.addBlade({
      view: 'buttongrid',
      size: [3, 3],
      cells: (x, y) => ({
        title: ' '
      }),
      label: 'Brush'
    }).on('click', (ev) => {
      const idx = ev.index[1] * 3 + ev.index[0];
      const color = this.colorPalette[idx];
      this.state.brushColor = color.hex;
      this.state.brushColorIndex = idx;
      this.app.setBrushColor(color.hex);
      this.updateColorSelection();
      this.triggerAutosave();
    });

    // Custom brush color
    folder.addBinding(this.state, 'brushColor', {
      label: 'Custom'
    }).on('change', (ev) => {
      this.app.setBrushColor(ev.value);
      this.triggerAutosave();
    });

    // Shadow color palette
    this.shadowPaletteBlade = folder.addBlade({
      view: 'buttongrid',
      size: [3, 3],
      cells: (x, y) => ({
        title: ' '
      }),
      label: 'Shadow'
    }).on('click', (ev) => {
      const idx = ev.index[1] * 3 + ev.index[0];
      const color = this.colorPalette[idx];
      this.state.shadowColor = color.hex;
      this.state.shadowColorIndex = idx;
      const brush = this.app.getActiveBrush();
      if (brush.params.shadowColor !== undefined) {
        brush.params.shadowColor = color.hex;
      }
      this.updateShadowColorSelection();
      this.triggerAutosave();
    });

    // Custom shadow color
    folder.addBinding(this.state, 'shadowColor', {
      label: 'Custom'
    }).on('change', (ev) => {
      const brush = this.app.getActiveBrush();
      if (brush.params.shadowColor !== undefined) {
        brush.params.shadowColor = ev.value;
      }
      this.triggerAutosave();
    });

    // Apply colors after creation
    setTimeout(() => {
      this.applyPaletteColors();
      this.applyShadowPaletteColors();
    }, 0);
  }

  /**
   * Create styles folder (brush type, mode, width, etc.)
   */
  createStylesFolder() {
    const folder = this.pane.addFolder({ title: 'Styles', expanded: false });
    this.folders.styles = folder;

    // Brush type selection
    const brushList = this.app.getBrushList();
    const brushOptions = brushList.reduce((acc, b) => {
      acc[b.name] = b.index;
      return acc;
    }, {});

    folder.addBinding(this.state, 'brushIndex', {
      label: 'Type',
      options: brushOptions
    }).on('change', (ev) => {
      this.app.setActiveBrush(ev.value);
      this.triggerAutosave();
    });

    // Width slider
    folder.addBinding(this.state, 'brushWidth', {
      label: 'Width',
      min: 2,
      max: 128,
      step: 1
    }).on('change', (ev) => {
      this.app.setBrushWidth(ev.value);
      this.triggerAutosave();
    });

    // Mode names for mosaic
    this.modeList = [
      { name: 'Smooth', value: 'smooth' },
      { name: 'Ribbon', value: 'ribbon' },
      { name: 'Glow', value: 'glow' },
      { name: 'Neon', value: 'neon' },
      { name: 'Basic', value: 'basic' },
      { name: 'Dope', value: 'dope' },
      { name: 'Arrow', value: 'arrow' },
      { name: 'Fat', value: 'arrowFat' }
    ];

    // Mode selection as button grid (4x2)
    this.modeBlade = folder.addBlade({
      view: 'buttongrid',
      size: [4, 2],
      cells: (x, y) => ({
        title: this.modeList[y * 4 + x].name
      }),
      label: 'Mode'
    }).on('click', (ev) => {
      const idx = ev.index[1] * 4 + ev.index[0];
      const mode = this.modeList[idx];
      this.state.brushMode = mode.value;
      const brush = this.app.getActiveBrush();
      if (brush.params.mode !== undefined) {
        brush.params.mode = mode.value;
      }
      this.updateModeSelection();
      this.triggerAutosave();
    });

    // Apply mode mosaic styling after creation
    setTimeout(() => this.applyModeStyles(), 0);

    // Glow intensity (for glow/neon modes)
    folder.addBinding(this.state, 'glowIntensity', {
      label: 'Glow',
      min: 0,
      max: 1,
      step: 0.1
    }).on('change', (ev) => {
      const brush = this.app.getActiveBrush();
      if (brush.params.glowIntensity !== undefined) {
        brush.params.glowIntensity = ev.value;
      }
      this.triggerAutosave();
    });

    // Shadow offset (for C++ modes)
    folder.addBinding(this.state, 'shadowOffset', {
      label: 'Shadow',
      min: 0,
      max: 20,
      step: 1
    }).on('change', (ev) => {
      const brush = this.app.getActiveBrush();
      if (brush.params.shadowOffset !== undefined) {
        brush.params.shadowOffset = ev.value;
      }
      this.triggerAutosave();
    });
  }

  /**
   * Apply colors to palette buttons
   */
  applyPaletteColors() {
    if (!this.colorPaletteBlade) return;

    const container = this.colorPaletteBlade.element;
    container.classList.add('tp-color-mosaic');

    const buttons = container.querySelectorAll('.tp-btnv');
    buttons.forEach((btn, idx) => {
      if (idx < this.colorPalette.length) {
        const color = this.colorPalette[idx];
        btn.style.setProperty('--btn-color', color.hex);
        btn.title = color.name;  // Tooltip with color name
        // Add dark border for light colors
        if (color.hex === '#FFFFFF' || color.hex === '#C2FF0A') {
          btn.style.border = '2px solid #333';
        }
      }
    });

    this.updateColorSelection();
  }

  /**
   * Update color selection highlight
   */
  updateColorSelection() {
    if (!this.colorPaletteBlade) return;

    const container = this.colorPaletteBlade.element;
    const buttons = container.querySelectorAll('.tp-btnv');
    buttons.forEach((btn, idx) => {
      btn.classList.toggle('selected', idx === this.state.brushColorIndex);
    });
  }

  /**
   * Apply mode mosaic styling
   */
  applyModeStyles() {
    if (!this.modeBlade) return;

    const container = this.modeBlade.element;
    container.classList.add('tp-mode-mosaic');

    this.updateModeSelection();
  }

  /**
   * Update mode selection highlight
   */
  updateModeSelection() {
    if (!this.modeBlade) return;

    const container = this.modeBlade.element;
    const buttons = container.querySelectorAll('.tp-btnv');
    buttons.forEach((btn, idx) => {
      if (idx < this.modeList.length) {
        btn.classList.toggle('selected', this.modeList[idx].value === this.state.brushMode);
      }
    });
  }

  /**
   * Apply colors to shadow palette buttons
   */
  applyShadowPaletteColors() {
    if (!this.shadowPaletteBlade) return;

    const container = this.shadowPaletteBlade.element;
    container.classList.add('tp-color-mosaic');

    const buttons = container.querySelectorAll('.tp-btnv');
    buttons.forEach((btn, idx) => {
      if (idx < this.colorPalette.length) {
        const color = this.colorPalette[idx];
        btn.style.setProperty('--btn-color', color.hex);
        btn.title = color.name;  // Tooltip with color name
        if (color.hex === '#FFFFFF' || color.hex === '#C2FF0A') {
          btn.style.border = '2px solid #333';
        }
      }
    });

    this.updateShadowColorSelection();
  }

  /**
   * Update shadow color selection highlight
   */
  updateShadowColorSelection() {
    if (!this.shadowPaletteBlade) return;

    const container = this.shadowPaletteBlade.element;
    const buttons = container.querySelectorAll('.tp-btnv');
    buttons.forEach((btn, idx) => {
      btn.classList.toggle('selected', idx === this.state.shadowColorIndex);
    });
  }

  /**
   * Create effects folder (bloom + drips)
   */
  createEffectsFolder() {
    const folder = this.pane.addFolder({ title: 'Effects', expanded: false });
    this.folders.effects = folder;

    // Drips section
    folder.addBinding(this.state, 'dripsEnabled', {
      label: 'Drips'
    }).on('change', () => { this.updateDripParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'dripsFrequency', {
      label: 'Drip Freq',
      min: 1,
      max: 120,
      step: 1
    }).on('change', () => { this.updateDripParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'dripsSpeed', {
      label: 'Drip Spd',
      min: 0.1,
      max: 12,
      step: 0.1
    }).on('change', () => { this.updateDripParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'dripsDirection', {
      label: 'Drip Dir',
      options: {
        'South': 0,
        'West': 1,
        'North': 2,
        'East': 3
      }
    }).on('change', () => { this.updateDripParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'dripsWidth', {
      label: 'Drip W',
      min: 1,
      max: 25,
      step: 1
    }).on('change', () => { this.updateDripParams(); this.triggerAutosave(); });

    // Bloom section
    folder.addBinding(this.state, 'bloomEnabled', {
      label: 'Bloom'
    }).on('change', (ev) => {
      if (this.app.postProcessor) {
        this.app.postProcessor.params.bloomEnabled = ev.value;
      }
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'bloomIntensity', {
      label: 'Bloom Int',
      min: 0,
      max: 2,
      step: 0.1
    }).on('change', (ev) => {
      if (this.app.postProcessor) {
        this.app.postProcessor.params.bloomIntensity = ev.value;
      }
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'bloomThreshold', {
      label: 'Bloom Thr',
      min: 0,
      max: 1,
      step: 0.05
    }).on('change', (ev) => {
      if (this.app.postProcessor) {
        this.app.postProcessor.params.bloomThreshold = ev.value;
      }
      this.triggerAutosave();
    });
  }

  /**
   * Update drip parameters on all brushes
   */
  updateDripParams() {
    for (const brush of this.app.brushes) {
      if (brush.params.dripsEnabled !== undefined) {
        brush.params.dripsEnabled = this.state.dripsEnabled;
        brush.params.dripsFrequency = this.state.dripsFrequency;
        brush.params.dripsSpeed = this.state.dripsSpeed;
        brush.params.dripsDirection = this.state.dripsDirection;
        brush.params.dripsWidth = this.state.dripsWidth;
      }
    }
  }

  /**
   * Create camera settings folder
   */
  async createCameraFolder() {
    const folder = this.pane.addFolder({ title: 'Camera', expanded: true });
    this.folders.camera = folder;

    // Camera selection dropdown
    try {
      const cameras = await this.app.camera.constructor.getAvailableCameras();
      if (cameras.length > 0) {
        const cameraOptions = {};
        cameras.forEach((cam, i) => {
          const label = cam.label || `Camera ${i + 1}`;
          cameraOptions[label] = cam.deviceId;
        });

        let currentDeviceId = cameras[0].deviceId;
        if (this.app.camera.stream) {
          const videoTrack = this.app.camera.stream.getVideoTracks()[0];
          if (videoTrack) {
            const settings = videoTrack.getSettings();
            if (settings.deviceId) {
              currentDeviceId = settings.deviceId;
            }
          }
        }

        this.state.selectedCamera = currentDeviceId;
        folder.addBinding(this.state, 'selectedCamera', {
          label: 'Device',
          options: cameraOptions
        }).on('change', async (ev) => {
          try {
            await this.app.camera.switchCamera(ev.value);
            this.app.debugCanvas.width = this.app.camera.width;
            this.app.debugCanvas.height = this.app.camera.height;
            this.app.captureCanvas.width = this.app.camera.width;
            this.app.captureCanvas.height = this.app.camera.height;
            console.log('Switched to camera:', ev.value);
            this.triggerAutosave();
          } catch (e) {
            console.error('Failed to switch camera:', e);
          }
        });
      }
    } catch (e) {
      console.warn('Could not enumerate cameras:', e);
    }

    // Resolution selector
    this.state.resolution = '640x480';
    folder.addBinding(this.state, 'resolution', {
      label: 'Resolution',
      options: {
        '640×480 (VGA)': '640x480',
        '1280×720 (HD)': '1280x720',
        '1920×1080 (Full HD)': '1920x1080'
      }
    }).on('change', async (ev) => {
      const [w, h] = ev.value.split('x').map(Number);
      try {
        await this.app.camera.setResolution(w, h);
        this.app.debugCanvas.width = this.app.camera.width;
        this.app.debugCanvas.height = this.app.camera.height;
        this.app.captureCanvas.width = this.app.camera.width;
        this.app.captureCanvas.height = this.app.camera.height;
        this.app.tracker.init(this.app.camera.width, this.app.camera.height);
        console.log('Resolution changed to:', this.app.camera.width, 'x', this.app.camera.height);
        this.triggerAutosave();
      } catch (e) {
        console.error('Failed to change resolution:', e);
      }
    });

    // Flip controls
    folder.addBinding(this.state, 'flipH', {
      label: 'Flip Horizontal'
    }).on('change', (ev) => {
      this.app.camera.setFlipH(ev.value);
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'flipV', {
      label: 'Flip Vertical'
    }).on('change', (ev) => {
      this.app.camera.setFlipV(ev.value);
      this.triggerAutosave();
    });
  }

  /**
   * Create tracking settings folder
   */
  async createTrackingFolder() {
    const folder = this.pane.addFolder({ title: 'Laser Detection', expanded: false });
    this.folders.tracking = folder;

    // Preset selector
    folder.addBinding(this.state, 'trackerPreset', {
      label: 'Preset',
      options: {
        'Green Laser': 'Green Laser',
        'Red Laser': 'Red Laser',
        'Blue Laser': 'Blue Laser',
        'White/Bright': 'White/Bright'
      }
    }).on('change', (ev) => {
      const presets = {
        'Green Laser': [35, 85, 50, 255, 200, 255],
        'Red Laser': [0, 15, 100, 255, 200, 255],
        'Blue Laser': [100, 130, 100, 255, 200, 255],
        'White/Bright': [0, 180, 0, 50, 240, 255]
      };
      const p = presets[ev.value];
      if (p) {
        this.applyTrackerPreset(...p);
      }
      this.triggerAutosave();
    });

    // HSV color preview (computed from min/max values)
    this.state.hsvPreviewMin = this.hsvToHex(this.state.hueMin, this.state.satMin, this.state.valMin);
    this.state.hsvPreviewMax = this.hsvToHex(this.state.hueMax, this.state.satMax, this.state.valMax);

    folder.addBinding(this.state, 'hsvPreviewMin', {
      label: 'Min Color',
      view: 'color',
      disabled: true
    });

    folder.addBinding(this.state, 'hsvPreviewMax', {
      label: 'Max Color',
      view: 'color',
      disabled: true
    });

    // HSV range controls
    folder.addBinding(this.state, 'hueMin', {
      label: 'Hue Min',
      min: 0,
      max: 180,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'hueMax', {
      label: 'Hue Max',
      min: 0,
      max: 180,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'satMin', {
      label: 'Sat Min',
      min: 0,
      max: 255,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'satMax', {
      label: 'Sat Max',
      min: 0,
      max: 255,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'valMin', {
      label: 'Val Min',
      min: 0,
      max: 255,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'valMax', {
      label: 'Val Max',
      min: 0,
      max: 255,
      step: 1
    }).on('change', () => {
      this.updateTrackerParams();
      this.updateHsvPreviews();
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'smoothing', {
      label: 'Smoothing',
      min: 0,
      max: 1,
      step: 0.05
    }).on('change', () => { this.updateTrackerParams(); this.triggerAutosave(); });
  }

  /**
   * Create calibration folder
   */
  createCalibrationFolder() {
    const folder = this.pane.addFolder({ title: 'Calibration', expanded: false });
    this.folders.calibration = folder;

    // Camera calibration (input)
    folder.addBlade({
      view: 'text',
      label: '',
      parse: (v) => String(v),
      value: '--- Camera (Input) ---'
    });

    folder.addButton({ title: 'Camera Calib (Space)' }).on('click', () => {
      this.toggleCalibration();
    });

    folder.addButton({ title: 'Reset Camera' }).on('click', () => {
      this.app.resetCalibration();
    });

    // Projector calibration (output)
    folder.addBlade({
      view: 'text',
      label: '',
      parse: (v) => String(v),
      value: '--- Projector (Output) ---'
    });

    folder.addButton({ title: 'Projector Calib (P)' }).on('click', () => {
      this.app.toggleProjectorCalibration();
    });

    folder.addButton({ title: 'Reset Projector' }).on('click', () => {
      this.app.resetProjectorCalibration();
    });

    // Save all
    folder.addButton({ title: 'Save All (Ctrl+S)' }).on('click', () => {
      this.app.saveCalibration();
      this.app.saveProjectorCalibration();
    });
  }

  /**
   * Create display settings folder
   */
  createDisplayFolder() {
    const folder = this.pane.addFolder({ title: 'Display', expanded: false });
    this.folders.display = folder;

    folder.addBinding(this.state, 'backgroundColor', {
      label: 'Background'
    }).on('change', (ev) => {
      this.app.settings.backgroundColor = ev.value;
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'useMouseInput', {
      label: 'Mouse (M)'
    }).on('change', (ev) => {
      this.app.useMouseInput = ev.value;
      this.triggerAutosave();
    });

    folder.addButton({ title: 'Fullscreen' }).on('click', () => {
      this.toggleFullscreen();
    });
  }

  /**
   * Create actions folder
   */
  createActionsFolder() {
    const folder = this.pane.addFolder({ title: 'Actions', expanded: true });
    this.folders.actions = folder;

    folder.addButton({ title: 'Clear Canvas (C)' }).on('click', () => {
      this.app.clearCanvas();
    });

    folder.addButton({ title: 'Undo' }).on('click', () => {
      this.app.undo();
    });

    folder.addButton({ title: 'Projector Window' }).on('click', () => {
      this.openProjectorWindow();
    });
  }

  /**
   * Create erase zone folder
   */
  createEraseZoneFolder() {
    const folder = this.pane.addFolder({ title: 'Erase Zone', expanded: false });
    this.folders.eraseZone = folder;

    folder.addBinding(this.state, 'eraseZoneEnabled', {
      label: 'Enabled'
    }).on('change', (ev) => {
      this.app.settings.eraseZoneEnabled = ev.value;
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneX', {
      label: 'X (%)',
      min: 0,
      max: 100,
      step: 1
    }).on('change', (ev) => {
      this.app.settings.eraseZoneX = ev.value / 100;
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneY', {
      label: 'Y (%)',
      min: 0,
      max: 100,
      step: 1
    }).on('change', (ev) => {
      this.app.settings.eraseZoneY = ev.value / 100;
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneWidth', {
      label: 'Width (%)',
      min: 5,
      max: 50,
      step: 1
    }).on('change', (ev) => {
      this.app.settings.eraseZoneWidth = ev.value / 100;
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneHeight', {
      label: 'Height (%)',
      min: 5,
      max: 50,
      step: 1
    }).on('change', (ev) => {
      this.app.settings.eraseZoneHeight = ev.value / 100;
      this.triggerAutosave();
    });
  }

  /**
   * Create settings management folder (save/load/reset presets)
   */
  createSettingsFolder() {
    const folder = this.pane.addFolder({ title: 'Settings', expanded: false });
    this.folders.settings = folder;

    // Preset selector state
    this.presetState = {
      selectedPreset: '',
      presetName: ''
    };

    // Load preset dropdown
    this.updatePresetDropdown(folder);

    // Save preset name input
    folder.addBinding(this.presetState, 'presetName', {
      label: 'Preset Name'
    });

    // Save button
    folder.addButton({ title: 'Save Preset' }).on('click', () => {
      const name = this.presetState.presetName.trim();
      if (!name) {
        alert('Please enter a preset name');
        return;
      }

      // Confirm if preset exists
      const existing = this.settingsManager.getPresetsList();
      if (existing.includes(name)) {
        if (!confirm(`Preset "${name}" already exists. Overwrite?`)) {
          return;
        }
      }

      if (this.settingsManager.savePreset(name, this.state)) {
        this.currentPresetName = name;
        this.updatePresetDropdown(folder);
        alert(`Preset "${name}" saved`);
      } else {
        alert('Failed to save preset');
      }
    });

    // Load button
    folder.addButton({ title: 'Load Selected' }).on('click', () => {
      const name = this.presetState.selectedPreset;
      if (!name) {
        alert('Please select a preset to load');
        return;
      }

      const settings = this.settingsManager.loadPreset(name);
      if (settings) {
        this.loadSettings(settings);
        this.currentPresetName = name;
        alert(`Preset "${name}" loaded`);
      } else {
        alert(`Failed to load preset "${name}"`);
      }
    });

    // Delete button
    folder.addButton({ title: 'Delete Selected' }).on('click', () => {
      const name = this.presetState.selectedPreset;
      if (!name) {
        alert('Please select a preset to delete');
        return;
      }

      if (!confirm(`Delete preset "${name}"?`)) {
        return;
      }

      if (this.settingsManager.deletePreset(name)) {
        this.presetState.selectedPreset = '';
        this.updatePresetDropdown(folder);
        alert(`Preset "${name}" deleted`);
      } else {
        alert(`Failed to delete preset "${name}"`);
      }
    });

    // Separator
    folder.addBlade({
      view: 'separator'
    });

    // Reset to defaults button
    folder.addButton({ title: 'Reset to Defaults' }).on('click', () => {
      if (!confirm('Reset all settings to defaults?')) {
        return;
      }

      this.loadSettings(this.settingsManager.getDefaults());
      this.currentPresetName = '';
      this.settingsManager.clearAutosave();
      alert('Settings reset to defaults');
    });

    // Export/Import separator
    folder.addBlade({
      view: 'separator'
    });

    // Export button
    folder.addButton({ title: 'Export to File' }).on('click', () => {
      const json = this.settingsManager.exportToJson(this.state);
      const blob = new Blob([json], { type: 'application/json' });
      const url = URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = `laser-tag-settings-${Date.now()}.json`;
      a.click();
      URL.revokeObjectURL(url);
    });

    // Import button
    folder.addButton({ title: 'Import from File' }).on('click', () => {
      const input = document.createElement('input');
      input.type = 'file';
      input.accept = '.json';
      input.onchange = (e) => {
        const file = e.target.files[0];
        if (!file) return;

        const reader = new FileReader();
        reader.onload = (ev) => {
          const settings = this.settingsManager.importFromJson(ev.target.result);
          if (settings) {
            this.loadSettings(settings);
            alert('Settings imported successfully');
          } else {
            alert('Failed to import settings - invalid file');
          }
        };
        reader.readAsText(file);
      };
      input.click();
    });
  }

  /**
   * Update the preset dropdown with current presets list
   */
  updatePresetDropdown(folder) {
    // Remove existing preset binding if it exists
    if (this.presetBinding) {
      this.presetBinding.dispose();
    }

    const presets = this.settingsManager.getPresetsList();
    const options = { '(none)': '' };
    presets.forEach(name => {
      options[name] = name;
    });

    // Add as first item in folder
    this.presetBinding = folder.addBinding(this.presetState, 'selectedPreset', {
      label: 'Load Preset',
      options: options,
      index: 0
    });
  }

  /**
   * Load settings into GUI state and apply to app
   */
  loadSettings(settings) {
    // Merge with defaults to ensure all keys exist
    const merged = this.settingsManager.mergeWithDefaults(settings);

    // Update state
    Object.assign(this.state, merged);

    // Apply to app
    this.applyStateToApp();

    // Update UI selections
    this.updateColorSelection();
    this.updateShadowColorSelection();
    this.updateModeSelection();
    this.updateHsvPreviews();

    // Refresh all UI bindings
    this.pane.refresh();

    // Autosave the loaded settings
    this.triggerAutosave();
  }

  /**
   * Open projector window for secondary display
   */
  openProjectorWindow() {
    if (this.projectorWindow && !this.projectorWindow.closed) {
      this.projectorWindow.focus();
      return;
    }

    this.projectorWindow = window.open('', 'projector',
      'width=1280,height=720,menubar=no,toolbar=no,location=no,status=no');

    if (!this.projectorWindow) {
      alert('Popup blocked! Please allow popups for this site.');
      return;
    }

    this.projectorWindow.document.write(`
      <!DOCTYPE html>
      <html>
      <head>
        <title>L.A.S.E.R. TAG - Projector</title>
        <style>
          * { margin: 0; padding: 0; box-sizing: border-box; }
          body {
            background: #000;
            overflow: hidden;
            cursor: none;
            width: 100vw;
            height: 100vh;
            position: relative;
          }
          #canvas-container {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            overflow: visible;
          }
          canvas {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            display: block;
            transform-origin: 0 0;
          }
          .instructions {
            position: fixed;
            bottom: 10px;
            left: 10px;
            color: #333;
            font-family: monospace;
            font-size: 12px;
            z-index: 100;
            pointer-events: none;
          }
        </style>
      </head>
      <body>
        <div id="canvas-container">
          <canvas id="projector-canvas"></canvas>
        </div>
        <div class="instructions">Press F for fullscreen, P for calibration, ESC to exit</div>
      </body>
      </html>
    `);

    const canvas = this.projectorWindow.document.getElementById('projector-canvas');
    const ctx = canvas.getContext('2d');

    const resize = () => {
      canvas.width = this.projectorWindow.innerWidth;
      canvas.height = this.projectorWindow.innerHeight;
    };
    resize();
    this.projectorWindow.addEventListener('resize', resize);

    this.projectorWindow.addEventListener('keydown', (e) => {
      if (e.key === 'f' || e.key === 'F') {
        canvas.requestFullscreen().catch(() => {});
      }
      if (e.key === 'p' || e.key === 'P') {
        this.app.toggleProjectorCalibration();
      }
      if (e.key === 's' && (e.ctrlKey || e.metaKey)) {
        this.app.saveProjectorCalibration();
        e.preventDefault();
      }
    });

    // Mouse handlers for projector calibration on popup window
    let popupDragging = false;

    canvas.addEventListener('mousedown', (e) => {
      if (!this.app.isProjectorCalibrating) return;
      const idx = this.app.selectProjectorPoint(e.clientX, e.clientY, canvas);
      if (idx >= 0) {
        popupDragging = true;
      }
    });

    canvas.addEventListener('mousemove', (e) => {
      if (!this.app.isProjectorCalibrating || !popupDragging) return;
      if (this.app.projectorSelectedPoint >= 0) {
        this.app.moveProjectorPoint(this.app.projectorSelectedPoint, e.clientX, e.clientY, canvas);
      }
    });

    canvas.addEventListener('mouseup', () => {
      popupDragging = false;
      this.app.projectorSelectedPoint = -1;
    });

    canvas.addEventListener('mouseleave', () => {
      popupDragging = false;
    });

    this.app.projectorPopup = {
      window: this.projectorWindow,
      canvas: canvas,
      ctx: ctx
    };

    console.log('Projector window opened. Press F for fullscreen, P for calibration.');
  }

  /**
   * Sync GUI state from app
   */
  syncFromApp() {
    const trackerParams = this.app.getTrackerParams();

    this.state.hueMin = trackerParams.hueMin;
    this.state.hueMax = trackerParams.hueMax;
    this.state.satMin = trackerParams.satMin;
    this.state.satMax = trackerParams.satMax;
    this.state.valMin = trackerParams.valMin;
    this.state.valMax = trackerParams.valMax;
    this.state.smoothing = trackerParams.smoothing;
    this.state.showDebug = trackerParams.showDebug;

    this.app.setBrushColor(this.state.brushColor);
    this.app.setBrushWidth(this.state.brushWidth);

    const brush = this.app.getActiveBrush();
    if (brush.params.mode !== undefined) {
      brush.params.mode = this.state.brushMode;
    }
    if (brush.params.glowIntensity !== undefined) {
      brush.params.glowIntensity = this.state.glowIntensity;
    }
    if (brush.params.dripsEnabled !== undefined) {
      brush.params.dripsEnabled = this.state.dripsEnabled;
    }

    this.pane.refresh();
  }

  /**
   * Update tracker parameters from GUI state
   */
  updateTrackerParams() {
    this.app.setTrackerParams({
      hueMin: this.state.hueMin,
      hueMax: this.state.hueMax,
      satMin: this.state.satMin,
      satMax: this.state.satMax,
      valMin: this.state.valMin,
      valMax: this.state.valMax,
      smoothing: this.state.smoothing
    });
  }

  /**
   * Apply a tracker preset
   */
  applyTrackerPreset(hueMin, hueMax, satMin, satMax, valMin, valMax) {
    this.state.hueMin = hueMin;
    this.state.hueMax = hueMax;
    this.state.satMin = satMin;
    this.state.satMax = satMax;
    this.state.valMin = valMin;
    this.state.valMax = valMax;

    this.updateTrackerParams();
    this.pane.refresh();
  }

  /**
   * Toggle calibration mode
   */
  toggleCalibration() {
    const isCalibrating = this.app.toggleCalibration();
    console.log('Calibration mode:', isCalibrating ? 'ON' : 'OFF');
  }

  /**
   * Convert HSV (OpenCV format: H 0-180, S 0-255, V 0-255) to hex color
   */
  hsvToHex(h, s, v) {
    // Convert from OpenCV HSV range to standard (H 0-360, S 0-1, V 0-1)
    const hNorm = (h / 180) * 360;
    const sNorm = s / 255;
    const vNorm = v / 255;

    const c = vNorm * sNorm;
    const x = c * (1 - Math.abs((hNorm / 60) % 2 - 1));
    const m = vNorm - c;

    let r, g, b;
    if (hNorm < 60) { r = c; g = x; b = 0; }
    else if (hNorm < 120) { r = x; g = c; b = 0; }
    else if (hNorm < 180) { r = 0; g = c; b = x; }
    else if (hNorm < 240) { r = 0; g = x; b = c; }
    else if (hNorm < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    const toHex = (n) => Math.round((n + m) * 255).toString(16).padStart(2, '0');
    return `#${toHex(r)}${toHex(g)}${toHex(b)}`;
  }

  /**
   * Update HSV color previews
   */
  updateHsvPreviews() {
    this.state.hsvPreviewMin = this.hsvToHex(this.state.hueMin, this.state.satMin, this.state.valMin);
    this.state.hsvPreviewMax = this.hsvToHex(this.state.hueMax, this.state.satMax, this.state.valMax);
    this.pane.refresh();
  }

  /**
   * Toggle fullscreen
   */
  toggleFullscreen() {
    const projector = this.app.projectorCanvas;

    if (!document.fullscreenElement) {
      projector.requestFullscreen().catch(err => {
        console.error('Fullscreen error:', err);
      });
    } else {
      document.exitFullscreen();
    }
  }

  /**
   * Sync mouse input state from app
   */
  setMouseInputState(enabled) {
    this.state.useMouseInput = enabled;
    this.pane.refresh();
  }

  /**
   * Dispose of GUI
   */
  dispose() {
    if (this.pane) {
      this.pane.dispose();
      this.pane = null;
    }
  }
}
