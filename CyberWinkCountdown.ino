/*
  Cyberpunk Winking Smiley Countdown (10 -> 0)
  Board : Waveshare ESP32-C6-LCD-1.47 (ST7789, 172x320 IPS)
  Lib   : GFX Library for Arduino (Arduino_GFX_Library) by moononournation

  Uses an off-screen canvas (framebuffer) so the animation is flicker-free.
*/

#include <Arduino_GFX_Library.h>

// ---------- Waveshare ESP32-C6-LCD-1.47 pinout ----------
#define LCD_MOSI 6
#define LCD_SCLK 7
#define LCD_CS   14
#define LCD_DC   15
#define LCD_RST  21
#define LCD_BL   22

#define SCREEN_W 172
#define SCREEN_H 320

// Arduino_ESP32SPI isn't built for the ESP32-C6, so use the generic hardware SPI bus
Arduino_DataBus *bus = new Arduino_HWSPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, GFX_NOT_DEFINED);
// ST7789 172x320 panel: column offset 34 on both sides
Arduino_GFX *panel = new Arduino_ST7789(bus, LCD_RST, 0 /* rotation */, true /* IPS */,
                                        SCREEN_W, SCREEN_H, 34, 0, 34, 0);
Arduino_Canvas *gfx = new Arduino_Canvas(SCREEN_W, SCREEN_H, panel);

// ---------- Cyberpunk palette (RGB565) ----------
constexpr uint16_t c565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
const uint16_t COL_BG       = c565(6, 0, 18);      // deep night purple
const uint16_t COL_GRID     = c565(40, 0, 70);     // dim violet grid
const uint16_t COL_SCAN     = c565(14, 0, 30);     // scanline
const uint16_t COL_CYAN     = c565(0, 255, 240);   // neon cyan
const uint16_t COL_CYAN_DIM = c565(0, 90, 100);
const uint16_t COL_MAGENTA  = c565(255, 0, 170);   // hot pink
const uint16_t COL_MAG_DIM  = c565(100, 0, 70);
const uint16_t COL_YELLOW   = c565(255, 230, 0);   // warning yellow
const uint16_t COL_WHITE    = c565(255, 255, 255);

// ---------- Layout ----------
const int FACE_CX = SCREEN_W / 2;
const int FACE_CY = 100;
const int FACE_R  = 56;

// ---------- Helpers ----------

// Thick arc made of small filled circles. Angles in degrees,
// 0 = 3 o'clock, positive = clockwise (screen Y points down).
void thickArc(int cx, int cy, int r, int a0, int a1, int thick, uint16_t color) {
  for (int a = a0; a <= a1; a += 3) {
    float rad = a * DEG_TO_RAD;
    int x = cx + (int)(r * cos(rad));
    int y = cy + (int)(r * sin(rad));
    gfx->fillCircle(x, y, thick, color);
  }
}

// Centered text using the built-in 6x8 font
void centerText(const char *txt, int y, uint8_t size, uint16_t color, int dx = 0) {
  int w = strlen(txt) * 6 * size;
  gfx->setTextSize(size);
  gfx->setTextColor(color);
  gfx->setCursor((SCREEN_W - w) / 2 + dx, y);
  gfx->print(txt);
}

void drawBackground() {
  gfx->fillScreen(COL_BG);

  // Neon grid
  for (int x = 0; x < SCREEN_W; x += 16) gfx->drawFastVLine(x, 0, SCREEN_H, COL_GRID);
  for (int y = 0; y < SCREEN_H; y += 16) gfx->drawFastHLine(0, y, SCREEN_W, COL_GRID);

  // Scanlines
  for (int y = 1; y < SCREEN_H; y += 3) gfx->drawFastHLine(0, y, SCREEN_W, COL_SCAN);

  // HUD corner brackets
  const int L = 14;
  gfx->drawFastHLine(2, 2, L, COL_CYAN);              gfx->drawFastVLine(2, 2, L, COL_CYAN);
  gfx->drawFastHLine(SCREEN_W - 2 - L, 2, L, COL_CYAN); gfx->drawFastVLine(SCREEN_W - 3, 2, L, COL_CYAN);
  gfx->drawFastHLine(2, SCREEN_H - 3, L, COL_MAGENTA);  gfx->drawFastVLine(2, SCREEN_H - 2 - L, L, COL_MAGENTA);
  gfx->drawFastHLine(SCREEN_W - 2 - L, SCREEN_H - 3, L, COL_MAGENTA);
  gfx->drawFastVLine(SCREEN_W - 3, SCREEN_H - 2 - L, L, COL_MAGENTA);

  // Header
  gfx->setTextSize(1);
  gfx->setTextColor(COL_MAGENTA);
  gfx->setCursor(20, 8);
  gfx->print("NEO//TOKYO.SYS");
  gfx->setTextColor(COL_CYAN_DIM);
  gfx->setCursor(SCREEN_W - 38, 8);
  gfx->print("v2.077");
}

// Open eye: rounded neon "visor" with a highlight
void drawOpenEye(int ex, int ey) {
  gfx->fillRoundRect(ex - 10, ey - 13, 20, 26, 7, COL_MAG_DIM);   // glow
  gfx->fillRoundRect(ex - 8, ey - 11, 16, 22, 6, COL_MAGENTA);
  gfx->fillRoundRect(ex - 4, ey - 8, 5, 7, 2, COL_WHITE);          // shine
}

// Closed (winking) eye: happy "^" curve
void drawWinkEye(int ex, int ey) {
  thickArc(ex, ey + 6, 11, 200, 340, 2, COL_MAGENTA);
}

