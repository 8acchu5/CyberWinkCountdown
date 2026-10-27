/*
  CYBER WINK COUNTDOWN  //  LilyGO T-Display-S3 (ST7789, 320x170)
  -----------------------------------------------------------------
  A neon smiley that winks on every tick while counting down 10 -> 0,
  with chromatic-aberration digits, glitch bursts and CRT scanlines.

  REQUIREMENTS
   - Board: "ESP32S3 Dev Module" (USB CDC On Boot: Enabled, PSRAM: OPI PSRAM)
   - Library: TFT_eSPI (2.5.x or newer, for drawArc / smooth shapes)
   - In TFT_eSPI/User_Setup_Select.h, comment out the default include and enable:
       #include <User_Setups/Setup206_LilyGo_T_Display_S3.h>

  CONTROLS
   - Right button (GPIO14): restart the countdown
*/

#include <TFT_eSPI.h>

#define PIN_POWER_ON   15   // must be HIGH to power the LCD on battery
#define PIN_LCD_BL     38
#define PIN_BUTTON_2   14

TFT_eSPI tft;
TFT_eSprite spr(&tft);

const int W = 320;
const int H = 170;

// ---- Cyberpunk palette ----
uint16_t C_BG, C_GRID, C_CYAN, C_MAG, C_YEL, C_WHITE, C_FACE, C_SCAN;

// ---- Layout ----
const int FACE_X = 86;
const int FACE_Y = 92;
const int FACE_R = 62;
const int NUM_X  = 244;
const int NUM_Y  = 88;

// ---- State ----
int count = 10;
unsigned long lastTick = 0;
const unsigned long TICK_MS = 1000;
const unsigned long WINK_MS = 380;    // how long the wink lasts after each tick
const unsigned long GLITCH_MS = 140;  // glitch burst after each tick

// -----------------------------------------------------------------
void drawBackground() {
  spr.fillSprite(C_BG);

  // faint grid
  for (int x = 0; x < W; x += 16) spr.drawFastVLine(x, 0, H, C_GRID);
  for (int y = 0; y < H; y += 16) spr.drawFastHLine(0, y, W, C_GRID);

  // header / footer text
  spr.setTextFont(1);
  spr.setTextSize(1);
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(C_CYAN);
  spr.drawString("NEON//FACE.EXE", 4, 3);
  spr.setTextDatum(TR_DATUM);
  spr.setTextColor(C_MAG);
  spr.drawString("SYS:ARMED", W - 4, 3);
}

// Corner brackets, HUD style
void drawBrackets(int x, int y, int w, int h, int len, uint16_t col) {
  spr.drawFastHLine(x, y, len, col);           spr.drawFastVLine(x, y, len, col);
  spr.drawFastHLine(x + w - len, y, len, col); spr.drawFastVLine(x + w - 1, y, len, col);
  spr.drawFastHLine(x, y + h - 1, len, col);   spr.drawFastVLine(x, y + h - len, len, col);
  spr.drawFastHLine(x + w - len, y + h - 1, len, col);
  spr.drawFastVLine(x + w - 1, y + h - len, len, col);
}

void drawFace(bool wink, bool grin) {
  int cx = FACE_X, cy = FACE_Y, R = FACE_R;

  // outer glow (fading magenta rings)
  for (int i = 4; i >= 1; i--) {
    uint16_t glow = spr.alphaBlend(200 - i * 45, C_MAG, C_BG);
    spr.drawCircle(cx, cy, R + i * 2, glow);
  }

  // face fill + neon ring
  spr.fillSmoothCircle(cx, cy, R - 4, C_FACE, C_BG);
  spr.drawArc(cx, cy, R, R - 4, 0, 360, C_CYAN, C_BG, true);

  // left eye: always open
  spr.fillEllipse(cx - 22, cy - 16, 8, 12, C_CYAN);
  spr.fillCircle(cx - 19, cy - 21, 3, C_WHITE);            // highlight

  // right eye: wink = closed happy arc
  if (wink) {
    spr.drawArc(cx + 22, cy - 10, 11, 7, 110, 250, C_MAG, C_FACE, true);
  } else {
    spr.fillEllipse(cx + 22, cy - 16, 8, 12, C_CYAN);
    spr.fillCircle(cx + 25, cy - 21, 3, C_WHITE);
  }

  // blush marks
  for (int i = 0; i < 3; i++) {
    spr.drawLine(cx - 38 + i * 5, cy + 8, cx - 35 + i * 5, cy + 3, C_MAG);
    spr.drawLine(cx + 28 + i * 5, cy + 8, cx + 31 + i * 5, cy + 3, C_MAG);
  }

  // smile (drawArc: 0 deg = 6 o'clock, clockwise)
  int span = grin ? 85 : 55;
  int r = grin ? 38 : 34;
  spr.drawArc(cx, cy, r, r - 5, 360 - span, 360, C_MAG, C_FACE, true);
  spr.drawArc(cx, cy, r, r - 5, 0, span, C_MAG, C_FACE, true);
}

