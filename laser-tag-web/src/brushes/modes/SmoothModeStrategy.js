/**
 * SmoothModeStrategy - Smooth line rendering with variable width
 *
 * Default mode that draws smooth, anti-aliased strokes with
 * velocity-based width variation.
 */
import { BrushModeStrategy } from './BrushModeStrategy.js';

export class SmoothModeStrategy extends BrushModeStrategy {
  constructor() {
    super('smooth');
  }

  /**
   * Render a smooth segment with variable width
   * Uses filled quads with circular end caps for smooth appearance
   */
  renderSegment(ctx, p0, p1, color, params) {
    const w0 = p0.width || params.brushWidth;
    const w1 = p1.width || params.brushWidth;

    const dir = BrushModeStrategy.direction(p0, p1);
    if (!dir) return;

    const { nx, ny } = dir;

    // Draw filled quad with gradient width
    ctx.beginPath();
    ctx.moveTo(p0.x + nx * w0 / 2, p0.y + ny * w0 / 2);
    ctx.lineTo(p1.x + nx * w1 / 2, p1.y + ny * w1 / 2);
    ctx.lineTo(p1.x - nx * w1 / 2, p1.y - ny * w1 / 2);
    ctx.lineTo(p0.x - nx * w0 / 2, p0.y - ny * w0 / 2);
    ctx.closePath();

    ctx.fillStyle = `rgba(${color.r}, ${color.g}, ${color.b}, ${params.opacity})`;
    ctx.fill();

    // Draw end cap
    ctx.beginPath();
    ctx.arc(p1.x, p1.y, w1 / 2, 0, Math.PI * 2);
    ctx.fill();
  }
}
