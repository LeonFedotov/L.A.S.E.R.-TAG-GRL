/**
 * VectorBrush - Vector-based brush with smooth line rendering
 * Based on vectorBrush.cpp from the original L.A.S.E.R. TAG
 *
 * Refactored to use:
 * - BrushModeStrategy pattern for rendering modes
 * - DripManager for drip physics
 */
import { BaseBrush } from './BaseBrush.js';
import { DripManager } from './DripManager.js';
import { getStrategy } from './modes/index.js';

export class VectorBrush extends BaseBrush {
  constructor() {
    super('Vector');

    // Vector brush specific parameters
    this.params = {
      ...this.params,
      brushWidth: 15,
      minWidth: 2,
      maxWidth: 40,
      velocityScale: 0.5,    // How much velocity affects width
      smoothing: 0.85,       // Width smoothing (0=instant, 1=frozen). High value tames camera detection jitter
      mode: 'smooth',        // 'smooth', 'glow', 'basic', 'dope', 'arrow', 'arrowFat'
      shadowOffset: 8,       // Shadow offset for C++ style modes
      shadowColor: '#FF0AC2', // Shadow color for arrowFat mode
      glowIntensity: 0.5,
      // Drip parameters
      dripsEnabled: true,
      dripsFrequency: 30,    // 1-120, higher = more drips
      dripsSpeed: 0.3,       // 0.0-12.0, movement speed
      dripsDirection: 0,     // 0=south, 1=west, 2=north, 3=east
      dripsWidth: 1          // 1-25, line thickness
    };

    // Drip manager
    this.dripManager = new DripManager();

    // Counter for stroke ordering
    this.strokeCounter = 0;

    // Background canvas for finalized/baked content
    this.backgroundCanvas = null;
    this.backgroundCtx = null;
  }

  /**
   * Get current mode strategy
   * @returns {BrushModeStrategy}
   */
  getStrategy() {
    return getStrategy(this.params.mode);
  }

  /**
   * Override setCanvas to create background layer
   */
  setCanvas(canvas) {
    super.setCanvas(canvas);

    // Create background canvas for finalized content
    this.backgroundCanvas = document.createElement('canvas');
    this.backgroundCanvas.width = canvas.width;
    this.backgroundCanvas.height = canvas.height;
    this.backgroundCtx = this.backgroundCanvas.getContext('2d');
  }

  /**
   * Override startNewStroke to store the current mode with the stroke
   * Also bakes previous content to background before starting new stroke
   */
  startNewStroke(x, y) {
    // Bake all previous content to background before starting new stroke
    this.bakeToBackground();

    this.strokeCounter++;
    this._smoothedWidth = this.params.brushWidth;
    this.currentStroke = {
      points: [{ x, y, time: Date.now() }],
      color: { ...this.params.color },
      width: this.params.brushWidth,
      opacity: this.params.opacity,
      mode: this.params.mode,
      shadowColor: this.params.shadowColor,
      strokeIndex: this.strokeCounter
    };
    this.strokes.push(this.currentStroke);
  }

  /**
   * Override endStroke to mark stroke as complete
   */
  endStroke() {
    if (this.currentStroke) {
      this.currentStroke.complete = true;
    }
    this.currentStroke = null;
  }

  /**
   * Bake all completed strokes and drips to background canvas
   */
  bakeToBackground() {
    if (!this.backgroundCtx) return;

    // Only bake if there's content
    const completedStrokes = this.strokes.filter(s => s.complete && s.points.length >= 2);
    const hasContent = completedStrokes.length > 0 ||
                       this.dripManager.hasTrails() ||
                       this.dripManager.hasActiveDrips();
    if (!hasContent) return;

    // Finalize active drips
    this.dripManager.finalize();

    // Draw current content
    this.redraw();

    // Composite to background
    this.backgroundCtx.drawImage(this.canvas, 0, 0);

    // Clear working state
    this.strokes = this.strokes.filter(s => !s.complete);
    this.dripManager.clear();

    // Clear main canvas
    this.ctx.clearRect(0, 0, this.width, this.height);
  }

