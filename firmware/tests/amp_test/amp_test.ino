// Amp test: first sound for the mouth. Plays a little rising "hello" chirp
// (four notes) through the MAX98357A every 3 seconds.
// Wiring (see README): BCLK -> GPIO 40, LRC -> GPIO 39, DIN -> GPIO 38,
// VIN -> 5V, GND -> GND, speaker on the screw terminal.

#include <ESP_I2S.h>
#include <math.h>

const int RATE = 16000;
const int16_t VOLUME = 16000;  // out of 32767

I2SClass amp;
int16_t buf[512];  // stereo: each sample goes to both left and right

void tone(float freq, int ms) {
  int total = RATE * ms / 1000;
  static float phase = 0;
  for (int done = 0; done < total;) {
    int n = min(256, total - done);
    for (int i = 0; i < n; i++) {
      int left = total - done - i;  // short fade in/out so the notes don't click
      float env = min(1.0f, min((done + i) / 160.0f, left / 160.0f));
      buf[2 * i] = buf[2 * i + 1] = (int16_t)(VOLUME * env * sinf(phase));
      phase += 2 * PI * freq / RATE;
      if (phase > 2 * PI) phase -= 2 * PI;
    }
    amp.write((uint8_t *)buf, n * 4);
    done += n;
  }
}

void silence(int ms) {
  memset(buf, 0, sizeof(buf));
  for (int total = RATE * ms / 1000; total > 0; total -= 256) amp.write((uint8_t *)buf, min(256, total) * 4);
}

void setup() {
  Serial.begin(115200);
  amp.setPins(40, 39, 38);  // BCLK, LRC (WS), DIN
  if (!amp.begin(I2S_MODE_STD, RATE, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("I2S start failed");
    while (true) delay(1000);
  }
  Serial.println("Amp test: chirping every 3 s");
}

void loop() {
  const float notes[] = {523.25f, 659.25f, 783.99f, 1046.5f};  // C5 E5 G5 C6
  for (float f : notes) tone(f, 120);
  silence(3000);
  Serial.println("chirp");
}