void drawCountdown(unsigned long sinceTick, unsigned long now) {
  // HUD panel
  drawBrackets(170, 16, 146, 132, 12, C_CYAN);

  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(C_YEL);
  spr.drawString("T-MINUS", NUM_X, 20);

  // glitch jitter right after each tick
  int jx = 0, jy = 0;
  if (sinceTick < GLITCH_MS && count > 0) {
    jx = random(-6, 7);
    jy = random(-3, 4);
  }

  String s = String(count);
  spr.setTextFont(7);           // 7-segment font, digits only
  spr.setTextSize(2);
  spr.setTextDatum(MC_DATUM);

  // chromatic aberration: magenta left, cyan right, white core
  spr.setTextColor(C_MAG);   spr.drawString(s, NUM_X - 3 + jx, NUM_Y + jy);
  spr.setTextColor(C_CYAN);  spr.drawString(s, NUM_X + 3 + jx, NUM_Y + jy);
  spr.setTextColor(C_WHITE); spr.drawString(s, NUM_X + jx,     NUM_Y + jy);

  // segmented progress bar
  for (int i = 0; i < 10; i++) {
    int x = 178 + i * 13;
    uint16_t col = (i < count) ? C_CYAN : C_GRID;
    spr.fillRect(x, 136, 10, 6, col);
  }

  // finale text
  if (count == 0) {
    bool flash = (now / 250) % 2;
    spr.setTextFont(2);
    spr.setTextSize(1);
    spr.setTextDatum(TC_DATUM);
    spr.setTextColor(flash ? C_MAG : C_YEL);
    spr.drawString(">> SYSTEM ONLINE <<", W / 2 + 80, 152);
  }

  // random glitch bars across the screen
  if (sinceTick < GLITCH_MS && count > 0) {
    for (int i = 0; i < 4; i++) {
      int y = random(0, H);
      int x = random(0, W - 60);
      spr.fillRect(x, y, random(20, 120), random(1, 4), random(2) ? C_MAG : C_CYAN);
    }
  }
}

void drawScanlines() {
  for (int y = 0; y < H; y += 3) spr.drawFastHLine(0, y, W, C_SCAN);
}

// -----------------------------------------------------------------
void setup() {
  pinMode(PIN_POWER_ON, OUTPUT);
  digitalWrite(PIN_POWER_ON, HIGH);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);           // landscape 320x170
  tft.fillScreen(TFT_BLACK);

  pinMode(PIN_LCD_BL, OUTPUT);
  digitalWrite(PIN_LCD_BL, HIGH);

  C_BG    = tft.color565(6, 2, 18);      // deep violet-black
  C_GRID  = tft.color565(28, 10, 50);
  C_CYAN  = tft.color565(0, 240, 255);
  C_MAG   = tft.color565(255, 0, 170);
  C_YEL   = tft.color565(255, 230, 0);
  C_WHITE = TFT_WHITE;
  C_FACE  = tft.color565(14, 6, 36);
  C_SCAN  = tft.color565(3, 0, 10);

  spr.setColorDepth(16);
  spr.createSprite(W, H);       // full-screen buffer = flicker-free

  randomSeed(esp_random());
  lastTick = millis();
}

void loop() {
  unsigned long now = millis();

  // restart button
  if (digitalRead(PIN_BUTTON_2) == LOW) {
    count = 10;
    lastTick = now;
    delay(200);
  }

  // countdown tick
  if (count > 0 && now - lastTick >= TICK_MS) {
    count--;
    lastTick += TICK_MS;
  }

  unsigned long sinceTick = now - lastTick;

  // wink after every tick; at zero, wink on a slow loop
  bool wink = (count > 0) ? (sinceTick < WINK_MS) : ((now / 700) % 2 == 0);
  bool grin = (count == 0);

  drawBackground();
  drawFace(wink, grin);
  drawCountdown(sinceTick, now);
  drawScanlines();
  spr.pushSprite(0, 0);

  delay(16);
}
