/**
 * CoordWarping - Handles perspective transformation for calibration
 * Transforms points from camera space to projection space
 */
import { storageSave, storageLoad } from '../utils/StorageUtils.js';

export class CoordWarping {
  constructor() {
    // Source quad (camera calibration points)
    this.srcQuad = [
      { x: 0, y: 0 },       // Top-left
      { x: 640, y: 0 },     // Top-right
      { x: 640, y: 480 },   // Bottom-right
      { x: 0, y: 480 }      // Bottom-left
    ];

    // Destination quad (projection/output)
    this.dstQuad = [
      { x: 0, y: 0 },
      { x: 1, y: 0 },
      { x: 1, y: 1 },
      { x: 0, y: 1 }
    ];

    // Transformation matrix (3x3 homography)
    this.matrix = null;
    this.inverseMatrix = null;

    // Calibration state
    this.isCalibrating = false;
    this.selectedPoint = -1;
  }

  /**
   * Set source dimensions (camera resolution)
   * @param {number} width - Camera width
   * @param {number} height - Camera height
   */
  setSourceDimensions(width, height) {
    // Initialize with default full-frame quad
    this.srcQuad = [
      { x: 0, y: 0 },
      { x: width, y: 0 },
      { x: width, y: height },
      { x: 0, y: height }
    ];
    this.computeMatrix();
  }

  /**
   * Set destination dimensions (projection resolution)
   * @param {number} width - Output width
   * @param {number} height - Output height
   */
  setDestinationDimensions(width, height) {
    this.dstQuad = [
      { x: 0, y: 0 },
      { x: width, y: 0 },
      { x: width, y: height },
      { x: 0, y: height }
    ];
    this.computeMatrix();
  }

  /**
   * Update a source calibration point
   * @param {number} index - Point index (0-3)
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   */
  setSourcePoint(index, x, y) {
    if (index >= 0 && index < 4) {
      this.srcQuad[index] = { x, y };
      this.computeMatrix();
    }
  }

  /**
   * Compute the perspective transformation matrix
   * Uses the standard homography calculation
   */
  computeMatrix() {
    const src = this.srcQuad;
    const dst = this.dstQuad;

    // Compute homography matrix using Direct Linear Transform (DLT)
    // Building the 8x9 matrix for SVD
    const A = [];

    for (let i = 0; i < 4; i++) {
      const sx = src[i].x;
      const sy = src[i].y;
      const dx = dst[i].x;
      const dy = dst[i].y;

      A.push([-sx, -sy, -1, 0, 0, 0, sx * dx, sy * dx, dx]);
      A.push([0, 0, 0, -sx, -sy, -1, sx * dy, sy * dy, dy]);
    }

    // Solve using simple Gaussian elimination for this specific case
    // For a proper implementation, use SVD, but this works for 4-point homography
    this.matrix = this.solveHomography(src, dst);
    this.inverseMatrix = this.solveHomography(dst, src);
  }

  /**
   * Solve homography matrix for 4-point correspondence
   * Simplified implementation using adjugate method
   */
  solveHomography(src, dst) {
    // Use a simplified direct method for 4-point homography
    // Based on the getPerspectiveTransform algorithm

    const x0 = src[0].x, y0 = src[0].y;
    const x1 = src[1].x, y1 = src[1].y;
    const x2 = src[2].x, y2 = src[2].y;
    const x3 = src[3].x, y3 = src[3].y;

    const u0 = dst[0].x, v0 = dst[0].y;
    const u1 = dst[1].x, v1 = dst[1].y;
    const u2 = dst[2].x, v2 = dst[2].y;
    const u3 = dst[3].x, v3 = dst[3].y;

    // Build matrix A and vector b for Ah = b
    const A = [
      [x0, y0, 1, 0, 0, 0, -u0*x0, -u0*y0],
      [0, 0, 0, x0, y0, 1, -v0*x0, -v0*y0],
      [x1, y1, 1, 0, 0, 0, -u1*x1, -u1*y1],
      [0, 0, 0, x1, y1, 1, -v1*x1, -v1*y1],
      [x2, y2, 1, 0, 0, 0, -u2*x2, -u2*y2],
      [0, 0, 0, x2, y2, 1, -v2*x2, -v2*y2],
      [x3, y3, 1, 0, 0, 0, -u3*x3, -u3*y3],
      [0, 0, 0, x3, y3, 1, -v3*x3, -v3*y3]
    ];

    const b = [u0, v0, u1, v1, u2, v2, u3, v3];

    // Solve using Gaussian elimination with partial pivoting
    const h = this.gaussianElimination(A, b);

    if (!h) {
      // Return identity matrix if solve fails
      return [1, 0, 0, 0, 1, 0, 0, 0, 1];
    }

    // Return 3x3 homography matrix as flat array
    return [h[0], h[1], h[2], h[3], h[4], h[5], h[6], h[7], 1];
  }

  /**
   * Gaussian elimination with partial pivoting
   */
  gaussianElimination(A, b) {
    const n = 8;
    const augmented = A.map((row, i) => [...row, b[i]]);

    // Forward elimination
    for (let col = 0; col < n; col++) {
      // Find pivot
      let maxRow = col;
      for (let row = col + 1; row < n; row++) {
        if (Math.abs(augmented[row][col]) > Math.abs(augmented[maxRow][col])) {
          maxRow = row;
        }
      }

      // Swap rows
      [augmented[col], augmented[maxRow]] = [augmented[maxRow], augmented[col]];

      // Check for singular matrix
      if (Math.abs(augmented[col][col]) < 1e-10) {
        return null;
      }

      // Eliminate column
      for (let row = col + 1; row < n; row++) {
        const factor = augmented[row][col] / augmented[col][col];
        for (let j = col; j <= n; j++) {
          augmented[row][j] -= factor * augmented[col][j];
        }
      }
    }

    // Back substitution
    const x = new Array(n);
    for (let i = n - 1; i >= 0; i--) {
      x[i] = augmented[i][n];
      for (let j = i + 1; j < n; j++) {
        x[i] -= augmented[i][j] * x[j];
      }
      x[i] /= augmented[i][i];
    }

    return x;
  }

