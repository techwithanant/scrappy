// OLED face: draws two eyes and a smile on the 1.5" 128x128 colour OLED
// (SSD1351 driver), and blinks every few seconds.
// Wiring (see README): SCK -> GPIO 21, SDA -> GPIO 47, CS -> GPIO 14, DC -> GPIO 2,
// RST -> EN, VDD -> 3V3, GND -> GND. No libraries needed.

#include <SPI.h>

const int PIN_SCK = 21;
const int PIN_MOSI = 47;
const int PIN_CS = 14;
const int PIN_DC = 2;

const uint16_t EYES = 0x07FF;   // cyan (RGB565)
const uint16_t SMILE = 0xFD20;  // orange

uint16_t fb[128 * 128];  // RGB565, stored big-endian for the panel

void send(bool data, uint8_t b) {
  digitalWrite(PIN_DC, data ? HIGH : LOW);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(b);
  digitalWrite(PIN_CS, HIGH);
}

// SSD1351 takes command parameters with DC high: {command, param count, params...}.
void cmds(const uint8_t *seq, size_t len) {
  for (size_t i = 0; i < len;) {
    send(false, seq[i]);
    uint8_t n = seq[i + 1];
    for (uint8_t k = 0; k < n; k++) send(true, seq[i + 2 + k]);
    i += 2 + n;
  }
}

void initDisplay() {
  const uint8_t seq[] = {
    0xFD, 1, 0x12, 0xFD, 1, 0xB1,  // unlock
    0xAE, 0,                       // display off
    0xB3, 1, 0xF1,                 // clock
    0xCA, 1, 0x7F,                 // 128 rows
    0xA0, 1, 0x74,                 // remap: 65k colour, COM split
    0xA1, 1, 0x00, 0xA2, 1, 0x00,  // start line, offset
    0xB5, 1, 0x00, 0xAB, 1, 0x01,  // GPIO off, internal regulator on
    0xB1, 1, 0x32, 0xBE, 1, 0x05,  // precharge, VCOMH
    0xC1, 3, 0xC8, 0x80, 0xC8,     // contrast A/B/C
    0xC7, 1, 0x0F,                 // master contrast
    0xB4, 3, 0xA0, 0xB5, 0x55,     // segment low voltage
    0xB6, 1, 0x01,                 // second precharge
    0xA6, 0,                       // normal display
    0xAF, 0,                       // display on
  };
  cmds(seq, sizeof(seq));
}

void show() {
  const uint8_t win[] = {0x15, 2, 0x00, 0x7F, 0x75, 2, 0x00, 0x7F};
  cmds(win, sizeof(win));
  send(false, 0x5C);  // write RAM
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  SPI.writeBytes((uint8_t *)fb, sizeof(fb));
  digitalWrite(PIN_CS, HIGH);
}

void px(int x, int y, uint16_t c) {
  if (x < 0 || x > 127 || y < 0 || y > 127) return;
  fb[y * 128 + x] = (c >> 8) | (c << 8);  // swap to big-endian
}

void ellipse(int cx, int cy, int rx, int ry, uint16_t c) {
  for (int y = -ry; y <= ry; y++)
    for (int x = -rx; x <= rx; x++)
      if ((float)x * x / (rx * rx) + (float)y * y / (ry * ry) <= 1.0f) px(cx + x, cy + y, c);
}

void drawFace(bool blink) {
  memset(fb, 0, sizeof(fb));
  int eyeH = blink ? 2 : 18;
  ellipse(40, 52, 14, eyeH, EYES);
  ellipse(88, 52, 14, eyeH, EYES);
  for (int x = -24; x <= 24; x++) {  // smile
    int y = 100 - (x * x) / 40;  // ends curve up (screen y grows downward)
    for (int t = 0; t < 4; t++) px(64 + x, y + t - 8, SMILE);
  }
  show();
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_CS, OUTPUT);
  pinMode(PIN_DC, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  delay(200);  // RST is tied to EN, so the panel resets with the board
  initDisplay();
  drawFace(false);
  Serial.println("OLED face (SSD1351): face drawn");
}

void loop() {
  delay(random(2000, 4500));
  drawFace(true);
  delay(140);
  drawFace(false);
  Serial.println("blink");
}
