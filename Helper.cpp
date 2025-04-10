#include "CustomDef.h" // Include defines and extern globals
#include "Helper.h"    // Include function declarations

// --- Global Variable DEFINITIONS ---

// WiFi
const char *ssid = "Pandora2"; // Default, might be overwritten by WiFiManager
const char *password = "LetMeIn#123"; // Default

// NTP
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const long ntpSyncInterval = 15 * 60 * 1000;

// Timekeeping
const long interval = 1000;

// German Days/Months
const char *Wochentage[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const char *Monate[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};

// Other Globals
bool firstDisplayDone = false;
uint16_t RGB565_LIGHT_GREY = 0; // Initialized in InitDisplay
struct tm timeinfo;
unsigned long previousMillis = 0;
bool timeSynchronized = false;
unsigned long lastNtpSyncMillis = 0;
ClockState currentClockState = STATE_BOOTING;
// Touch Coordinate Display
unsigned long lastTouchMillis = 0;
const unsigned long touchDisplayTimeout = 3000;
bool touchCoordsVisible = false;
int lastDisplayedTouchX = 0;
int lastDisplayedTouchY = 0;
char previousTouchCoordsStr[20] = "";
uint16_t prev_touch_coords_x = 0;
uint16_t prev_touch_coords_y = 0;
uint16_t prev_touch_coords_w = 0;
uint16_t prev_touch_coords_h = 0;

// Font pointers
const GFXfont *font_freesansbold18 = &FreeSansBold18pt7b;
const GFXfont *font_freesans18 = &FreeSans18pt7b;
const GFXfont *font_freesans12 = &FreeSans12pt7b;

// Display/Touch Pointers (defined as nullptr initially)
Arduino_DataBus *bus = nullptr;
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *gfx = nullptr;
TAMC_GT911 *ts_ptr = nullptr;
int32_t w = 0; // Defined here, set in InitDisplay
int32_t h = 0; // Defined here, set in InitDisplay

// Custom Display Init Sequence
const uint8_t st7701_type5_init_operations[] = {
    // PASTE YOUR ACTUAL BYTE SEQUENCE HERE!
    // Example placeholder:
     0x01, 120, 0, // Software reset
     0x11, 120, 0, // Sleep out
     0x3a, 10, 1, 0x55, // Interface Pixel Format (16 bit)
     // ... potentially many more commands ...
     0x29, 120, 0 // Display on
};
const int st7701_type5_init_operations_len = sizeof(st7701_type5_init_operations);

// --- Function Implementations ---

void InstantiateGfxAndTouchObjects() {
    Serial.println("FUNCTION: InstantiateGfxAndTouchObjects");
    if (gfx || ts_ptr) { Serial.println("  WARNING: Objects already instantiated!"); return; }

    // Instantiate Display Objects
    bus = new Arduino_SWSPI( GFX_NOT_DEFINED, 39, 48, 47, GFX_NOT_DEFINED );
    rgbpanel = new Arduino_ESP32RGBPanel( 18, 17, 16, 21, 4, 5, 6, 7, 15, 8, 20, 3, 46, 9, 10, 11, 12, 13, 14, 0,
        1, 10, 8, 50, 1, 10, 8, 20, 0, 12000000, false, 0, 0, 0);
    gfx = new Arduino_RGB_Display( 480, 480, rgbpanel, 0, true, bus, GFX_NOT_DEFINED,
        st7701_type5_init_operations, st7701_type5_init_operations_len);

    // Instantiate Touch Object
    // Note: w and h are not set yet, InitTouch will use the defines/constructor defaults for max(w,h) etc.
    // If precise dimensions are needed *at instantiation*, move this to InitTouch after w/h are set.
    // However, the library likely uses them mainly for mapping later.
    ts_ptr = new TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, 480, 480); // Use known dims

    if (!bus || !rgbpanel || !gfx || !ts_ptr) {
         Serial.println("  ERROR: Failed to allocate memory for GFX/Touch objects!");
         while(1); // Halt
     }
     Serial.println("FUNCTION END: InstantiateGfxAndTouchObjects");
}

void InitDisplay() {
    Serial.println("FUNCTION: InitDisplay");
    if (!gfx) { Serial.println("  ERROR: gfx object not instantiated!"); return; }
    if (!gfx->begin()) { Serial.println("  ERROR: gfx->begin() failed!"); while (1); }
    w = gfx->width(); // Set global dimensions
    h = gfx->height();
    RGB565_LIGHT_GREY = gfx->color565(170, 170, 170); // Init color
    Serial.printf("  GFX Initialized. Resolution: %d x %d\n", w, h);
    gfx->fillScreen(BLACK);
    #ifdef GFX_BL
        pinMode(GFX_BL, OUTPUT); digitalWrite(GFX_BL, HIGH); Serial.println("  Backlight ON.");
    #endif
    Serial.println("FUNCTION END: InitDisplay");
}

