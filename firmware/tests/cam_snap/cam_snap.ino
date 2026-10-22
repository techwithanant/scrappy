// Camera snapshot: takes one sharp JPEG photo (640x480) when the laptop sends
// "s" over USB serial, and sends it back as "JPEG <bytes>\n" followed by the
// raw file. tools/grab_photo.ps1 (Windows) saves it as a .jpg. Tests the camera
// on its own.
// Camera pins are fixed by the board's ribbon socket (Freenove ESP32-S3 CAM).

#include "esp_camera.h"

bool initCamera() {
  camera_config_t c = {};
  c.pin_pwdn = -1;
  c.pin_reset = -1;
  c.pin_xclk = 15;
  c.pin_sccb_sda = 4;
  c.pin_sccb_scl = 5;
  c.pin_d7 = 16;
  c.pin_d6 = 17;
  c.pin_d5 = 18;
  c.pin_d4 = 12;
  c.pin_d3 = 10;
  c.pin_d2 = 8;
  c.pin_d1 = 9;
  c.pin_d0 = 11;
  c.pin_vsync = 6;
  c.pin_href = 7;
  c.pin_pclk = 13;
  c.xclk_freq_hz = 20000000;
  c.ledc_timer = LEDC_TIMER_0;
  c.ledc_channel = LEDC_CHANNEL_0;
  c.pixel_format = PIXFORMAT_JPEG;
  c.frame_size = FRAMESIZE_VGA;
  c.jpeg_quality = 10;  // lower = better quality
  c.fb_count = 1;
  c.fb_location = CAMERA_FB_IN_PSRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;
  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("Camera init FAILED: 0x%x (%s)\n", err, esp_err_to_name(err));
    return false;
  }
  Serial.printf("Camera OK: sensor PID 0x%04X. Send 's' to take a photo.\n", esp_camera_sensor_get()->id.PID);
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  if (!initCamera()) while (true) delay(1000);
}

void loop() {
  if (Serial.read() != 's') return;
  for (int i = 0; i < 5; i++) esp_camera_fb_return(esp_camera_fb_get());  // let exposure settle
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("ERROR no frame");
    return;
  }
  Serial.printf("JPEG %u\n", fb->len);
  Serial.write(fb->buf, fb->len);
  Serial.flush();
  esp_camera_fb_return(fb);
}
