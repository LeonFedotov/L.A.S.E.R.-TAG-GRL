/**
 * Camera module - handles webcam access via WebRTC
 */
export class Camera {
  constructor() {
    this.video = null;
    this.stream = null;
    this.isReady = false;
    this.width = 640;
    this.height = 480;
    this.flipH = false;   // Flip horizontally
    this.flipV = false;   // Flip vertically
    this.rotation = 0;    // Rotation in degrees (0, 90, 180, 270)
    this.isVideoFile = false;  // True when playing a video file instead of camera
  }

  /**
   * Initialize camera with specified constraints
   * @param {HTMLVideoElement} videoElement - Video element to attach stream to
   * @param {Object} options - Camera options
   * @returns {Promise<void>}
   */
  async init(videoElement, options = {}) {
    this.video = videoElement;

    // Check for secure context (HTTPS required for camera access)
    if (!window.isSecureContext) {
      throw new Error('Camera access requires HTTPS. Please use https:// or localhost');
    }

    // Check for mediaDevices API
    if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
      throw new Error('Camera API not available. Please use a modern browser with HTTPS');
    }

    const constraints = {
      video: {
        width: { ideal: options.width || 640 },
        height: { ideal: options.height || 480 },
        facingMode: options.facingMode || 'environment',
        frameRate: { ideal: options.frameRate || 30 }
      },
      audio: false
    };