void InitTouch() {
    Serial.println("FUNCTION: InitTouch");
    if (!ts_ptr) { Serial.println(" ERROR: ts_ptr object not instantiated!"); return; }
    Wire.begin(TOUCH_GT911_SDA, TOUCH_GT911_SCL); // Init I2C
    ts_ptr->begin(); // Init communication with chip
    // ts_ptr->setRotation(...); // Add if needed
    Serial.println("  GT911 Initialized.");
    Serial.println("FUNCTION END: InitTouch");
}

bool InitWifiManager() {
    Serial.println("FUNCTION: InitWifiManager");
    if (WiFi.status() == WL_CONNECTED) { Serial.println("  Already connected."); return true; }
    WiFiManager wifiManager;
    // wifiManager.resetSettings();
    wifiManager.setConnectTimeout(10);
    // Optionally set custom Portal name/password, IP, etc.
    // wifiManager.setAPStaticIPConfig(...);
    // wifiManager.setMinimumSignalQuality(30); // Example
    // wifiManager.setCustomHeadElement("<style>body{color: #eee; background: #333;}</style>"); // Example

    displayMessageScreen("Config WiFi", "Connect: FractalClockAP", WHITE);
    Serial.println("  Starting WiFiManager autoConnect...");
    bool success = wifiManager.autoConnect("FractalClockAP"); // Blocking call
    if (!success) {
        Serial.println("  WiFiManager Failed/Timeout.");
        displayMessageScreen("WiFi Failed", "", RED); delay(2000);
    } else {
        Serial.println("  WiFiManager Connected!");
        displayMessageScreen("WiFi Connected!", WiFi.localIP().toString().c_str(), GREEN); delay(1500);
    }
    Serial.println("FUNCTION END: InitWifiManager");
    return success;
}

bool syncNTPTime() {
    Serial.println("FUNCTION: syncNTPTime");
    displayMessageScreen("Syncing Time...", ntpServer, WHITE);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm checkTime;
    bool success = getLocalTime(&checkTime, 15000);
    if (!success) {
        Serial.println("  NTP Sync Failed!");
        displayMessageScreen("NTP Sync Failed!", "", RED); delay(2000);
        timeSynchronized = false;
    } else {
        Serial.println("  NTP Sync Successful!");
        timeSynchronized = true;
        timeinfo = checkTime;
        lastNtpSyncMillis = millis();
        displayMessageScreen("Time Synced!", "", GREEN); delay(1000);
        Serial.printf("  Current time: %s", asctime(&timeinfo));
    }
    Serial.println("FUNCTION END: syncNTPTime");
    return success;
}

void resyncNTPTime() {
    Serial.println("FUNCTION: resyncNTPTime");
    if (currentClockState != STATE_RUNNING) { Serial.println("  Skipped: Not RUNNING"); return; }
    if (WiFi.status() != WL_CONNECTED) { Serial.println("  Skipped: WiFi disconnected."); timeSynchronized = false; currentClockState = STATE_WIFI_NTP_FAILED; return; }
    Serial.print("  Attempting periodic NTP re-sync... ");
    struct tm tempTimeInfo;
    if (!getLocalTime(&tempTimeInfo, 5000)) {
        Serial.println("Failed.");
    } else {
         Serial.println("Success.");
         timeinfo = tempTimeInfo; timeSynchronized = true; lastNtpSyncMillis = millis();
         Serial.printf("  Time re-synchronized: %s", asctime(&timeinfo));
    }
     Serial.println("FUNCTION END: resyncNTPTime");
}

void ShowStaticFields(const struct tm* currentTime) {
    Serial.println("FUNCTION: ShowStaticFields");
     if (!gfx || currentTime == NULL) return;
    // Simple fixed layout for static fields
    const int y_weekday = 12;
    const int lineSpacing = 10;
    int16_t tx, ty; uint16_t tw, mainFontHeight;
    gfx->setFont(font_freesansbold18); gfx->setTextSize(1);
    gfx->getTextBounds("M", 0, 0, &tx, &ty, &tw, &mainFontHeight);
    const int y_date = y_weekday + mainFontHeight + lineSpacing;

    char buffer[40];
    snprintf(buffer, sizeof(buffer), "%s,", Wochentage[currentTime->tm_wday]);
    centerText(buffer, y_weekday, RGB565_LIGHT_GREY, font_freesansbold18, 1);
    snprintf(buffer, sizeof(buffer), "%d. %s %d", currentTime->tm_mday, Monate[currentTime->tm_mon], currentTime->tm_year + 1900);
    centerText(buffer, y_date, RGB565_LIGHT_GREY, font_freesansbold18, 1);
    Serial.println("FUNCTION END: ShowStaticFields");
}

