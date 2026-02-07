/**
 * SettingsManager - Handles localStorage persistence for GUI settings
 * Features:
 * - Autosave session that persists across refreshes
 * - Save/Load presets with custom names
 * - Reset to defaults
 */
import { storageSave, storageLoad, storageRemove } from '../utils/StorageUtils.js';

const STORAGE_PREFIX = 'laserTag_';
const AUTOSAVE_KEY = `${STORAGE_PREFIX}autosave`;
const PRESETS_INDEX_KEY = `${STORAGE_PREFIX}presets_index`;

export class SettingsManager {
  constructor() {
    // Default settings - the source of truth for reset
    this.defaults = {
      // Brush settings
      brushColor: '#0AC2FF',
      brushColorIndex: 4,
      brushWidth: 4,
      brushMode: 'smooth',
      brushIndex: 0,
      glowIntensity: 0.5,
      shadowOffset: 8,
      shadowColor: '#FF0AC2',
      shadowColorIndex: 3,

      // Drip settings
      dripsEnabled: true,
      dripsFrequency: 30,
      dripsSpeed: 0.3,
      dripsDirection: 0,
      dripsWidth: 1,

      // Bloom/Post-processing settings
      bloomEnabled: false,
      bloomIntensity: 0.5,
      bloomThreshold: 0.3,

      // Tracker settings
      hueMin: 35,
      hueMax: 85,
      satMin: 50,
      satMax: 255,
      valMin: 200,
      valMax: 255,
      smoothing: 0.3,
      trackerPreset: 'Green Laser',

      // Advanced tracking options
      useKalman: true,
      useOpticalFlow: true,
      useCamshift: false,

      // Display settings
      showDebug: true,
      backgroundColor: '#000000',
      brightness: 100,
      useMouseInput: false,

      // Camera settings
      flipH: false,
      flipV: false,
      rotation: 0,

      // Erase zone settings
      eraseZoneEnabled: false,
      eraseZoneX: 0,
      eraseZoneY: 0,
      eraseZoneWidth: 15,
      eraseZoneHeight: 15
    };

    // Debounce timer for autosave
    this.autosaveTimer = null;
    this.autosaveDelay = 500; // ms
  }

  /**
   * Get default settings (deep copy)
   * @returns {Object}
   */
  getDefaults() {
    return JSON.parse(JSON.stringify(this.defaults));
  }

  /**
   * Load autosaved session
   * @returns {Object|null} Settings or null if none found
   */
  loadAutosave() {
    const settings = storageLoad(AUTOSAVE_KEY);
    if (settings) console.log('Loaded autosaved settings');
    return settings;
  }

  /**
   * Save settings to autosave slot (debounced)
   * @param {Object} settings - Current settings state
   */
  autosave(settings) {
    // Debounce to avoid too many writes
    if (this.autosaveTimer) {
      clearTimeout(this.autosaveTimer);
    }

    this.autosaveTimer = setTimeout(() => {
      this._doAutosave(settings);
    }, this.autosaveDelay);
  }

  /**
   * Perform the actual autosave
   * @param {Object} settings
   */
  _doAutosave(settings) {
    storageSave(AUTOSAVE_KEY, settings);
  }

  /**
   * Clear autosaved session
   */
  clearAutosave() {
    storageRemove(AUTOSAVE_KEY);
    console.log('Autosave cleared');
  }

  /**
   * Save settings with a custom name
   * @param {string} name - Preset name
   * @param {Object} settings - Settings to save
   * @returns {boolean} Success
   */
  savePreset(name, settings) {
    if (!name || !name.trim()) {
      console.error('Preset name cannot be empty');
      return false;
    }

    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;

    // Include calibration data in preset
    const cameraQuad = storageLoad('laserTagCalibration');
    const projectorQuad = storageLoad('laserTag_projectorQuad');

    const saved = storageSave(key, {
      name: name.trim(),
      savedAt: new Date().toISOString(),
      settings: settings,
      calibration: {
        camera: cameraQuad,
        projector: projectorQuad
      }
    }, `Preset "${name}"`);

    if (saved) {
      // Update presets index
      const presets = this.getPresetsList();
      if (!presets.includes(name.trim())) {
        presets.push(name.trim());
        storageSave(PRESETS_INDEX_KEY, presets);
      }
    }

    return saved;
  }

  /**
   * Load a preset by name
   * @param {string} name - Preset name
   * @returns {Object|null} Settings or null if not found
   */
  loadPreset(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;
    const data = storageLoad(key, null, `Preset "${name}"`);
    if (!data) return null;

    console.log(`Preset "${name}" loaded`);

    // Restore calibration data if present
    if (data.calibration) {
      if (data.calibration.camera) {
        storageSave('laserTagCalibration', data.calibration.camera);
      }
      if (data.calibration.projector) {
        storageSave('laserTag_projectorQuad', data.calibration.projector);
      }
    }

    return data.settings;
  }

  /**
   * Delete a preset by name
   * @param {string} name - Preset name
   * @returns {boolean} Success
   */
  deletePreset(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;
    storageRemove(key);

    // Update presets index
    const presets = this.getPresetsList();
    const idx = presets.indexOf(name.trim());
    if (idx >= 0) {
      presets.splice(idx, 1);
      storageSave(PRESETS_INDEX_KEY, presets);
    }

    console.log(`Preset "${name}" deleted`);
    return true;
  }

  /**
   * Get list of saved preset names
   * @returns {Array<string>}
   */
  getPresetsList() {
    return storageLoad(PRESETS_INDEX_KEY, [], 'presets list');
  }

  /**
   * Get preset metadata (name, savedAt) without loading full settings
   * @param {string} name - Preset name
   * @returns {Object|null}
   */
  getPresetInfo(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;
    const data = storageLoad(key);
    if (!data) return null;
    return { name: data.name, savedAt: data.savedAt };
  }

  /**
   * Merge loaded settings with defaults (for handling new settings added in updates)
   * @param {Object} loaded - Loaded settings
   * @returns {Object} Merged settings
   */
  mergeWithDefaults(loaded) {
    return {
      ...this.getDefaults(),
      ...loaded
    };
  }

  /**
   * Export settings as JSON string (for file download)
   * @param {Object} settings
   * @returns {string}
   */
  exportToJson(settings) {
    return JSON.stringify({
      appName: 'L.A.S.E.R. TAG',
      version: '1.0',
      exportedAt: new Date().toISOString(),
      settings: settings
    }, null, 2);
  }

  /**
   * Import settings from JSON string
   * @param {string} jsonStr
   * @returns {Object|null} Settings or null on error
   */
  importFromJson(jsonStr) {
    try {
      const data = JSON.parse(jsonStr);
      if (data.settings) {
        return this.mergeWithDefaults(data.settings);
      }
    } catch (e) {
      console.error('Failed to import settings:', e);
    }
    return null;
  }
}
