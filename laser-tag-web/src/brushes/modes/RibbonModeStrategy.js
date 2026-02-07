/**
 * RibbonModeStrategy - Base class for ribbon-based modes (dope, arrow, arrowFat)
 *
 * These modes use perpendicular ribbon quads following stroke direction
 * Port of the C++ pattern: perpendicular = (nrm_y, -nrm_x)
 */
import { BrushModeStrategy } from './BrushModeStrategy.js';

export class RibbonModeStrategy extends BrushModeStrategy {
  constructor(name) {
    super(name);
    // Pooled arrays for calculateRibbonPoints to avoid per-frame allocation
    this._topPool = [];
    this._bottomPool = [];
  }

  /**
   * Ribbon modes require full redraw for proper shadow layering
   */
  requiresFullRedraw() {
    return true;
  }

  /**
   * Get shadow color for this mode
   * Override in subclasses for different shadow colors
   * @param {Object} stroke - Stroke data
   * @param {Object} params - Brush parameters
   * @returns {string} Hex color
   */
  getShadowColor(stroke, params) {
    return '#000000';  // Default black shadow
  }

  /**
   * Calculate perpendicular points along stroke using C++ pattern: (nrmY, -nrmX)
   * @param {Array} points - Stroke points
   * @param {number} offset - Shadow offset (0 for main stroke)
   * @param {Object} params - Brush parameters
   * @returns {{topPoints: Array, bottomPoints: Array}}
   */
  calculateRibbonPoints(points, offset, params) {
    const top = this._topPool;
    const bottom = this._bottomPool;
    top.length = points.length;
    bottom.length = points.length;

    for (let i = 0; i < points.length; i++) {
      const p = points[i];
      const w = p.width || params.brushWidth;

      // Calculate normal from surrounding points
      let nrmX = 0, nrmY = 1;
      if (i < points.length - 1) {
        const dir = BrushModeStrategy.direction(p, points[i + 1]);
        if (dir) { nrmX = dir.nrmX; nrmY = dir.nrmY; }
      } else if (i > 0) {
        const dir = BrushModeStrategy.direction(points[i - 1], p);
        if (dir) { nrmX = dir.nrmX; nrmY = dir.nrmY; }
      }

      // Perpendicular using C++ pattern: (nrm_y, -nrm_x)
      // Reuse or create point objects
      if (!top[i]) { top[i] = { x: 0, y: 0 }; bottom[i] = { x: 0, y: 0 }; }
      top[i].x = p.x - offset + nrmY * w / 2;
      top[i].y = p.y + offset - nrmX * w / 2;
      bottom[i].x = p.x - offset - nrmY * w / 2;
      bottom[i].y = p.y + offset + nrmX * w / 2;
    }

    return { topPoints: top, bottomPoints: bottom };
  }

  /**
   * Draw stroke shadow as ribbon
   */
  drawStrokeShadow(ctx, stroke, params) {
    const { topPoints, bottomPoints } = this.calculateRibbonPoints(
      stroke.points,
      params.shadowOffset,
      params
    );

    if (topPoints.length === 0) return;

    ctx.beginPath();
    ctx.moveTo(topPoints[0].x, topPoints[0].y);
    for (let i = 1; i < topPoints.length; i++) {
      ctx.lineTo(topPoints[i].x, topPoints[i].y);
    }
    for (let i = bottomPoints.length - 1; i >= 0; i--) {
      ctx.lineTo(bottomPoints[i].x, bottomPoints[i].y);
    }
    ctx.closePath();

    const shadowColor = this.getShadowColor(stroke, params);
    ctx.fillStyle = this.hexToRgba(shadowColor, 0.7);
    ctx.fill();
  }

  /**
   * Draw stroke main color as ribbon
   */
  drawStroke(ctx, stroke, params) {
    const { topPoints, bottomPoints } = this.calculateRibbonPoints(
      stroke.points,
      0,  // No offset for main stroke
      params
    );

    if (topPoints.length === 0) return;

    ctx.beginPath();
    ctx.moveTo(topPoints[0].x, topPoints[0].y);
    for (let i = 1; i < topPoints.length; i++) {
      ctx.lineTo(topPoints[i].x, topPoints[i].y);
    }
    for (let i = bottomPoints.length - 1; i >= 0; i--) {
      ctx.lineTo(bottomPoints[i].x, bottomPoints[i].y);
    }
    ctx.closePath();

    const color = stroke.color;
    ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${params.opacity})`;
    ctx.fill();
  }

  /**
   * Render single segment (for live drawing, used by dope only)
   */
  renderSegment(ctx, p0, p1, color, params) {
    const w0 = p0.width || params.brushWidth;
    const w1 = p1.width || params.brushWidth;
    const offset = params.shadowOffset;

    const dir = BrushModeStrategy.direction(p0, p1);
    if (!dir) return;

    const nrmX = dir.nrmX;
    const nrmY = dir.nrmY;

    // Draw shadow quad
    ctx.beginPath();
    ctx.moveTo(p0.x - offset + nrmY * w0 / 2, p0.y + offset - nrmX * w0 / 2);
    ctx.lineTo(p1.x - offset + nrmY * w1 / 2, p1.y + offset - nrmX * w1 / 2);
    ctx.lineTo(p1.x - offset - nrmY * w1 / 2, p1.y + offset + nrmX * w1 / 2);
    ctx.lineTo(p0.x - offset - nrmY * w0 / 2, p0.y + offset + nrmX * w0 / 2);
    ctx.closePath();
    ctx.fillStyle = this.hexToRgba(this.getShadowColor(null, params), 0.7);
    ctx.fill();

    // Draw main quad
    ctx.beginPath();
    ctx.moveTo(p0.x + nrmY * w0 / 2, p0.y - nrmX * w0 / 2);
    ctx.lineTo(p1.x + nrmY * w1 / 2, p1.y - nrmX * w1 / 2);
    ctx.lineTo(p1.x - nrmY * w1 / 2, p1.y + nrmX * w1 / 2);
    ctx.lineTo(p0.x - nrmY * w0 / 2, p0.y + nrmX * w0 / 2);
    ctx.closePath();
    ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${params.opacity})`;
    ctx.fill();
  }
}


