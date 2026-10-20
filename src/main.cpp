
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// =============================================
// TFT CONFIGURATION — ESP32
// Landscape: 320 x 240
// =============================================
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST    8
#define TFT_MOSI  11
#define TFT_SCLK  12

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

// =============================================
// RGB565 NEON COLORS
// =============================================
const uint16_t BG_BLACK  = 0x0000;
const uint16_t NEON_PINK = 0xF81F;
const uint16_t HOT_PINK  = 0xF9B6;
const uint16_t NEON_PURPLE = 0x801F;
const uint16_t NEON_CYAN = 0x07FF;
const uint16_t NEON_BLUE = 0x43FF;
const uint16_t GOLD      = 0xFEA0;
const uint16_t WHITE     = 0xFFFF;
const uint16_t RED       = 0xF800;
const uint16_t YELLOW    = 0xFFE0;
const uint16_t GREEN     = 0x07E0;

// =============================================
// STATE MACHINE
// =============================================
const unsigned long TOTAL_DURATION = 20000UL;
const unsigned long PHASE1_END = 6000UL;
const unsigned long PHASE2_END = 12000UL;

const unsigned long HEART_INTERVAL = 450UL;
const unsigned long TEXT_INTERVAL  = 300UL;
const unsigned long FIREWORK_INTERVAL = 220UL;

unsigned long animationStart = 0;
unsigned long lastEffect = 0;

int currentPhase = -1;
bool effectOn = false;

// =============================================
// HELPER: CENTER TEXT HORIZONTALLY
// Uses the built-in Adafruit GFX font
// =============================================
void drawCenteredText(const char *text,
                      int16_t y,
                      uint8_t size,
                      uint16_t color) {
  int16_t x1, y1;
  uint16_t w, h;

  tft.setFont();
  tft.setTextSize(size);
  tft.getTextBounds(text, 0, y, &x1, &y1, &w, &h);

  int16_t x = (320 - (int16_t)w) / 2 - x1;

  tft.setTextColor(color);
  tft.setCursor(x, y);
  tft.print(text);
}

// =============================================
// HEART: FILLED NEON-STYLE HEART
// x,y = center of heart
// =============================================
void drawHeart(int16_t x, int16_t y,
               int16_t s, uint16_t color) {
  // Two rounded lobes
  tft.fillCircle(x - s / 2, y - s / 4, s / 2, color);
  tft.fillCircle(x + s / 2, y - s / 4, s / 2, color);

  // Lower point
  tft.fillTriangle(
    x - s, y - s / 6,
    x + s, y - s / 6,
    x,     y + s,
    color
  );
}

// =============================================
// NEON HEART: LAYERED COLOR EFFECT
// =============================================
void drawNeonHeart(int16_t x, int16_t y, int16_t s) {
  drawHeart(x, y, s + 6, NEON_PURPLE);
  drawHeart(x, y, s, NEON_PINK);
}

// =============================================
// PHASE 0: INTRO
// 0–6 seconds
// =============================================
void drawPhase0() {
  tft.fillScreen(BG_BLACK);

  drawCenteredText("HAPPY", 30, 3, NEON_PURPLE);
  drawCenteredText("20/10", 68, 4, NEON_PINK);

  // Decorative neon lines
  tft.drawFastHLine(65, 120, 190, NEON_PURPLE);
  tft.drawFastHLine(90, 123, 140, NEON_PINK);

  // Static decorations
  drawNeonHeart(45, 178, 12);
  drawNeonHeart(275, 178, 12);

  drawCenteredText("WOMEN'S DAY", 204, 2, NEON_CYAN);
}

// =============================================
// PHASE 1: COLOR-CHANGING MESSAGE
// 6–12 seconds
// =============================================
void drawPhase1() {
  tft.fillScreen(BG_BLACK);

  drawCenteredText("HAPPY", 40, 3, NEON_PURPLE);

  // Reserve this region for changing text
  tft.fillRect(0, 82, 320, 100, BG_BLACK);

  drawCenteredText("WOMEN'S", 88, 3, NEON_PINK);
  drawCenteredText("DAY!", 126, 4, NEON_CYAN);

  // Small decorative hearts
  drawNeonHeart(35, 190, 9);
  drawNeonHeart(285, 190, 9);

  tft.drawFastHLine(75, 214, 170, NEON_PURPLE);
}