  /**
   * Continue the current stroke with velocity-based width
   */
  continueStroke(x, y) {
    if (!this.currentStroke) return;

    const points = this.currentStroke.points;
    const lastPoint = points[points.length - 1];

    // Calculate velocity
    const dx = x - lastPoint.x;
    const dy = y - lastPoint.y;
    const dist = Math.sqrt(dx * dx + dy * dy);
    const dt = Date.now() - lastPoint.time;
    const velocity = dt > 0 ? dist / dt : 0;

    // Calculate target width based on velocity with deadzone
    // Velocity below threshold doesn't affect width (prevents jitter at slow speeds)
    let targetWidth = this.params.brushWidth;
    if (this.params.velocityScale > 0) {
      const deadzone = 0.15; // px/ms — below this, treat as zero velocity
      const effectiveVelocity = Math.max(0, velocity - deadzone);
      const velocityFactor = 1 - Math.min(effectiveVelocity * this.params.velocityScale, 0.8);
      targetWidth = this.params.minWidth +
        (this.params.maxWidth - this.params.minWidth) * velocityFactor;
    }

    // Smooth width with EMA to prevent jitter from detection noise
    // smoothing param controls inertia: 0 = instant, 1 = frozen
    const alpha = 1 - this.params.smoothing;
    this._smoothedWidth = this._smoothedWidth * (1 - alpha) + targetWidth * alpha;
    const width = this._smoothedWidth;

    // Only add point if moved enough (higher threshold reduces jaggedness)
    if (dist > 3) {
      this.currentStroke.points.push({
        x,
        y,
        time: Date.now(),
        width,
        velocity
      });

      // Render the new segment
      this.renderSegment(points.length - 1);

      // Spawn drips
      const strategy = this.getStrategy();
      if (this.params.dripsEnabled && strategy.supportsDrips()) {
        const maxFreq = 120;
        const probability = this.params.dripsFrequency / maxFreq;
        if (Math.random() < probability) {
          this.spawnDrip(x, y);
        }
      }
    }
  }

  /**
   * Spawn a drip at position
   */
  spawnDrip(x, y, colorOverride = null) {
    let dripColor;
    if (colorOverride) {
      const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(colorOverride);
      if (result) {
        dripColor = {
          r: parseInt(result[1], 16),
          g: parseInt(result[2], 16),
          b: parseInt(result[3], 16)
        };
      } else {
        dripColor = { ...this.params.color };
      }
    } else {
      dripColor = { ...this.params.color };
    }

    this.dripManager.spawn({
      x,
      y,
      direction: this.params.dripsDirection,
      speed: this.params.dripsSpeed,
      width: this.params.dripsWidth,
      color: dripColor,
      opacity: colorOverride ? 0.7 : this.params.opacity,
      canvasWidth: this.width,
      canvasHeight: this.height,
      strokeIndex: this.currentStroke ? this.currentStroke.strokeIndex : this.strokeCounter
    });
  }

  /**
   * Render a single segment during live drawing
   */
  renderSegment(segmentIndex) {
    if (!this.currentStroke || segmentIndex < 1) return;

    const points = this.currentStroke.points;
    const p0 = points[segmentIndex - 1];
    const p1 = points[segmentIndex];
    const strategy = this.getStrategy();

    // Some modes need full redraw for proper shadow layering
    if (strategy.requiresFullRedraw()) {
      this.redraw();
      return;
    }

    strategy.renderSegment(this.ctx, p0, p1, this.currentStroke.color, this.params);
  }

  /**
   * Render frame - update drips and redraw if needed
   */
  render() {
    if (this.params.dripsEnabled && this.dripManager.hasActiveDrips()) {
      this.dripManager.update();
      this.redraw();
    }
  }

  /**
   * Redraw all strokes from scratch
   */
  redraw() {
    // Clear canvas
    this.ctx.clearRect(0, 0, this.width, this.height);

    // Draw baked background
    if (this.backgroundCanvas) {
      this.ctx.drawImage(this.backgroundCanvas, 0, 0);
    }

    const validStrokes = this.strokes.filter(s => s.points.length >= 2);

    // Draw each stroke with its drip trails
    for (const stroke of validStrokes) {
      const strategy = getStrategy(stroke.mode || 'smooth');

      // Draw shadow first (if mode has shadows)
      strategy.drawStrokeShadow(this.ctx, stroke, this.params);

      // Draw shadow decorations (arrow head shadows)
      if (strategy.drawShadowDecorations) {
        strategy.drawShadowDecorations(this.ctx, stroke, this.params);
      }

      // Draw main stroke
      strategy.drawStroke(this.ctx, stroke, this.params);

      // Draw decorations (arrow heads)
      strategy.drawStrokeDecorations(this.ctx, stroke, this.params);

      // Draw drip trails for this stroke
      this.dripManager.drawTrailsForStroke(this.ctx, stroke.strokeIndex, this.strokes);
    }

    // Draw orphan drip trails
    this.dripManager.drawTrailsForStroke(this.ctx, null, this.strokes);
  }

  /**
   * Clear everything
   */
  clear() {
    super.clear();
    this.dripManager.clear();
    this.strokeCounter = 0;
    if (this.backgroundCtx) {
      this.backgroundCtx.clearRect(0, 0, this.backgroundCanvas.width, this.backgroundCanvas.height);
    }
  }
}
