/**
 * StorageUtils - Shared localStorage persistence helpers
 */

/**
 * Save JSON-serializable data to localStorage
 * @param {string} key - Storage key
 * @param {*} data - Data to serialize
 * @param {string} [debugName] - Name for log messages
 * @returns {boolean} True if saved
 */
export function storageSave(key, data, debugName) {
  try {
    localStorage.setItem(key, JSON.stringify(data));
    if (debugName) console.log(`${debugName} saved`);
    return true;
  } catch (e) {
    console.error(`Failed to save ${debugName || key}:`, e);
    return false;
  }
}

/**
 * Load and parse JSON from localStorage
 * @param {string} key - Storage key
 * @param {*} [defaultValue=null] - Value to return if key not found
 * @param {string} [debugName] - Name for log messages
 * @returns {*} Parsed data or defaultValue
 */
export function storageLoad(key, defaultValue = null, debugName) {
  try {
    const saved = localStorage.getItem(key);
    if (saved !== null) {
      return JSON.parse(saved);
    }
  } catch (e) {
    console.error(`Failed to load ${debugName || key}:`, e);
  }
  return defaultValue;
}

/**
 * Remove a key from localStorage
 * @param {string} key
 */
export function storageRemove(key) {
  localStorage.removeItem(key);
}
