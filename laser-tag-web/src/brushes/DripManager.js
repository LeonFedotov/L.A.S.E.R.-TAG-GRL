/**
 * DripManager - Handles drip particle physics and rendering
 * Extracted from VectorBrush to separate drip concerns
 *
 * Based on original L.A.S.E.R. TAG drips.cpp
 * Drips travel a set distance with deceleration, drawing continuous trails
 */
export class DripManager {
  constructor() {
    // Active drips
    this.drips = [];
    // Trail segments for redraw
    this.trails = [];
    // Index of first undrawn trail (for incremental rendering)
    this._drawnTrailCount = 0;
  }

  /**
   * Spawn a new drip particle
   * @param {Object} options - Drip options
   * @param {number} options.x - Starting X position
   * @param {number} options.y - Starting Y position
   * @param {number} options.direction - 0=south, 1=west, 2=north, 3=east
   * @param {number} options.speed - Movement speed
   * @param {number} options.width - Line width
   * @param {Object} options.color - {r, g, b} color
   * @param {number} options.opacity - Opacity (0-1)
   * @param {number} options.canvasWidth - Canvas width for bounds
   * @param {number} options.canvasHeight - Canvas height for bounds
   * @param {number} [options.strokeIndex] - Parent stroke index for layering
   */
  spawn(options) {
    const {
      x, y, direction, speed, width, color, opacity,
      canvasWidth, canvasHeight, strokeIndex
    } = options;

    // Calculate max length based on direction and available space
    let maxLength = 0;
    if (direction === 0) {        // south
      maxLength = canvasHeight - y;
    } else if (direction === 1) { // west
      maxLength = x;
    } else if (direction === 2) { // north
      maxLength = y;
    } else if (direction === 3) { // east
      maxLength = canvasWidth - x;
    }

    if (maxLength < 10) return;

    // Random length up to 1/3 of available space (like original)
    let length = Math.random() * (maxLength / 3);
    length *= 0.75;
    length = Math.max(20, Math.min(150, length));

    // Calculate velocity based on direction
    let vx = 0, vy = 0;
    if (direction === 0) vy = speed;       // south
    else if (direction === 1) vx = -speed; // west
    else if (direction === 2) vy = -speed; // north
    else if (direction === 3) vx = speed;  // east

    this.drips.push({
      x,
      y,
      prevX: x,
      prevY: y,
      vx,
      vy,
      distance: 0,
      maxDistance: length,
      width,
      color: { ...color },
      opacity,
      active: true,
      strokeIndex,
      canvasWidth,
      canvasHeight
    });
  }

  /**
   * Update drip physics - called each frame
   * Generates trail segments as drips move
   */
  update() {
    for (let i = this.drips.length - 1; i >= 0; i--) {
      const drip = this.drips[i];

      if (!drip.active) {
        this.drips.splice(i, 1);
        continue;
      }

      // Store previous position
      drip.prevX = drip.x;
      drip.prevY = drip.y;

      // Update distance traveled
      drip.distance += Math.abs(drip.vx) + Math.abs(drip.vy);

      // Check if reached target
      if (drip.distance >= drip.maxDistance) {
        drip.active = false;
        continue;
      }

      // Deceleration when approaching end (last 24 pixels)
      if (drip.maxDistance - drip.distance < 24) {
        drip.vx *= 0.987;
        drip.vy *= 0.987;

        // Stop if velocity negligible
        if (Math.abs(drip.vx) < 0.01 && Math.abs(drip.vy) < 0.01) {
          drip.active = false;
          continue;
        }
      }

      // Update position
      drip.x += drip.vx;
      drip.y += drip.vy;

      // Clamp to canvas bounds
      if (drip.x < 0 || drip.x >= drip.canvasWidth ||
          drip.y < 0 || drip.y >= drip.canvasHeight) {
        drip.active = false;
        continue;
      }

      // Store trail segment for redraw
      this.trails.push({
        x0: drip.prevX,
        y0: drip.prevY,
        x1: drip.x,
        y1: drip.y,
        width: drip.width,
        color: { ...drip.color },
        opacity: drip.opacity,
        strokeIndex: drip.strokeIndex
      });
    }
  }

  /**
   * Draw all trail segments
   * @param {CanvasRenderingContext2D} ctx
   */
  drawAllTrails(ctx) {
    for (const trail of this.trails) {
      this.drawTrailSegment(ctx, trail);
    }
  }

  /**
   * Draw only trail segments added since last call (incremental rendering).
   * Avoids full canvas clear+redraw when only drips are animating.
   * @param {CanvasRenderingContext2D} ctx
   */
  drawNewTrails(ctx) {
    for (let i = this._drawnTrailCount; i < this.trails.length; i++) {
      this.drawTrailSegment(ctx, this.trails[i]);
    }
    this._drawnTrailCount = this.trails.length;
  }

  /**
   * Reset the drawn trail counter (call after full redraw)
   */
  resetDrawnCount() {
    this._drawnTrailCount = this.trails.length;
  }

  /**
   * Draw trails for a specific stroke index
   * @param {CanvasRenderingContext2D} ctx
   * @param {number|null} strokeIndex - Stroke index, or null for orphans
   * @param {Array} activeStrokes - Array of strokes to check for orphans
   */
  drawTrailsForStroke(ctx, strokeIndex, activeStrokes = []) {
    for (const trail of this.trails) {
      const matches = strokeIndex === null
        ? (trail.strokeIndex === undefined ||
           !activeStrokes.some(s => s.strokeIndex === trail.strokeIndex))
        : trail.strokeIndex === strokeIndex;

      if (matches) {
        this.drawTrailSegment(ctx, trail);
      }
    }
  }

  /**
   * Draw a single trail segment
   * @param {CanvasRenderingContext2D} ctx
   * @param {Object} trail
   */
  drawTrailSegment(ctx, trail) {
    ctx.beginPath();
    ctx.moveTo(trail.x0, trail.y0);
    ctx.lineTo(trail.x1, trail.y1);
    ctx.strokeStyle = `rgba(${trail.color.r}, ${trail.color.g}, ${trail.color.b}, ${trail.opacity})`;
    ctx.lineWidth = trail.width;
    ctx.lineCap = 'round';
    ctx.stroke();
  }

  /**
   * Finalize all active drips (add current position as final trail)
   * Called before baking to background
   */
  finalize() {
    for (const drip of this.drips) {
      if (drip.active) {
        this.trails.push({
          x0: drip.prevX,
          y0: drip.prevY,
          x1: drip.x,
          y1: drip.y,
          width: drip.width,
          color: { ...drip.color },
          opacity: drip.opacity,
          strokeIndex: drip.strokeIndex
        });
      }
    }
  }

  /**
   * Check if there are active drips
   * @returns {boolean}
   */
  hasActiveDrips() {
    return this.drips.length > 0;
  }

  /**
   * Check if there are any trails
   * @returns {boolean}
   */
  hasTrails() {
    return this.trails.length > 0;
  }

  /**
   * Clear all drips and trails
   */
  clear() {
    this.drips = [];
    this.trails = [];
    this._drawnTrailCount = 0;
  }

  /**
   * Clear only trails (keep active drips)
   */
  clearTrails() {
    this.trails = [];
    this._drawnTrailCount = 0;
  }
}
