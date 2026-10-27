
#include <TFT_eSPI.h>
#include <SPI.h>

// LILYGO T-Display-S3
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite canvas = TFT_eSprite(&tft);

const int SCREEN_W = 320;
const int SCREEN_H = 170;

// Cyberpunk colors: RGB565
const uint16_t BG       = 0x0841; // Dark blue-black
const uint16_t CYAN     = 0x07FF;
const uint16_t MAGENTA  = 0xF81F;
const uint16_t WHITE    = 0xFFFF;
const uint16_t PURPLE   = 0x781F;
const uint16_t DARKCYAN = 0x0328;

int countdown = 10;

unsigned long lastSecond = 0;
unsigned long lastFrame = 0;

const unsigned long COUNT_INTERVAL = 1000;
const unsigned long FRAME_INTERVAL = 70;

// Countdown begins after setup
unsigned long startTime;

// Draw a neon-style eye
void drawOpenEye(int x, int y) {
  // Outer neon glow effect
  canvas.drawRoundRect(x - 3, y - 3, 48, 35, 10, MAGENTA);
  canvas.drawRoundRect(x, y, 42, 29, 8, CYAN);

  // Eye interior
  canvas.fillRoundRect(x + 4, y + 4, 34, 21, 6, DARKCYAN);
  canvas.fillCircle(x + 21, y + 14, 8, CYAN);
  canvas.fillCircle(x + 21, y + 14, 4, WHITE);

  // Small highlight
  canvas.fillCircle(x + 18, y + 11, 2, WHITE);
}

// Draw a winking eye
void drawWinkEye(int x, int y) {
  canvas.drawLine(x, y + 14, x + 8, y + 20, MAGENTA);
  canvas.drawLine(x + 8, y + 20, x + 21, y + 23, CYAN);
  canvas.drawLine(x + 21, y + 23, x + 34, y + 20, CYAN);
  canvas.drawLine(x + 34, y + 20, x + 42, y + 14, MAGENTA);

  // Extra neon lower edge
  canvas.drawLine(x + 7, y + 25, x + 34, y + 25, PURPLE);
}

// Draw a smiling mouth
void drawSmile(int cx, int cy) {
  // Polygonal cyberpunk smile
  canvas.drawLine(cx - 25, cy, cx - 20, cy + 9, MAGENTA);
  canvas.drawLine(cx - 20, cy + 9, cx - 10, cy + 15, CYAN);
  canvas.drawLine(cx - 10, cy + 15, cx, cy + 17, CYAN);
  canvas.drawLine(cx, cy + 17, cx + 10, cy + 15, CYAN);
  canvas.drawLine(cx + 10, cy + 15, cx + 20, cy + 9, CYAN);
  canvas.drawLine(cx + 20, cy + 9, cx + 25, cy, MAGENTA);

  // Mouth corner accents
  canvas.drawPixel(cx - 25, cy - 1, WHITE);
  canvas.drawPixel(cx + 25, cy - 1, WHITE);
}

// Small decorative HUD
void drawHUD() {
  // Corner brackets
  canvas.drawFastHLine(6, 7, 24, CYAN);
  canvas.drawFastVLine(6, 7, 12, CYAN);

  canvas.drawFastHLine(290, 7, 24, MAGENTA);
  canvas.drawFastVLine(313, 7, 12, MAGENTA);

  canvas.drawFastHLine(6, 156, 24, MAGENTA);
  canvas.drawFastVLine(6, 145, 12, MAGENTA);

  canvas.drawFastHLine(290, 156, 24, CYAN);
  canvas.drawFastVLine(313, 145, 12, CYAN);

  // Status labels
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextColor(CYAN, BG);
  canvas.setTextSize(1);
  canvas.drawString("NEO_AI", 35, 8);

  canvas.setTextColor(MAGENTA, BG);
  canvas.drawRightString("ONLINE", 285, 8, 1);

  // HUD divider
  canvas.drawFastHLine(35, 20, 250, PURPLE);
  canvas.drawFastHLine(35, 150, 250, PURPLE);

  // Decorative side indicators
  for (int i = 0; i < 4; i++) {
    uint16_t c = (i % 2 == 0) ? CYAN : MAGENTA;
    canvas.fillRect(13, 48 + i * 8, 4, 4, c);
    canvas.fillRect(303, 48 + i * 8, 4, 4, c);
  }
}

void drawFace() {
  canvas.fillSprite(BG);

  drawHUD();

  // Face position
  int eyeY = 42;

  // Wink for 220 ms every 2.6 seconds
  unsigned long phase = millis() % 2600;
  bool winking = (phase < 220);

  // Eyes
  drawOpenEye(104, eyeY);

  if (winking) {
    drawWinkEye(174, eyeY);
  } else {
    drawOpenEye(174, eyeY);
  }

  // Smile
  drawSmile(160, 99);

  // Countdown
  canvas.setTextDatum(MC_DATUM);
  canvas.setTextColor(MAGENTA, BG);
  canvas.setTextSize(2);
  canvas.drawString("T-MINUS", 160, 126, 1);

  canvas.setTextColor(CYAN, BG);
  canvas.setTextSize(2);
  canvas.drawNumber(countdown, 160, 141, 2);

  // Final state
  if (countdown == 0) {
    canvas.fillRoundRect(113, 27, 94, 17, 4, PURPLE);
    canvas.setTextColor(WHITE, PURPLE);
    canvas.setTextSize(1);
    canvas.drawCentreString("READY!", 160, 31, 1);
  }

  // Transfer completed frame to display
  canvas.pushSprite(0, 0);
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG);

  canvas.setColorDepth(16);

  if (!canvas.createSprite(SCREEN_W, SCREEN_H)) {
    Serial.println("Sprite allocation failed!");
    while (true) {
      delay(1000);
    }
  }

  startTime = millis();
  lastSecond = startTime;
  lastFrame = 0;

  drawFace();
}

void loop() {
  unsigned long now = millis();

  // Countdown: 10, 9, 8 ... 0
  if (countdown > 0 &&
      now - lastSecond >= COUNT_INTERVAL) {

    unsigned long elapsed = now - lastSecond;
    int secondsPassed = elapsed / COUNT_INTERVAL;

    countdown -= secondsPassed;

    if (countdown < 0) {
      countdown = 0;
    }

    lastSecond += secondsPassed * COUNT_INTERVAL;
  }

  // Animate the face without blocking the countdown
  if (now - lastFrame >= FRAME_INTERVAL) {
    lastFrame = now;
    drawFace();
  }
}
