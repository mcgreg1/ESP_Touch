#ifndef HELPER_H
#define HELPER_H

// Include necessary types for function signatures
#include <Arduino_GFX_Library.h>
#include <time.h> // For tm struct

// --- Function Declarations ---

// Initialization
void InstantiateGfxAndTouchObjects(); // Renamed for clarity
void InitDisplay();
void InitTouch();
bool InitWifiManager();
bool InitSD();
void InitAudio();
void initializeColors(); 

// NTP
bool syncNTPTime();
void resyncNTPTime();

// Display Drawing
void ShowStaticFields(const struct tm* currentTime);
void drawStationButton(int index, bool isActive); // Helper to draw one station button
void drawVolumeButtons();                          // Helper to draw volume buttons
void displayClock(const struct tm* currentTime, bool showFractals=false);
void UpdateTouchCoordsDisplay();
void displayMessageScreen(const char* line1, const char* line2 = nullptr, uint16_t color = WHITE);

// Drawing Utilities
void centerText(const char *text, int y, uint16_t color, const GFXfont *font, uint8_t size);
void drawButtonVisual(int x, int y, int w, int h, const char* label, uint16_t bgColor, uint16_t textColor, const GFXfont* font, uint8_t size);
// Input Handling
void handleTouchInput();

// Time & Math
void incrementLocalTime();
int calculateFutureTimeNum(int currentHour, int currentMin, int currentSec, int secondsToAdd); // Kept for now
void primeFactorsToString(int n, char* buffer, size_t bufferSize);

#endif // HELPER_H