    try {
      this.stream = await navigator.mediaDevices.getUserMedia(constraints);
      this.video.srcObject = this.stream;

      // Wait for video to be ready
      await new Promise((resolve, reject) => {
        this.video.onloadedmetadata = () => {
          this.video.play()
            .then(resolve)
            .catch(reject);
        };
        this.video.onerror = reject;
      });

      // Get actual dimensions
      this.width = this.video.videoWidth;
      this.height = this.video.videoHeight;
      this.isReady = true;

      console.log(`Camera initialized: ${this.width}x${this.height}`);
    } catch (error) {
      console.error('Camera initialization failed:', error);
      throw error;
    }
  }

  /**
   * Get list of available cameras
   * @returns {Promise<MediaDeviceInfo[]>}
   */
  static async getAvailableCameras() {
    const devices = await navigator.mediaDevices.enumerateDevices();
    return devices.filter(device => device.kind === 'videoinput');
  }

  /**
   * Switch to a different camera
   * @param {string} deviceId - Device ID to switch to
   */
  async switchCamera(deviceId) {
    if (this.stream) {
      this.stop();
    }

    const constraints = {
      video: {
        deviceId: { exact: deviceId },
        width: { ideal: this.width },
        height: { ideal: this.height }
      },
      audio: false
    };

    this.stream = await navigator.mediaDevices.getUserMedia(constraints);
    this.video.srcObject = this.stream;
    await this.video.play();

    this.width = this.video.videoWidth;
    this.height = this.video.videoHeight;
    this.isReady = true;
  }

  /**
   * Change camera resolution
   * @param {number} width - Desired width
   * @param {number} height - Desired height
   */
  async setResolution(width, height) {
    if (!this.stream) return;

    const videoTrack = this.stream.getVideoTracks()[0];
    if (!videoTrack) return;

    const deviceId = videoTrack.getSettings().deviceId;

    // Stop current stream
    this.stop();

    // Restart with new resolution
    const constraints = {
      video: {
        deviceId: deviceId ? { exact: deviceId } : undefined,
        width: { ideal: width },
        height: { ideal: height }
      },
      audio: false
    };

    this.stream = await navigator.mediaDevices.getUserMedia(constraints);
    this.video.srcObject = this.stream;
    await this.video.play();

    this.width = this.video.videoWidth;
    this.height = this.video.videoHeight;
    this.isReady = true;
    console.log(`Resolution changed to: ${this.width}x${this.height}`);
  }

  /**
   * Get current video frame as ImageData
   * @param {CanvasRenderingContext2D} ctx - Canvas context to use for capture
   * @returns {ImageData|null}
   */
  getFrame(ctx) {
    if (!this.isReady) return null;

    const isRotated90or270 = this.rotation === 90 || this.rotation === 270;
    const outWidth = isRotated90or270 ? this.height : this.width;
    const outHeight = isRotated90or270 ? this.width : this.height;

    ctx.save();

    // Apply rotation and flips
    ctx.translate(outWidth / 2, outHeight / 2);

    // Apply rotation
    if (this.rotation !== 0) {
      ctx.rotate(this.rotation * Math.PI / 180);
    }

    // Apply flips
    ctx.scale(
      this.flipH ? -1 : 1,
      this.flipV ? -1 : 1
    );

    ctx.translate(-this.width / 2, -this.height / 2);
    ctx.drawImage(this.video, 0, 0, this.width, this.height);
    ctx.restore();

    return ctx.getImageData(0, 0, outWidth, outHeight);
  }

  /**
   * Get output dimensions (accounting for rotation)
   * @returns {{width: number, height: number}}
   */
  getOutputDimensions() {
    const isRotated90or270 = this.rotation === 90 || this.rotation === 270;
    return {
      width: isRotated90or270 ? this.height : this.width,
      height: isRotated90or270 ? this.width : this.height
    };
  }

  /**
   * Toggle horizontal flip
   * @param {boolean} enabled - Whether to flip horizontally
   */
  setFlipH(enabled) {
    this.flipH = enabled;
  }

  /**
   * Toggle vertical flip
   * @param {boolean} enabled - Whether to flip vertically
   */
  setFlipV(enabled) {
    this.flipV = enabled;
  }

  /**
   * Set rotation angle
   * @param {number} degrees - Rotation in degrees (0, 90, 180, 270)
   */
  setRotation(degrees) {
    // Normalize to 0, 90, 180, 270
    this.rotation = ((degrees % 360) + 360) % 360;
    // Snap to nearest 90 degrees
    this.rotation = Math.round(this.rotation / 90) * 90;
  }

  /**
   * @deprecated Use setFlipH instead
   */
  setMirror(enabled) {
    this.flipH = enabled;
  }

  /**
   * Initialize from a video file URL instead of a webcam
   * @param {HTMLVideoElement} videoElement - Video element to attach to
   * @param {string} url - Video file URL
   * @returns {Promise<void>}
   */
  async initFromVideo(videoElement, url) {
    // Stop any existing stream
    this.stop();

    this.video = videoElement;
    this.video.srcObject = null;
    this.video.src = url;
    this.video.loop = true;
    this.video.muted = true;

    await new Promise((resolve, reject) => {
      this.video.onloadedmetadata = () => {
        this.video.play()
          .then(resolve)
          .catch(reject);
      };
      this.video.onerror = () => reject(new Error(`Failed to load video: ${url}`));
    });

    // Upscale small videos so laser dots are large enough for contour detection
    const nativeW = this.video.videoWidth;
    const nativeH = this.video.videoHeight;
    const MIN_DIM = 640;
    if (nativeW < MIN_DIM && nativeH < MIN_DIM) {
      const scale = Math.ceil(MIN_DIM / Math.max(nativeW, nativeH));
      this.width = nativeW * scale;
      this.height = nativeH * scale;
    } else {
      this.width = nativeW;
      this.height = nativeH;
    }
    this.isReady = true;
    this.isVideoFile = true;

    console.log(`Video file loaded: ${url} (native ${nativeW}x${nativeH}, output ${this.width}x${this.height})`);
  }

  /**
   * Stop camera stream or video playback
   */
  stop() {
    if (this.stream) {
      this.stream.getTracks().forEach(track => track.stop());
      this.stream = null;
    }
    if (this.video && this.isVideoFile) {
      this.video.pause();
      this.video.src = '';
      this.isVideoFile = false;
    }
    this.isReady = false;
  }

  /**
   * Check if camera is available
   * @returns {boolean}
   */
  static isSupported() {
    return !!(navigator.mediaDevices && navigator.mediaDevices.getUserMedia);
  }
}