/**
 * DopeModeStrategy - Dope mode with perpendicular ribbon and black shadow
 */
export class DopeModeStrategy extends RibbonModeStrategy {
  constructor() {
    super('dope');
  }
}


/**
 * ArrowModeStrategy - Arrow mode (dope + arrow head)
 */
export class ArrowModeStrategy extends RibbonModeStrategy {
  constructor() {
    super('arrow');
  }

  /**
   * Draw arrow head shadow
   */
  drawArrowHeadShadow(ctx, stroke, params) {
    const points = stroke.points;
    if (points.length < 2) return;

    const lastPt = points[points.length - 1];
    const prevPt = points[points.length - 2];
    const w = lastPt.width || params.brushWidth;
    const offset = params.shadowOffset;

    const dir = BrushModeStrategy.direction(prevPt, lastPt);
    if (!dir) return;

    const nrmX = dir.nrmX;
    const nrmY = dir.nrmY;

    // Shadow offset applied
    const sx = lastPt.x - offset;
    const sy = lastPt.y + offset;

    const tipX = sx + nrmX * w * 2;
    const tipY = sy + nrmY * w * 2;
    const baseLeftX = sx + nrmY * w * 1.5;
    const baseLeftY = sy - nrmX * w * 1.5;
    const baseRightX = sx - nrmY * w * 1.5;
    const baseRightY = sy + nrmX * w * 1.5;

    ctx.beginPath();
    ctx.moveTo(baseLeftX, baseLeftY);
    ctx.lineTo(tipX, tipY);
    ctx.lineTo(baseRightX, baseRightY);
    ctx.closePath();
    ctx.fillStyle = this.hexToRgba(this.getShadowColor(stroke, params), 0.7);
    ctx.fill();
  }

  /**
   * Draw arrow head
   */
  drawArrowHead(ctx, stroke, params) {
    const points = stroke.points;
    if (points.length < 2) return;

    const lastPt = points[points.length - 1];
    const prevPt = points[points.length - 2];
    const w = lastPt.width || params.brushWidth;
    const color = stroke.color;

    const dir = BrushModeStrategy.direction(prevPt, lastPt);
    if (!dir) return;

    const nrmX = dir.nrmX;
    const nrmY = dir.nrmY;

    const tipX = lastPt.x + nrmX * w * 2;
    const tipY = lastPt.y + nrmY * w * 2;
    const baseLeftX = lastPt.x + nrmY * w * 1.5;
    const baseLeftY = lastPt.y - nrmX * w * 1.5;
    const baseRightX = lastPt.x - nrmY * w * 1.5;
    const baseRightY = lastPt.y + nrmX * w * 1.5;

    ctx.beginPath();
    ctx.moveTo(baseLeftX, baseLeftY);
    ctx.lineTo(tipX, tipY);
    ctx.lineTo(baseRightX, baseRightY);
    ctx.closePath();
    ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${params.opacity})`;
    ctx.fill();
  }

  /**
   * Draw stroke decorations (arrow head)
   */
  drawStrokeDecorations(ctx, stroke, params) {
    this.drawArrowHead(ctx, stroke, params);
  }

  /**
   * Draw shadow decorations (arrow head shadow)
   */
  drawShadowDecorations(ctx, stroke, params) {
    this.drawArrowHeadShadow(ctx, stroke, params);
  }
}


/**
 * ArrowFatModeStrategy - Arrow fat mode with magenta shadow
 */
export class ArrowFatModeStrategy extends ArrowModeStrategy {
  constructor() {
    super();
    this.name = 'arrowFat';
  }

  /**
   * ArrowFat uses configurable shadow color (default magenta 0xFF0AC2)
   */
  getShadowColor(stroke, params) {
    return (stroke && stroke.shadowColor) || params.shadowColor || '#FF0AC2';
  }
}
