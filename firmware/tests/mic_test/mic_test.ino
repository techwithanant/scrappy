// Mic test: reads the INMP441 and prints a live voice-level bar.
// Wiring (see README): SCK -> GPIO 42, WS -> GPIO 41, SD -> GPIO 1, L/R -> GND, VDD -> 3V3.

#include <ESP_I2S.h>
#include <math.h>

const int PIN_SCK = 42;
const int PIN_WS = 41;
const int PIN_SD = 1;
const int SAMPLE_RATE = 16000;

I2SClass mic;
int32_t buf[512];  // stereo frames: left, right, left, right...

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Scrappy: mic test ===");

  mic.setPins(PIN_SCK, PIN_WS, -1, PIN_SD);
  if (!mic.begin(I2S_MODE_STD, SAMPLE_RATE, I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO)) {
    Serial.println("I2S start FAILED");
    while (true) delay(1000);
  }
  Serial.println("I2S started. Talk, clap or blow near the mic.");
}

// Level of one channel in dBFS (0 = loudest possible).
float levelDb(int ch, int frames, int32_t &peak) {
  double sum = 0;
  peak = 0;
  for (int i = 0; i < frames; i++) {
    int32_t s = buf[i * 2 + ch] >> 8;  // INMP441 sends 24 bits in the top of a 32-bit slot
    sum += (double)s * s;
    if (abs(s) > peak) peak = abs(s);
  }
  double rms = sqrt(sum / frames);
  return rms < 1 ? -120 : 20 * log10(rms / 8388608.0);
}

void loop() {
  size_t got = mic.readBytes((char *)buf, sizeof(buf));
  int frames = got / 8;
  if (frames == 0) return;

  static uint32_t last = 0;
  if (millis() - last < 100) return;
  last = millis();

  int32_t peakL, peakR;
  float dbL = levelDb(0, frames, peakL);
  float dbR = levelDb(1, frames, peakR);
  bool useLeft = peakL >= peakR;
  float db = useLeft ? dbL : dbR;
  int32_t peak = useLeft ? peakL : peakR;

  if (peak == 0) {
    Serial.println("silence: all zeros -> check the SD wire (GPIO 1) and VDD (3V3)");
    return;
  }

  int bars = constrain((int)((db + 90) / 2), 0, 45);  // -90 dB..0 dB -> 0..45 chars
  char line[64];
  memset(line, ' ', 45);
  memset(line, '#', bars);
  line[45] = 0;
  Serial.printf("%s |%s| %6.1f dB\n", useLeft ? "L" : "R", line, db);
}