void displayClock(const struct tm* currentTime) {
     if (!gfx || currentTime == NULL) return;
     if (currentClockState != STATE_RUNNING) return; // Only draw if running normally

    static char previousTimeStr[9] = "";
    static int16_t prev_time_y = -1;
    static uint16_t prev_time_w = 0, prev_time_h = 0;
    static char previousFactorStr[MAX_FACTOR_STR_LEN] = "";
    static uint16_t prev_factor_h = 0;

    const GFXfont *timeFont = font_freesansbold18;
    const GFXfont *factorFont = font_freesans18;

    // Simplified Layout (Assume static fields take up ~60 pixels height)
    const int static_area_height = 60;
    int16_t tx, ty; uint16_t tw, timeHeight, factorFontHeight;
    gfx->setFont(timeFont); gfx->setTextSize(2);
    gfx->getTextBounds("00:00:00", 0, 0, &tx, &ty, &tw, &timeHeight);
    gfx->setFont(factorFont); gfx->setTextSize(1);
    gfx->getTextBounds("0", 0, 0, &tx, &ty, &tw, &factorFontHeight);
    const int lineSpacing = 10;
    int y_time_top_target = static_area_height + lineSpacing; // Time below static area
    int y_factor_top_target = y_time_top_target + timeHeight + lineSpacing;

    char timeStr[9];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);
    int timeNum = currentTime->tm_hour * 10000 + currentTime->tm_min * 100 + currentTime->tm_sec;
    char factorBuffer[MAX_FACTOR_STR_LEN];
    primeFactorsToString(timeNum, factorBuffer, sizeof(factorBuffer));
    char combinedOutputStr[10 + MAX_FACTOR_STR_LEN];
    snprintf(combinedOutputStr, sizeof(combinedOutputStr), "%06d=%s", timeNum, factorBuffer);

    if (strcmp(timeStr, previousTimeStr) != 0) {
        if (prev_time_w > 0) { gfx->fillRect((w - prev_time_w) / 2, prev_time_y, prev_time_w, prev_time_h, BLACK); }
        centerText(timeStr, y_time_top_target, WHITE, timeFont, 2);
        gfx->setFont(timeFont); gfx->setTextSize(2);
        gfx->getTextBounds(timeStr, 0, 0, &tx, &ty, &prev_time_w, &prev_time_h);
        prev_time_y = y_time_top_target;
        strcpy(previousTimeStr, timeStr);
    }
    if (strcmp(combinedOutputStr, previousFactorStr) != 0) {
        int left_margin = 4;
        if (prev_factor_h > 0) { gfx->fillRect(0, y_factor_top_target, w, prev_factor_h, BLACK); }
        gfx->setFont(factorFont); gfx->setTextSize(1); gfx->setTextColor(WHITE);
        uint16_t h1; gfx->getTextBounds(combinedOutputStr, 0, 0, &tx, &ty, &tw, &h1);
        int cursor_y = y_factor_top_target - ty; gfx->setCursor(left_margin, cursor_y); gfx->print(combinedOutputStr);
        strcpy(previousFactorStr, combinedOutputStr); prev_factor_h = h1;
    }
}

