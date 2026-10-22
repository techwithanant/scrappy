// Touch test: the head pat. A small square of copper sheet with one wire,
// read by the ESP32-S3's built-in touch sensor. Prints the reading and says
// "pat!" when you touch the copper.
// Wiring (see README): copper pad wire -> GPIO 3 (touch pin T3). Nothing else.

const int PIN_TOUCH = 3;
const float TRIGGER = 1.3;  // a touch reads at least 1.3x the untouched level

uint32_t baseline = 0;

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Scrappy: touch test === (don't touch the copper for 2 s)");
  uint64_t sum = 0;
  for (int i = 0; i < 40; i++) {
    sum += touchRead(PIN_TOUCH);
    delay(50);
  }
  baseline = sum / 40;
  Serial.printf("Untouched level: %u. Now pat the copper.\n", baseline);
}

void loop() {
  uint32_t v = touchRead(PIN_TOUCH);
  bool touched = v > baseline * TRIGGER;
  Serial.printf("%7u %s\n", v, touched ? "<- pat!" : "");
  delay(150);
}
