#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Cyberpunk Color Palette (RGB565)
#define COLOR_BG      0x0000 // Deep Black
#define COLOR_CYAN    0x07FF // Neon Cyan
#define COLOR_MAGENTA 0xF81F // Hot Pink / Magenta
#define COLOR_GREEN   0x07E0 // Acid Green
#define COLOR_YELLOW  0xFFE0 // Warning Yellow
#define COLOR_DARK_GY 0x18E3 // Dark Gray for HUD lines

// LilyGo T-Display-S3 display power pin
#ifndef TFT_POWER_ON
#define TFT_POWER_ON 15
#endif

void setup() {
  // Power on the display module
  pinMode(TFT_POWER_ON, OUTPUT);
  digitalWrite(TFT_POWER_ON, HIGH);

  tft.init();
  tft.setRotation(1); // Landscape orientation (320x170)
  tft.fillScreen(COLOR_BG);
  
  // Draw initial static HUD frame
  drawHUD();
}

void loop() {
  for (int i = 10; i >= 0; i--) {
    // Alternate wink state on each tick
    drawCyberFace(i % 2 == 0); 
    
    // Update countdown timer display
    drawCountdown(i);
    
    delay(1000);
  }
  
  // Critical flash sequence when reaching 0
  for(int f = 0; f < 3; f++) {
    tft.fillScreen(COLOR_MAGENTA);
    delay(120);
    tft.fillScreen(COLOR_BG);
    drawHUD();
    delay(120);
  }
}

void drawHUD() {
  // Outer cyberpunk frame lines
  tft.drawRect(5, 5, 310, 160, COLOR_CYAN);
  tft.drawRect(7, 7, 306, 156, COLOR_DARK_GY);
  
  // Corner tech accents (Magenta)
  tft.drawFastHLine(5, 5, 20, COLOR_MAGENTA);
  tft.drawFastVLine(5, 5, 20, COLOR_MAGENTA);
  tft.drawFastHLine(295, 5, 20, COLOR_MAGENTA);
  tft.drawFastVLine(314, 5, 20, COLOR_MAGENTA);
  
  tft.drawFastHLine(5, 164, 20, COLOR_MAGENTA);
  tft.drawFastVLine(5, 149, 20, COLOR_MAGENTA);
  tft.drawFastHLine(295, 164, 20, COLOR_MAGENTA);
  tft.drawFastVLine(314, 149, 20, COLOR_MAGENTA);
  
  // System Header Text
  tft.setTextColor(COLOR_CYAN, COLOR_BG);
  tft.setTextSize(1);
  tft.drawString("SYS.AI // INTERFACE_ONLINE", 15, 12);
  
  // Header divider
  tft.drawFastHLine(15, 25, 185, COLOR_DARK_GY);
}

void drawCyberFace(bool isWinking) {
  int cx = 100; // Center X of face region
  int cy = 95;  // Center Y of face region
  
  // Clear previous face rendering bounds
  tft.fillRect(40, 45, 120, 100, COLOR_BG);
  
  // Robotic head perimeter border
  tft.drawRect(cx - 50, cy - 45, 100, 90, COLOR_DARK_GY);
  tft.drawPixel(cx - 50, cy - 45, COLOR_CYAN);
  tft.drawPixel(cx + 49, cy - 45, COLOR_CYAN);
  tft.drawPixel(cx - 50, cy + 44, COLOR_CYAN);
  tft.drawPixel(cx + 49, cy + 44, COLOR_CYAN);

  // Left Eye (Always open glowing cyan module)
  tft.fillRect(cx - 30, cy - 20, 16, 16, COLOR_BG);
  tft.drawRect(cx - 30, cy - 20, 16, 16, COLOR_CYAN);
  tft.fillRect(cx - 25, cy - 15, 6, 6, COLOR_CYAN);

  // Right Eye (Toggles between open and winking dash)
  tft.fillRect(cx + 14, cy - 20, 16, 16, COLOR_BG);
  if (isWinking) {
    tft.drawFastHLine(cx + 14, cy - 12, 16, COLOR_MAGENTA);
    tft.drawFastHLine(cx + 16, cy - 10, 12, COLOR_CYAN);
  } else {
    tft.drawRect(cx + 14, cy - 20, 16, 16, COLOR_CYAN);
    tft.fillRect(cx + 19, cy - 15, 6, 6, COLOR_CYAN);
  }

  // Smirk / Grin (Cyberpunk segmented line with glitch tooth)
  tft.drawFastHLine(cx - 20, cy + 20, 40, COLOR_GREEN);
  tft.drawFastVLine(cx - 20, cy + 15, 5, COLOR_GREEN);
  tft.drawFastVLine(cx + 20, cy + 15, 5, COLOR_GREEN);
  tft.fillRect(cx + 5, cy + 17, 6, 3, COLOR_YELLOW);
}

void drawCountdown(int num) {
  // Clear countdown HUD area on the right
  tft.fillRect(210, 45, 95, 100, COLOR_BG);
  
  // HUD Subheader
  tft.setTextColor(COLOR_MAGENTA, COLOR_BG);
  tft.setTextSize(1);
  tft.drawString("COUNTDOWN", 212, 48);
  
  // Large Countdown Digits
  tft.setTextColor(COLOR_YELLOW, COLOR_BG);
  tft.setTextSize(4);
  
  if (num < 10) {
    tft.setCursor(240, 75);
  } else {
    tft.setCursor(215, 75);
  }
  tft.print(num);
  
  // Dynamic status feedback text
  tft.setTextSize(1);
  if (num > 3) {
    tft.setTextColor(COLOR_CYAN, COLOR_BG);
    tft.drawString("STATUS: NOMINAL", 212, 125);
  } else {
    tft.setTextColor(COLOR_MAGENTA, COLOR_BG);
    tft.drawString("STATUS: CRITICAL", 212, 125);
  }
}