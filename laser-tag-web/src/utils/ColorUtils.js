/**
 * ColorUtils - Shared hex color parsing utilities
 */

const HEX_REGEX = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i;

/**
 * Parse hex color string to {r, g, b} object
 * @param {string} hex - Hex color (e.g. '#FF0AC2' or 'FF0AC2')
 * @returns {{r: number, g: number, b: number}|null}
 */
export function hexToRgb(hex) {
  const result = HEX_REGEX.exec(hex);
  if (!result) return null;
  return {
    r: parseInt(result[1], 16),
    g: parseInt(result[2], 16),
    b: parseInt(result[3], 16)
  };
}

/**
 * Parse hex color string to rgba() CSS string
 * @param {string} hex - Hex color
 * @param {number} alpha - Alpha (0-1)
 * @returns {string}
 */
export function hexToRgba(hex, alpha = 1) {
  const rgb = hexToRgb(hex);
  if (!rgb) return `rgba(0, 0, 0, ${alpha})`;
  return `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, ${alpha})`;
}
