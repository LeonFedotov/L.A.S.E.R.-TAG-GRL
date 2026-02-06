/**
 * TweakpaneGui - Creates and manages GUI controls using Tweakpane
 * Modern, clean UI replacement for lil-gui
 */
import { Pane } from 'tweakpane';
import * as EssentialsPlugin from '@tweakpane/plugin-essentials';
import { SettingsManager } from './SettingsManager.js';
import { AppGuiAdapter } from './AppGuiAdapter.js';

// Channel name for popup window communication
const POPUP_CHANNEL = 'laserTagProjectorChannel';
const POPUP_OPEN_KEY = 'laserTag_projectorPopupOpen';

export class TweakpaneGui {
  constructor(app) {
    this.app = app;
    this.adapter = new AppGuiAdapter(app);
    this.pane = null;
    this.folders = {};
    this.bindings = [];
    this.projectorWindow = null;

    // BroadcastChannel for popup reconnection
    this.popupChannel = new BroadcastChannel(POPUP_CHANNEL);
    this.setupPopupChannelListener();

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

    // GUI state - load from autosave or use defaults
    const autosaved = this.settingsManager.loadAutosave();
    this.state = autosaved
      ? this.settingsManager.mergeWithDefaults(autosaved)
      : this.settingsManager.getDefaults();

    // Current preset name (for display)
    this.currentPresetName = '';
  }

  /**
   * Set up BroadcastChannel listener for popup reconnection
   */
  setupPopupChannelListener() {
    this.popupChannel.onmessage = (event) => {
      if (event.data.type === 'popup-ready' && !this.projectorWindow) {
        // Popup is ready - try to reconnect (only if we don't already have a reference)
        this.tryReconnectPopup();
      }
    };
  }

  /**
   * Try to reconnect to an existing popup window after page refresh
   */
  tryReconnectPopup() {
    // Already connected
    if (this.projectorWindow && !this.projectorWindow.closed) {
      return;
    }

    // Check if a popup was open before refresh
    if (localStorage.getItem(POPUP_OPEN_KEY) !== 'true') {
      return;
    }

    // Use opener relationship or find by name without focusing
    // window.open with empty URL and same name returns reference without focusing
    try {
      const existingPopup = window.open('', 'laserTagProjector', 'noopener=false');
      if (existingPopup && !existingPopup.closed) {
        const canvas = existingPopup.document.getElementById('projector-canvas');
        const overlayCanvas = existingPopup.document.getElementById('overlay-canvas');
        const container = existingPopup.document.getElementById('canvas-container');
        if (canvas && overlayCanvas) {
          this.projectorWindow = existingPopup;
          const ctx = canvas.getContext('2d');
          const overlayCtx = overlayCanvas.getContext('2d');

          // Re-register event handlers
          this.setupPopupEventHandlers(existingPopup, canvas, overlayCanvas, container);

          // Update adapter with popup reference
          this.adapter.setProjectorPopup({
            window: existingPopup,
            canvas: canvas,
            ctx: ctx,
            overlayCanvas: overlayCanvas,
            overlayCtx: overlayCtx,
            container: container
          });

          // Tell popup to stop sending ready messages
          this.popupChannel.postMessage({ type: 'connected' });

          // Return focus to main window
          window.focus();

          console.log('Reconnected to existing projector window');
          return;
        }
      }
    } catch (e) {
      // Cross-origin or other error
    }

    // Popup was closed or doesn't exist anymore
    localStorage.removeItem(POPUP_OPEN_KEY);
  }

  /**
   * Trigger autosave of current state
   */
  triggerAutosave() {
    this.settingsManager.autosave(this.state);
  }