void drawFace(bool leftClosed, bool rightClosed, int jitterX) {
  int cx = FACE_CX + jitterX;
  int cy = FACE_CY;

  // Outer glow ring + thick neon outline
  gfx->drawCircle(cx, cy, FACE_R + 4, COL_CYAN_DIM);
  gfx->drawCircle(cx, cy, FACE_R + 3, COL_CYAN_DIM);
  for (int i = 0; i < 3; i++) gfx->drawCircle(cx, cy, FACE_R - i, COL_CYAN);

  // Circuit tick marks around the ring
  for (int a = 0; a < 360; a += 30) {
    float rad = a * DEG_TO_RAD;
    int x1 = cx + (int)((FACE_R + 6) * cos(rad));
    int y1 = cy + (int)((FACE_R + 6) * sin(rad));
    int x2 = cx + (int)((FACE_R + 11) * cos(rad));
    int y2 = cy + (int)((FACE_R + 11) * sin(rad));
    gfx->drawLine(x1, y1, x2, y2, (a % 60 == 0) ? COL_MAGENTA : COL_CYAN_DIM);
  }

  // Eyes
  int eyL = cx - 21, eyR = cx + 21, eyY = cy - 14;
  if (leftClosed)  drawWinkEye(eyL, eyY);  else drawOpenEye(eyL, eyY);
  if (rightClosed) drawWinkEye(eyR, eyY);  else drawOpenEye(eyR, eyY);

  // Smile (glow + core)
  thickArc(cx, cy + 2, 32, 25, 155, 4, COL_CYAN_DIM);
  thickArc(cx, cy + 2, 32, 25, 155, 2, COL_CYAN);

  // Cheek "blush" circuitry
  for (int i = 0; i < 3; i++) {
    gfx->drawFastHLine(cx - 44, cy + 10 + i * 4, 10, COL_MAGENTA);
    gfx->drawFastHLine(cx + 34, cy + 10 + i * 4, 10, COL_MAGENTA);
  }
}

// Big countdown number with an RGB-split glitch effect
void drawNumber(int n, bool glitch) {
  char buf[4];
  snprintf(buf, sizeof(buf), "%d", n);

  centerText("T-MINUS", 172, 2, COL_YELLOW);

  int split = glitch ? random(3, 7) : 2;
  centerText(buf, 196, 8, COL_MAGENTA, -split);
  centerText(buf, 196, 8, COL_CYAN, split);
  centerText(buf, 196, 8, COL_WHITE, 0);
}

void drawProgressBar(int n) {
  const int x = 16, y = 278, w = SCREEN_W - 32, h = 12;
  gfx->drawRect(x, y, w, h, COL_CYAN);
  int segW = (w - 4) / 10;
  for (int i = 0; i < 10; i++) {
    uint16_t c = (i < n) ? COL_MAGENTA : COL_MAG_DIM;
    gfx->fillRect(x + 2 + i * segW, y + 2, segW - 2, h - 4, c);
  }
}

void drawFooter(bool cursorOn) {
  gfx->setTextSize(1);
  gfx->setTextColor(COL_CYAN);
  gfx->setCursor(18, 302);
  gfx->print("> uplink stable");
  if (cursorOn) gfx->fillRect(18 + 15 * 6 + 2, 302, 5, 8, COL_CYAN);
}

// Random horizontal glitch bars
void drawGlitchBars() {
  int bars = random(2, 5);
  for (int i = 0; i < bars; i++) {
    int y = random(20, SCREEN_H - 20);
    int x = random(0, SCREEN_W / 2);
    int w = random(20, SCREEN_W - x);
    gfx->fillRect(x, y, w, random(1, 4), random(2) ? COL_MAGENTA : COL_CYAN);
  }
}

// ---------- Main ----------

void setup() {
  Serial.begin(115200);

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);   // backlight on

  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
    while (true) delay(1000);
  }
  randomSeed(esp_random());
}

void runCountdown() {
  for (int n = 10; n >= 0; n--) {
    unsigned long start = millis();
    bool winkRightSide = (n % 2 == 0);   // alternate which eye winks

    while (millis() - start < 1000) {
      unsigned long t = millis() - start;

      bool winking = (t > 150 && t < 500);       // wink mid-second
      bool glitch  = (t < 120) || (random(100) < 4);
      int  jitter  = glitch ? random(-3, 4) : 0;

      drawBackground();
      if (n > 0) {
        drawFace(winking && !winkRightSide, winking && winkRightSide, jitter);
      } else {
        drawFace(true, true, jitter);            // double happy wink at zero
      }
      drawNumber(n, glitch);
      drawProgressBar(n);
      drawFooter((millis() / 400) % 2);
      if (glitch) drawGlitchBars();

      gfx->flush();
      delay(25);
    }
  }
}

void showFinale() {
  unsigned long start = millis();
  while (millis() - start < 3000) {
    bool flash = (millis() / 250) % 2;
    drawBackground();
    drawFace(true, true, random(100) < 10 ? random(-3, 4) : 0);
    centerText("SYSTEM", 190, 3, flash ? COL_MAGENTA : COL_CYAN);
    centerText("ONLINE", 222, 3, flash ? COL_CYAN : COL_MAGENTA);
    drawProgressBar(0);
    drawFooter(flash);
    if (random(100) < 15) drawGlitchBars();
    gfx->flush();
    delay(30);
  }
}

void loop() {
  runCountdown();
  showFinale();   // then loops and counts down again
}
