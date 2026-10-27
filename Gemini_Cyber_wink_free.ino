#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// Cyberpunk Colors
#define CYAN_NEON    0x07FF
#define PINK_NEON    0xF81F
#define PURPLE_NEON  0x780F
#define DARK_BG      0x0000
#define YELLOW_FACE  0xFFE0

void drawCyberGrid()
{
    // Horizontal grid
    for (int y = 0; y < 170; y += 20)
    {
        tft.drawLine(0, y, 320, y, PURPLE_NEON);
    }

    // Vertical grid
    for (int x = 0; x < 320; x += 20)
    {
        tft.drawLine(x, 0, x, 170, PURPLE_NEON);
    }
}

void drawGlowCircle(int x, int y, int r, uint16_t color)
{
    for (int i = 8; i > 0; i--)
    {
        tft.drawCircle(x, y, r + i, color);
    }
}

void drawFace(bool wink)
{
    int centerX = 160;
    int centerY = 85;

    // Neon glow
    drawGlowCircle(centerX, centerY, 58, PINK_NEON);

    // Face
    tft.fillCircle(centerX, centerY, 55, YELLOW_FACE);

    // Left Eye
    if (wink)
    {
        tft.drawFastHLine(centerX - 25, centerY - 15, 20, DARK_BG);
        tft.drawFastHLine(centerX - 25, centerY - 14, 20, DARK_BG);
    }
    else
    {
        tft.fillCircle(centerX - 15, centerY - 15, 6, DARK_BG);
    }

    // Right Eye
    tft.fillCircle(centerX + 20, centerY - 15, 6, DARK_BG);

    // Smile
    tft.drawArc(centerX, centerY + 5,
                30, 25,
                20, 160,
                DARK_BG, YELLOW_FACE, true);

    // Neon cheeks
    tft.drawCircle(centerX - 30, centerY + 10, 6, CYAN_NEON);
    tft.drawCircle(centerX + 30, centerY + 10, 6, CYAN_NEON);
}

void drawCountdown(int count)
{
    String txt = String(count);

    // Clear area
    tft.fillRect(110, 125, 100, 40, DARK_BG);

    // Glow effect
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(PINK_NEON);
    tft.drawString(txt, 161, 141, 7);

    tft.setTextColor(CYAN_NEON);
    tft.drawString(txt, 160, 140, 7);
}

void setup()
{
    tft.init();
    tft.setRotation(1);   // Landscape

    tft.fillScreen(DARK_BG);

    drawCyberGrid();

    // Cyberpunk title
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(CYAN_NEON);
    tft.drawString("CYBER COUNTDOWN", 160, 15, 2);
}

void loop()
{
    for (int i = 10; i >= 0; i--)
    {
        tft.fillScreen(DARK_BG);

        drawCyberGrid();

        tft.setTextColor(CYAN_NEON);
        tft.drawString("CYBER COUNTDOWN", 160, 15, 2);

        bool winkState = (i % 2 == 0);

        drawFace(winkState);
        drawCountdown(i);

        delay(1000);
    }

    // Finished message
    tft.fillScreen(DARK_BG);
    drawCyberGrid();

    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(PINK_NEON);
    tft.drawString("MISSION", 160, 60, 4);

    tft.setTextColor(CYAN_NEON);
    tft.drawString("COMPLETE", 160, 105, 4);

    while (true)
    {
        delay(100);
    }
}