  /**
   * Apply current GUI state to app components (used when loading autosave)
   */
  applyStateToApp() {
    // Brush settings via adapter
    this.adapter.setBrushColor(this.state.brushColor);
    this.adapter.setBrushWidth(this.state.brushWidth);
    this.adapter.setActiveBrush(this.state.brushIndex);

    const brush = this.adapter.getActiveBrush();
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

    // Bloom via adapter
    this.adapter.setBloomEnabled(this.state.bloomEnabled);
    this.adapter.setBloomIntensity(this.state.bloomIntensity);
    this.adapter.setBloomThreshold(this.state.bloomThreshold);

    // Tracker
    this.updateTrackerParams();

    // Display
    this.adapter.setBackgroundColor(this.state.backgroundColor);
    this.adapter.setMouseInputEnabled(this.state.useMouseInput);

    // Camera
    this.adapter.setCameraFlipH(this.state.flipH);
    this.adapter.setCameraFlipV(this.state.flipV);
    this.adapter.setCameraRotation(this.state.rotation);

    // Erase zone
    this.adapter.setEraseZone({
      enabled: this.state.eraseZoneEnabled,
      x: this.state.eraseZoneX / 100,
      y: this.state.eraseZoneY / 100,
      width: this.state.eraseZoneWidth / 100,
      height: this.state.eraseZoneHeight / 100
    });
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

    // Update HSV color previews
    this.updateHsvPreviews();

    // Apply initial drip settings
    this.updateDripParams();

    // Apply loaded settings to app (important when loading autosave)
    this.applyStateToApp();

    // Try to reconnect to an existing projector popup window
    this.tryReconnectPopup();
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
      this.adapter.setBrushColor(color.hex);
      this.updateColorSelection();
      this.triggerAutosave();
    });

    // Custom brush color
    folder.addBinding(this.state, 'brushColor', {
      label: 'Custom'
    }).on('change', (ev) => {
      this.adapter.setBrushColor(ev.value);
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
      const brush = this.adapter.getActiveBrush();
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
      const brush = this.adapter.getActiveBrush();
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
    const brushList = this.adapter.getBrushList();
    const brushOptions = brushList.reduce((acc, b) => {
      acc[b.name] = b.index;
      return acc;
    }, {});

    folder.addBinding(this.state, 'brushIndex', {
      label: 'Type',
      options: brushOptions
    }).on('change', (ev) => {
      this.adapter.setActiveBrush(ev.value);
      this.triggerAutosave();
    });

    // Width slider
    folder.addBinding(this.state, 'brushWidth', {
      label: 'Width',
      min: 2,
      max: 128,
      step: 1
    }).on('change', (ev) => {
      this.adapter.setBrushWidth(ev.value);
      this.triggerAutosave();
    });

    // Mode names for mosaic (6 modes: 2 web + 4 original C++)
    this.modeList = [
      { name: 'Smooth', value: 'smooth' },
      { name: 'Glow', value: 'glow' },
      { name: 'Basic', value: 'basic' },
      { name: 'Dope', value: 'dope' },
      { name: 'Arrow', value: 'arrow' },
      { name: 'Fat', value: 'arrowFat' }
    ];

    // Mode selection as button grid (3x2)
    this.modeBlade = folder.addBlade({
      view: 'buttongrid',
      size: [3, 2],
      cells: (x, y) => ({
        title: this.modeList[y * 3 + x].name
      }),
      label: 'Mode'
    }).on('click', (ev) => {
      const idx = ev.index[1] * 3 + ev.index[0];
      const mode = this.modeList[idx];
      this.state.brushMode = mode.value;
      const brush = this.adapter.getActiveBrush();
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
      const brush = this.adapter.getActiveBrush();
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
      const brush = this.adapter.getActiveBrush();
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
      this.adapter.setBloomEnabled(ev.value);
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'bloomIntensity', {
      label: 'Bloom Int',
      min: 0,
      max: 2,
      step: 0.1
    }).on('change', (ev) => {
      this.adapter.setBloomIntensity(ev.value);
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'bloomThreshold', {
      label: 'Bloom Thr',
      min: 0,
      max: 1,
      step: 0.05
    }).on('change', (ev) => {
      this.adapter.setBloomThreshold(ev.value);
      this.triggerAutosave();
    });
  }

  /**
   * Update drip parameters on all brushes
   */
  updateDripParams() {
    for (const brush of this.adapter.getBrushes()) {
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
      const cameras = await this.adapter.getAvailableCameras();
      if (cameras.length > 0) {
        const cameraOptions = {};
        cameras.forEach((cam, i) => {
          const label = cam.label || `Camera ${i + 1}`;
          cameraOptions[label] = cam.deviceId;
        });

        let currentDeviceId = cameras[0].deviceId;
        const storedDeviceId = this.adapter.getCurrentCameraDeviceId();
        if (storedDeviceId) {
          currentDeviceId = storedDeviceId;
        }

        this.state.selectedCamera = currentDeviceId;
        folder.addBinding(this.state, 'selectedCamera', {
          label: 'Device',
          options: cameraOptions
        }).on('change', async (ev) => {
          try {
            await this.adapter.switchCameraDevice(ev.value);
            console.log('Switched to camera:', ev.value);
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
        await this.adapter.setCameraResolution(w, h);
      } catch (e) {
        console.error('Failed to change resolution:', e);
      }
    });

    // Flip controls
    folder.addBinding(this.state, 'flipH', {
      label: 'Flip Horizontal'
    }).on('change', (ev) => {
      this.adapter.setCameraFlipH(ev.value);
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'flipV', {
      label: 'Flip Vertical'
    }).on('change', (ev) => {
      this.adapter.setCameraFlipV(ev.value);
      this.triggerAutosave();
    });

    // Rotation control
    folder.addBinding(this.state, 'rotation', {
      label: 'Rotation',
      options: {
        '0°': 0,
        '90°': 90,
        '180°': 180,
        '270°': 270
      }
    }).on('change', (ev) => {
      this.adapter.setCameraRotation(ev.value);
      this.triggerAutosave();
    });
  }

  /**
   * Create tracking settings folder
   */
  createTrackingFolder() {
    const folder = this.pane.addFolder({ title: 'Laser Detection', expanded: false });
    this.folders.tracking = folder;

    // Preset selector
    folder.addBinding(this.state, 'trackerPreset', {
      label: 'Preset',
      options: {
        'Green Laser': 'Green Laser',
        'Red Laser': 'Red Laser',
        'Blue Laser': 'Blue Laser',
        'White/Bright': 'White/Bright',
        'Red Object': 'Red Object',
        'Green Object': 'Green Object',
        'Blue Object': 'Blue Object'
      }
    }).on('change', (ev) => {
      // Laser presets: high brightness (Val Min 200+) for laser pointers
      // Object presets: lower brightness (Val Min 80+) for colored objects
      const presets = {
        'Green Laser': [35, 85, 50, 255, 200, 255],
        'Red Laser': [0, 15, 100, 255, 200, 255],
        'Blue Laser': [100, 130, 100, 255, 200, 255],
        'White/Bright': [0, 180, 0, 50, 240, 255],
        'Red Object': [0, 10, 120, 255, 80, 255],
        'Green Object': [35, 85, 50, 255, 80, 255],
        'Blue Object': [100, 130, 80, 255, 80, 255]
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

    // Advanced tracking options
    folder.addBlade({
      view: 'separator'
    });

    folder.addBinding(this.state, 'useKalman', {
      label: 'Kalman Filter'
    }).on('change', () => { this.updateTrackerParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'useOpticalFlow', {
      label: 'Optical Flow'
    }).on('change', () => { this.updateTrackerParams(); this.triggerAutosave(); });

    folder.addBinding(this.state, 'useCamshift', {
      label: 'CAMShift'
    }).on('change', () => { this.updateTrackerParams(); this.triggerAutosave(); });
  }

  /**
   * Create calibration folder
   */
  createCalibrationFolder() {
    const folder = this.pane.addFolder({ title: 'Calibration', expanded: false });
    this.folders.calibration = folder;

    // Camera calibration (input) - nested folder
    const cameraFolder = folder.addFolder({ title: 'Camera (Input)', expanded: true });

    cameraFolder.addButton({ title: 'Toggle Calibration (Space)' }).on('click', () => {
      this.toggleCalibration();
    });

    cameraFolder.addButton({ title: 'Reset' }).on('click', () => {
      this.adapter.resetCalibration();
    });

    // Projector calibration (output) - nested folder
    const projectorFolder = folder.addFolder({ title: 'Projector (Output)', expanded: true });

    projectorFolder.addButton({ title: 'Toggle Calibration (P)' }).on('click', () => {
      this.adapter.toggleProjectorCalibration();
    });

    // Checkerboard pattern for calibration
    this.state.showCheckerboard = false;
    projectorFolder.addBinding(this.state, 'showCheckerboard', {
      label: 'Checkerboard'
    }).on('change', (ev) => {
      this.adapter.setProjectorCheckerboard(ev.value);
    });

    projectorFolder.addButton({ title: 'Reset' }).on('click', () => {
      this.adapter.resetProjectorCalibration();
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
      this.adapter.setBackgroundColor(ev.value);
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'useMouseInput', {
      label: 'Mouse (M)'
    }).on('change', (ev) => {
      this.adapter.setMouseInputEnabled(ev.value);
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
      this.adapter.clearCanvas();
    });

    folder.addButton({ title: 'Undo' }).on('click', () => {
      this.adapter.undo();
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
      this.adapter.setEraseZone({ enabled: ev.value });
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneX', {
      label: 'X (%)',
      min: 0,
      max: 100,
      step: 1
    }).on('change', (ev) => {
      this.adapter.setEraseZone({ x: ev.value / 100 });
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneY', {
      label: 'Y (%)',
      min: 0,
      max: 100,
      step: 1
    }).on('change', (ev) => {
      this.adapter.setEraseZone({ y: ev.value / 100 });
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneWidth', {
      label: 'Width (%)',
      min: 5,
      max: 50,
      step: 1
    }).on('change', (ev) => {
      this.adapter.setEraseZone({ width: ev.value / 100 });
      this.triggerAutosave();
    });

    folder.addBinding(this.state, 'eraseZoneHeight', {
      label: 'Height (%)',
      min: 5,
      max: 50,
      step: 1
    }).on('change', (ev) => {
      this.adapter.setEraseZone({ height: ev.value / 100 });
      this.triggerAutosave();
    });
  }

  /**
   * Create settings persistence folder (save/load/reset)
   */
  createSettingsFolder() {
    const folder = this.pane.addFolder({ title: 'Settings', expanded: false });
    this.folders.settings = folder;

    // Preset name input
    this.state.presetName = '';
    folder.addBinding(this.state, 'presetName', {
      label: 'Preset Name'
    });

    // Save preset button
    folder.addButton({ title: 'Save Preset' }).on('click', () => {
      const name = this.state.presetName.trim();
      if (!name) {
        alert('Please enter a preset name');
        return;
      }
      if (this.settingsManager.savePreset(name, this.state)) {
        this.currentPresetName = name;
        this.updatePresetsDropdown();
        alert(`Preset "${name}" saved!`);
      }
    });

    // Presets dropdown
    this.state.selectedPreset = '';
    this.presetsBinding = folder.addBinding(this.state, 'selectedPreset', {
      label: 'Load Preset',
      options: this.getPresetsOptions()
    }).on('change', (ev) => {
      if (ev.value) {
        this.loadPreset(ev.value);
      }
    });

    // Delete preset button
    folder.addButton({ title: 'Delete Preset' }).on('click', () => {
      const name = this.state.selectedPreset;
      if (!name) {
        alert('Please select a preset to delete');
        return;
      }
      if (confirm(`Delete preset "${name}"?`)) {
        this.settingsManager.deletePreset(name);
        this.state.selectedPreset = '';
        this.updatePresetsDropdown();
      }
    });

    folder.addBlade({ view: 'separator' });

    // Reset to defaults button
    folder.addButton({ title: 'Reset to Defaults' }).on('click', () => {
      if (confirm('Reset all settings to defaults?')) {
        this.resetToDefaults();
      }
    });

    // Clear autosave button
    folder.addButton({ title: 'Clear Autosave' }).on('click', () => {
      if (confirm('Clear autosaved settings? (Will use defaults on next refresh)')) {
        this.settingsManager.clearAutosave();
        alert('Autosave cleared');
      }
    });
  }

  /**
   * Get presets options for dropdown
   */
  getPresetsOptions() {
    const presets = this.settingsManager.getPresetsList();
    const options = { '-- Select --': '' };
    presets.forEach(name => {
      options[name] = name;
    });
    return options;
  }

  /**
   * Update presets dropdown options
   */
  updatePresetsDropdown() {
    if (this.presetsBinding) {
      // Remove and recreate the binding with new options
      const folder = this.folders.settings;
      const index = folder.children.indexOf(this.presetsBinding);
      this.presetsBinding.dispose();

      this.presetsBinding = folder.addBinding(this.state, 'selectedPreset', {
        label: 'Load Preset',
        options: this.getPresetsOptions(),
        index: index
      }).on('change', (ev) => {
        if (ev.value) {
          this.loadPreset(ev.value);
        }
      });
    }
  }

  /**
   * Load a preset by name
   */
  loadPreset(name) {
    const settings = this.settingsManager.loadPreset(name);
    if (settings) {
      // Merge with defaults to handle any missing keys
      const merged = this.settingsManager.mergeWithDefaults(settings);
      Object.assign(this.state, merged);
      this.currentPresetName = name;
      this.applyStateToApp();
      this.pane.refresh();
      this.updateColorSelection();
      this.updateShadowColorSelection();
      this.updateModeSelection();
      this.updateHsvPreviews();
      this.triggerAutosave();
    }
  }

  /**
   * Reset all settings to defaults
   */
  resetToDefaults() {
    const defaults = this.settingsManager.getDefaults();
    Object.assign(this.state, defaults);
    this.currentPresetName = '';
    this.applyStateToApp();
    this.pane.refresh();
    this.updateColorSelection();
    this.updateShadowColorSelection();
    this.updateModeSelection();
    this.updateHsvPreviews();
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

    // Open as popup with minimal chrome - no URL bar, toolbars, menus
    this.projectorWindow = window.open('', 'laserTagProjector',
      'popup=yes,width=1280,height=720,menubar=no,toolbar=no,location=no,status=no,scrollbars=no,resizable=yes,titlebar=no,directories=no');

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
            isolation: isolate;
          }
          #canvas-container {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            overflow: visible;
            background: #000;
          }
          #canvas-container:fullscreen {
            width: 100vw;
            height: 100vh;
          }
          #projector-canvas {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            display: block;
            transform-origin: 0 0;
            z-index: 1;
          }
          #overlay-canvas {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            display: block;
            pointer-events: none;
            z-index: 1000 !important;
          }
          #fullscreen-hint {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            padding: 8px;
            background: rgba(0,0,0,0.8);
            color: #888;
            font-family: sans-serif;
            font-size: 12px;
            text-align: center;
            z-index: 2000;
            cursor: pointer;
          }
          #fullscreen-hint:hover {
            color: #fff;
            background: rgba(0,0,0,0.9);
          }
          body:fullscreen #fullscreen-hint,
          #canvas-container:fullscreen ~ #fullscreen-hint {
            display: none;
          }
          /* Ensure canvases fill fullscreen container and maintain z-index */
          #canvas-container:fullscreen #projector-canvas {
            width: 100%;
            height: 100%;
            z-index: 1;
          }
          #canvas-container:fullscreen #overlay-canvas {
            width: 100%;
            height: 100%;
            z-index: 1000 !important;
          }
        </style>
      </head>
      <body>
        <div id="canvas-container">
          <canvas id="projector-canvas"></canvas>
          <canvas id="overlay-canvas"></canvas>
        </div>
        <div id="fullscreen-hint">Click here or press F to go fullscreen (hides address bar)</div>
        <script>
          // Fullscreen on hint click, double-click, or F key
          function goFullscreen() {
            if (!document.fullscreenElement) {
              document.documentElement.requestFullscreen().catch(() => {});
            } else {
              document.exitFullscreen().catch(() => {});
            }
          }
          document.getElementById('fullscreen-hint').addEventListener('click', goFullscreen);
          document.addEventListener('dblclick', goFullscreen);
          document.addEventListener('keydown', (e) => {
            if (e.key.toLowerCase() === 'f') goFullscreen();
          });
        </script>
        <script>
          // BroadcastChannel for reconnection after main window refresh
          const channel = new BroadcastChannel('laserTagProjectorChannel');
          let readyInterval = null;

          // Listen for connection confirmation
          channel.onmessage = (event) => {
            if (event.data.type === 'connected' && readyInterval) {
              clearInterval(readyInterval);
              readyInterval = null;
            }
          };

          // Notify main window that popup is ready
          channel.postMessage({ type: 'popup-ready' });

          // Send ready message periodically until connected (max 30 seconds)
          readyInterval = setInterval(() => {
            channel.postMessage({ type: 'popup-ready' });
          }, 2000);
          setTimeout(() => { if (readyInterval) clearInterval(readyInterval); }, 30000);

          // Mark popup as open in localStorage
          localStorage.setItem('laserTag_projectorPopupOpen', 'true');

          // Clean up on close
          window.addEventListener('beforeunload', () => {
            localStorage.removeItem('laserTag_projectorPopupOpen');
            channel.close();
          });
        </script>
      </body>
      </html>
    `);

    // Mark popup as open
    localStorage.setItem(POPUP_OPEN_KEY, 'true');

    const canvas = this.projectorWindow.document.getElementById('projector-canvas');
    const ctx = canvas.getContext('2d');
    const overlayCanvas = this.projectorWindow.document.getElementById('overlay-canvas');
    const overlayCtx = overlayCanvas.getContext('2d');
    const container = this.projectorWindow.document.getElementById('canvas-container');

    // Set up event handlers
    this.setupPopupEventHandlers(this.projectorWindow, canvas, overlayCanvas, container);

    this.adapter.setProjectorPopup({
      window: this.projectorWindow,
      canvas: canvas,
      ctx: ctx,
      overlayCanvas: overlayCanvas,
      overlayCtx: overlayCtx,
      container: container
    });

    // Listen for popup close to clean up state
    const checkPopupClosed = setInterval(() => {
      if (!this.projectorWindow || this.projectorWindow.closed) {
        clearInterval(checkPopupClosed);
        this.projectorWindow = null;
        this.adapter.setProjectorPopup(null);
        localStorage.removeItem(POPUP_OPEN_KEY);
      }
    }, 500);

    console.log('Projector window opened.');
  }

  /**
   * Set up event handlers for popup window
   * @param {Window} popupWindow - The popup window
   * @param {HTMLCanvasElement} canvas - The content canvas
   * @param {HTMLCanvasElement} overlayCanvas - The overlay canvas (for calibration UI)
   * @param {HTMLElement} container - The canvas container (for fullscreen)
   */
  setupPopupEventHandlers(popupWindow, canvas, overlayCanvas, container) {
    const resize = () => {
      // Use screen dimensions if in fullscreen, otherwise window dimensions
      const isFullscreen = popupWindow.document.fullscreenElement !== null;
      let w, h;
      if (isFullscreen) {
        // In fullscreen, use screen dimensions
        w = popupWindow.screen.width;
        h = popupWindow.screen.height;
      } else {
        w = popupWindow.innerWidth;
        h = popupWindow.innerHeight;
      }
      canvas.width = w;
      canvas.height = h;
      overlayCanvas.width = w;
      overlayCanvas.height = h;
      console.log(`Popup resize: ${w}x${h}, fullscreen: ${isFullscreen}`);
    };
    resize();
    popupWindow.addEventListener('resize', resize);

    // Also resize on fullscreen change
    popupWindow.document.addEventListener('fullscreenchange', () => {
      // Delay slightly to let fullscreen dimensions settle
      setTimeout(resize, 100);
      // Also resize again after a longer delay for slower systems
      setTimeout(resize, 500);
    });

    // F key fullscreen disabled - use native OS fullscreen instead

    // Mouse handlers for projector calibration - on overlay canvas for proper interaction
    let popupDragging = false;

    overlayCanvas.style.pointerEvents = 'auto';
    overlayCanvas.addEventListener('mousedown', (e) => {
      if (!this.adapter.isProjectorCalibrating()) return;
      const idx = this.adapter.selectProjectorPoint(e.clientX, e.clientY, canvas);
      if (idx >= 0) {
        popupDragging = true;
      }
    });

    overlayCanvas.addEventListener('mousemove', (e) => {
      if (!this.adapter.isProjectorCalibrating() || !popupDragging) return;
      const selectedPoint = this.adapter.getProjectorSelectedPoint();
      if (selectedPoint >= 0) {
        this.adapter.moveProjectorPoint(selectedPoint, e.clientX, e.clientY, overlayCanvas);
      }
    });

    overlayCanvas.addEventListener('mouseup', () => {
      popupDragging = false;
      this.adapter.setProjectorSelectedPoint(-1);
    });

    overlayCanvas.addEventListener('mouseleave', () => {
      popupDragging = false;
    });
  }

  /**
   * Sync GUI state from app
   */
  syncFromApp() {
    const trackerParams = this.adapter.getTrackerParams();

    this.state.hueMin = trackerParams.hueMin;
    this.state.hueMax = trackerParams.hueMax;
    this.state.satMin = trackerParams.satMin;
    this.state.satMax = trackerParams.satMax;
    this.state.valMin = trackerParams.valMin;
    this.state.valMax = trackerParams.valMax;
    this.state.smoothing = trackerParams.smoothing;
    this.state.showDebug = trackerParams.showDebug;

    this.adapter.setBrushColor(this.state.brushColor);
    this.adapter.setBrushWidth(this.state.brushWidth);

    const brush = this.adapter.getActiveBrush();
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
    this.adapter.setTrackerParams({
      hueMin: this.state.hueMin,
      hueMax: this.state.hueMax,
      satMin: this.state.satMin,
      satMax: this.state.satMax,
      valMin: this.state.valMin,
      valMax: this.state.valMax,
      smoothing: this.state.smoothing,
      useKalman: this.state.useKalman,
      useOpticalFlow: this.state.useOpticalFlow,
      useCamshift: this.state.useCamshift
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
    this.updateHsvPreviews();
    this.pane.refresh();
  }

  /**
   * Toggle calibration mode
   */
  toggleCalibration() {
    const isCalibrating = this.adapter.toggleCalibration();
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
    const projector = this.adapter.getProjectorCanvas();

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
