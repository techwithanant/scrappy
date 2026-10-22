// Pin check: drives every pin Scrappy uses HIGH (3.3 V), so each one can be
// measured with a multimeter in the breadboard. A pin that reads 0 V in the
// breadboard has a bad solder joint between the header pin and the board.
// Run it with ONLY the ESP32 on the breadboard (nothing else wired).

const int PINS[] = {1, 2, 3, 14, 21, 38, 39, 40, 41, 42, 45, 47};

void setup() {
  for (int p : PINS) {
    pinMode(p, OUTPUT);
    digitalWrite(p, HIGH);
  }
}

void loop() {}
