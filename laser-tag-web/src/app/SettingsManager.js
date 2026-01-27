/**
 * SettingsManager - Handles localStorage persistence for GUI settings
 * Features:
 * - Autosave session that persists across refreshes
 * - Save/Load presets with custom names
 * - Reset to defaults
 */

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
      smoothing: 0.5,
      trackerPreset: 'Green Laser',

      // Display settings
      showDebug: true,
      backgroundColor: '#000000',
      brightness: 100,
      useMouseInput: false,

      // Camera settings
      flipH: false,
      flipV: false,
      resolution: '640x480',

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
    try {
      const saved = localStorage.getItem(AUTOSAVE_KEY);
      if (saved) {
        const parsed = JSON.parse(saved);
        console.log('Loaded autosaved settings');
        return parsed;
      }
    } catch (e) {
      console.error('Failed to load autosave:', e);
    }
    return null;
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
    try {
      localStorage.setItem(AUTOSAVE_KEY, JSON.stringify(settings));
      // console.log('Settings autosaved');
    } catch (e) {
      console.error('Failed to autosave settings:', e);
    }
  }

  /**
   * Clear autosaved session
   */
  clearAutosave() {
    try {
      localStorage.removeItem(AUTOSAVE_KEY);
      console.log('Autosave cleared');
    } catch (e) {
      console.error('Failed to clear autosave:', e);
    }
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

    try {
      // Save the preset
      localStorage.setItem(key, JSON.stringify({
        name: name.trim(),
        savedAt: new Date().toISOString(),
        settings: settings
      }));

      // Update presets index
      const presets = this.getPresetsList();
      if (!presets.includes(name.trim())) {
        presets.push(name.trim());
        localStorage.setItem(PRESETS_INDEX_KEY, JSON.stringify(presets));
      }

      console.log(`Preset "${name}" saved`);
      return true;
    } catch (e) {
      console.error('Failed to save preset:', e);
      return false;
    }
  }

  /**
   * Load a preset by name
   * @param {string} name - Preset name
   * @returns {Object|null} Settings or null if not found
   */
  loadPreset(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;

    try {
      const saved = localStorage.getItem(key);
      if (saved) {
        const data = JSON.parse(saved);
        console.log(`Preset "${name}" loaded`);
        return data.settings;
      }
    } catch (e) {
      console.error(`Failed to load preset "${name}":`, e);
    }
    return null;
  }

  /**
   * Delete a preset by name
   * @param {string} name - Preset name
   * @returns {boolean} Success
   */
  deletePreset(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;

    try {
      localStorage.removeItem(key);

      // Update presets index
      const presets = this.getPresetsList();
      const idx = presets.indexOf(name.trim());
      if (idx >= 0) {
        presets.splice(idx, 1);
        localStorage.setItem(PRESETS_INDEX_KEY, JSON.stringify(presets));
      }

      console.log(`Preset "${name}" deleted`);
      return true;
    } catch (e) {
      console.error(`Failed to delete preset "${name}":`, e);
      return false;
    }
  }

  /**
   * Get list of saved preset names
   * @returns {Array<string>}
   */
  getPresetsList() {
    try {
      const saved = localStorage.getItem(PRESETS_INDEX_KEY);
      if (saved) {
        return JSON.parse(saved);
      }
    } catch (e) {
      console.error('Failed to get presets list:', e);
    }
    return [];
  }

  /**
   * Get preset metadata (name, savedAt) without loading full settings
   * @param {string} name - Preset name
   * @returns {Object|null}
   */
  getPresetInfo(name) {
    const key = `${STORAGE_PREFIX}preset_${name.trim()}`;

    try {
      const saved = localStorage.getItem(key);
      if (saved) {
        const data = JSON.parse(saved);
        return {
          name: data.name,
          savedAt: data.savedAt
        };
      }
    } catch (e) {
      console.error(`Failed to get preset info for "${name}":`, e);
    }
    return null;
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