void UpdateTouchCoordsDisplay() {
    if (!gfx) return;
    char currentCoordsStr[20];
    bool needsRedraw = false; bool clearNeeded = false;
    unsigned long currentMillis = millis();

    if (touchCoordsVisible) {
        if (currentMillis - lastTouchMillis > touchDisplayTimeout) { touchCoordsVisible = false; clearNeeded = true; currentCoordsStr[0]='\0'; }
        else { snprintf(currentCoordsStr, sizeof(currentCoordsStr), "T:%3d,%3d", lastDisplayedTouchX, lastDisplayedTouchY); if(strcmp(currentCoordsStr, previousTouchCoordsStr) != 0 || prev_touch_coords_w == 0) needsRedraw = true; }
    } else { currentCoordsStr[0]='\0'; if(prev_touch_coords_w > 0) clearNeeded = true; }

    if (needsRedraw || clearNeeded) {
        if (prev_touch_coords_w > 0) { gfx->fillRect(prev_touch_coords_x, prev_touch_coords_y, prev_touch_coords_w, prev_touch_coords_h, BLACK); prev_touch_coords_w = 0; }
        if (touchCoordsVisible) {
            gfx->setFont(NULL); gfx->setTextSize(1); gfx->setTextColor(YELLOW);
            int16_t x1, y1; uint16_t w1, h1; gfx->getTextBounds(currentCoordsStr, 0, 0, &x1, &y1, &w1, &h1);
            int cursorX = 0 - x1; int cursorY = 0 - y1; gfx->setCursor(cursorX, cursorY); gfx->print(currentCoordsStr);
            prev_touch_coords_x = cursorX + x1; prev_touch_coords_y = cursorY + y1; prev_touch_coords_w = w1; prev_touch_coords_h = h1;
        } else { prev_touch_coords_w = 0; prev_touch_coords_h = 0; }
        strcpy(previousTouchCoordsStr, currentCoordsStr);
    }
}

void displayMessageScreen(const char* line1, const char* line2, uint16_t color) {
    Serial.printf("FUNCTION: displayMessageScreen - L1: %s, L2: %s\n", line1, line2 ? line2 : "NULL");
    if (!gfx) return;
    gfx->fillScreen(BLACK); gfx->setFont(NULL); gfx->setTextSize(2); gfx->setTextColor(color);
    int16_t x1, y1; uint16_t w1, h1, w2 = 0, h2 = 0; int lineSpacing = 10;
    gfx->getTextBounds(line1, 0, 0, &x1, &y1, &w1, &h1);
    if (line2) gfx->getTextBounds(line2, 0, 0, &x1, &y1, &w2, &h2);
    int totalDrawHeight = h1 + (line2 ? (h2 + lineSpacing) : 0);
    int startY = (h - totalDrawHeight) / 2; if (startY < 0) startY = 0;
    centerText(line1, startY, color, NULL, 2);
    if (line2) centerText(line2, startY + h1 + lineSpacing, color, NULL, 2);
    Serial.println("FUNCTION END: displayMessageScreen");
}

void centerText(const char *text, int y, uint16_t color, const GFXfont *font, uint8_t size) {
    if (!gfx || !text || strlen(text) == 0) return;
    int16_t x1, y1; uint16_t text_w, text_h;
    gfx->setFont(font); gfx->setTextSize(size);
    gfx->getTextBounds(text, 0, 0, &x1, &y1, &text_w, &text_h);
    int cursor_x = (w - text_w) / 2 - x1; int cursor_y = y - y1;
    if (cursor_x < 0) cursor_x = 0;
    gfx->setCursor(cursor_x, cursor_y); gfx->setTextColor(color); gfx->print(text);
}

void handleTouchInput() {
     if (!ts_ptr) return;
     // Reads touch hardware state into ts_ptr->isTouched, ts_ptr->touches, ts_ptr->points[]
     // This is already done in the main loop to update the coordinate display vars.
     // This function would be expanded in the future if touch actions (buttons etc.) are needed.
}

void incrementLocalTime() {
     if (!timeSynchronized) return;
     time_t currentTimeSec = mktime(&timeinfo);
     currentTimeSec += 1;
     localtime_r(¤tTimeSec, &timeinfo);
}

int calculateFutureTimeNum(int h, int m, int s, int add) { return 0; /* Placeholder */ }

void primeFactorsToString(int n, char* buffer, size_t bufferSize) {
     if (!buffer || bufferSize == 0) return;
    buffer[0] = '\0'; char tempBuf[20];
    if (n <= 0) { strncpy(buffer, "0", bufferSize - 1); buffer[bufferSize-1]='\0'; return; }
    if (n == 1) { strncpy(buffer, "1", bufferSize - 1); buffer[bufferSize-1]='\0'; return; }
    size_t currentLen = 0; bool firstFactor = true;
    auto addFactor = [&](const char* factorStr) { /* ... lambda as before ... */ return true;}; // Needs full lambda
    while (n % 2 == 0) { if (!addFactor("2")) goto end_factorization; n /= 2; }
    for (int i = 3; i * i <= n; i += 2) { while (n % i == 0) { snprintf(tempBuf, sizeof(tempBuf), "%d", i); if (!addFactor(tempBuf)) goto end_factorization; n /= i; }}
    if (n > 1) { snprintf(tempBuf, sizeof(tempBuf), "%d", n); addFactor(tempBuf); }
end_factorization: buffer[bufferSize - 1] = '\0';
}