  /**
   * Transform a point from source to destination space
   * @param {number} x - Source X coordinate
   * @param {number} y - Source Y coordinate
   * @returns {Object} - Transformed point {x, y}
   */
  transform(x, y) {
    if (!this.matrix) {
      return { x, y };
    }

    const m = this.matrix;

    // Apply homography: [x', y', w'] = H * [x, y, 1]
    const w = m[6] * x + m[7] * y + m[8];
    const xp = (m[0] * x + m[1] * y + m[2]) / w;
    const yp = (m[3] * x + m[4] * y + m[5]) / w;

    return { x: xp, y: yp };
  }

  /**
   * Transform a point from destination to source space (inverse)
   * @param {number} x - Destination X coordinate
   * @param {number} y - Destination Y coordinate
   * @returns {Object} - Transformed point {x, y}
   */
  inverseTransform(x, y) {
    if (!this.inverseMatrix) {
      return { x, y };
    }

    const m = this.inverseMatrix;

    const w = m[6] * x + m[7] * y + m[8];
    const xp = (m[0] * x + m[1] * y + m[2]) / w;
    const yp = (m[3] * x + m[4] * y + m[5]) / w;

    return { x: xp, y: yp };
  }

  /**
   * Check if a point is inside the source quad (camera calibration area)
   * Uses cross product method for convex polygon containment
   * @param {number} x - X coordinate in camera space
   * @param {number} y - Y coordinate in camera space
   * @returns {boolean} - True if point is inside the quad
   */
  isInsideSourceQuad(x, y) {
    const quad = this.srcQuad;

    // Cross product sign check for each edge
    // Point is inside if all cross products have same sign
    const sign = (p1, p2, p3) => {
      return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    };

    const point = { x, y };

    // Check each edge of the quad (in order: TL->TR->BR->BL)
    const d1 = sign(point, quad[0], quad[1]);
    const d2 = sign(point, quad[1], quad[2]);
    const d3 = sign(point, quad[2], quad[3]);
    const d4 = sign(point, quad[3], quad[0]);

    const hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0) || (d4 < 0);
    const hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0) || (d4 > 0);

    // Point is inside if all signs are the same (all positive or all negative)
    return !(hasNeg && hasPos);
  }

  /**
   * Check if a point is near a calibration point
   * @param {number} x - X coordinate
   * @param {number} y - Y coordinate
   * @param {number} threshold - Distance threshold
   * @returns {number} - Index of nearest point, or -1 if none within threshold
   */
  findNearestPoint(x, y, threshold = 35) {
    let minDist = threshold;
    let nearest = -1;

    for (let i = 0; i < this.srcQuad.length; i++) {
      const dx = this.srcQuad[i].x - x;
      const dy = this.srcQuad[i].y - y;
      const dist = Math.sqrt(dx * dx + dy * dy);

      if (dist < minDist) {
        minDist = dist;
        nearest = i;
      }
    }

    return nearest;
  }

  /**
   * Save calibration to localStorage
   * @param {string} key - Storage key
   */
  save(key = 'laserTagCalibration') {
    storageSave(key, { srcQuad: this.srcQuad, dstQuad: this.dstQuad });
  }

  /**
   * Load calibration from localStorage
   * @param {string} key - Storage key
   * @returns {boolean} - True if loaded successfully
   */
  load(key = 'laserTagCalibration') {
    const data = storageLoad(key, null, 'calibration');
    if (data && data.srcQuad && data.dstQuad) {
      this.srcQuad = data.srcQuad;
      this.dstQuad = data.dstQuad;
      this.computeMatrix();
      return true;
    }
    return false;
  }

  /**
   * Draw calibration quad visualization
   * @param {CanvasRenderingContext2D} ctx - Canvas context
   * @param {number} scaleX - X scale factor
   * @param {number} scaleY - Y scale factor
   */
  draw(ctx, scaleX = 1, scaleY = 1) {
    ctx.save();

    // Draw quad outline
    ctx.beginPath();
    ctx.moveTo(this.srcQuad[0].x * scaleX, this.srcQuad[0].y * scaleY);
    for (let i = 1; i < 4; i++) {
      ctx.lineTo(this.srcQuad[i].x * scaleX, this.srcQuad[i].y * scaleY);
    }
    ctx.closePath();
    ctx.strokeStyle = '#0ff';
    ctx.lineWidth = 2;
    ctx.stroke();

    // Draw points
    const labels = ['TL', 'TR', 'BR', 'BL'];
    for (let i = 0; i < 4; i++) {
      const x = this.srcQuad[i].x * scaleX;
      const y = this.srcQuad[i].y * scaleY;

      ctx.beginPath();
      ctx.arc(x, y, 16, 0, Math.PI * 2);
      ctx.fillStyle = this.selectedPoint === i ? '#ff0' : '#0ff';
      ctx.fill();
      ctx.strokeStyle = '#000';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Label
      ctx.fillStyle = '#000';
      ctx.font = 'bold 12px monospace';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText(labels[i], x, y);
    }

    ctx.restore();
  }
}