// =============================================
// FIREWORK BURST
// Fireworks stay outside the central text area
// =============================================
void drawFirework(int16_t cx, int16_t cy,
                  uint16_t color, int16_t radius) {
  // Eight rays
  for (int i = 0; i < 8; i++) {
    float angle = i * 0.785398f;

    int16_t x2 = cx + (int16_t)(cosf(angle) * radius);
    int16_t y2 = cy + (int16_t)(sinf(angle) * radius);

    tft.drawLine(cx, cy, x2, y2, color);

    // Spark at ray tip
    tft.fillCircle(x2, y2, 2, WHITE);
  }

  // Bright center
  tft.fillCircle(cx, cy, 3, color);
}

// =============================================
// PHASE 2: FIREWORKS + FINAL MESSAGE
// 12–20 seconds
// =============================================
void drawPhase2() {
  tft.fillScreen(BG_BLACK);

  drawCenteredText("LOVE &", 91, 3, NEON_PINK);
  drawCenteredText("HAPPINESS", 130, 2, NEON_CYAN);

  // Decorative borders around the message
  tft.drawFastHLine(80, 82, 160, NEON_PURPLE);
  tft.drawFastHLine(80, 169, 160, NEON_PURPLE);

  // Fixed corner hearts
  drawNeonHeart(25, 125, 7);
  drawNeonHeart(295, 125, 7);

  // Small footer
  drawCenteredText("HAPPY 20/10", 207, 2, GOLD);
}

// =============================================
// SELECT AND DRAW A NEW PHASE
// =============================================
void enterPhase(int phase, unsigned long now) {
  currentPhase = phase;
  lastEffect = now;
  effectOn = false;

  switch (currentPhase) {
    case 0:
      drawPhase0();
      break;

    case 1:
      drawPhase1();
      break;

    case 2:
      drawPhase2();
      break;
  }
}

// =============================================
// SETUP
// =============================================
void setup() {
  Serial.begin(115200);
  delay(200);

  // Explicit SPI pins
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  tft.init(240, 320);
  tft.setRotation(1);  // Landscape: 320 x 240
  tft.setTextWrap(false);
  tft.fillScreen(BG_BLACK);

  randomSeed((uint32_t)micros());

  animationStart = millis();

  enterPhase(0, animationStart);

  Serial.println("Neon 20/10 display started!");
}

// =============================================
// LOOP — NON-BLOCKING ANIMATION
// =============================================
void loop() {
  const unsigned long now = millis();

  // Modulo keeps the timeline repeating every 20 seconds
  const unsigned long elapsed =
      (now - animationStart) % TOTAL_DURATION;

  int newPhase;

  if (elapsed < PHASE1_END) {
    newPhase = 0;
  } else if (elapsed < PHASE2_END) {
    newPhase = 1;
  } else {
    newPhase = 2;
  }

  // Clear the screen only when entering a new phase
  if (newPhase != currentPhase) {
    enterPhase(newPhase, now);
  }

  // -------------------------------------------
  // PHASE 0: HEART BLINK
  // -------------------------------------------
  if (currentPhase == 0) {
    if (now - lastEffect >= HEART_INTERVAL) {
      lastEffect = now;
      effectOn = !effectOn;

      // Clear only the heart animation area
      tft.fillRect(90, 143, 140, 48, BG_BLACK);

      if (effectOn) {
        drawNeonHeart(160, 157, 17);
      } else {
        // Dim purple heart for a breathing effect
        drawHeart(160, 157, 17, NEON_PURPLE);
      }
    }
  }

  // -------------------------------------------
  // PHASE 1: ALTERNATING NEON TEXT COLORS
  // -------------------------------------------
  else if (currentPhase == 1) {
    if (now - lastEffect >= TEXT_INTERVAL) {
      lastEffect = now;
      effectOn = !effectOn;

      // Update only the text region
      tft.fillRect(0, 82, 320, 76, BG_BLACK);

      drawCenteredText(
        "WOMEN'S",
        88,
        3,
        effectOn ? NEON_PINK : NEON_PURPLE
      );

      drawCenteredText(
        "DAY!",
        126,
        4,
        effectOn ? NEON_CYAN : HOT_PINK
      );
    }
  }

  // -------------------------------------------
  // PHASE 2: FIREWORKS
  // -------------------------------------------
  else if (currentPhase == 2) {
    if (now - lastEffect >= FIREWORK_INTERVAL) {
      lastEffect = now;

      // Randomly choose upper or lower screen band
      int16_t x = random(12, 308);
      int16_t y;

      if (random(0, 2) == 0) {
        y = random(12, 65);
      } else {
        y = random(177, 234);
      }

      const uint16_t colors[] = {
        NEON_PINK,
        NEON_PURPLE,
        NEON_CYAN,
        NEON_BLUE,
        GOLD,
        RED,
        WHITE
      };

      uint16_t color = colors[random(0, 7)];
      int16_t radius = random(7, 19);

      drawFirework(x, y, color, radius);
    }
  }
}
