#include "CustomDef.h" // Include defines and extern globals
#include "Helper.h"    // Include function declarations
#include <Preferences.h>

// --- Global Variable DEFINITIONS ---
Preferences preferences; 
// WiFi
const char *ssid = "Pandora2"; // Default, might be overwritten by WiFiManager
const char *password = "LetMeIn#123"; // Default

// NTP
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
const long ntpSyncInterval = 5 * 60 * 1000;

// Alarm State
struct tm alarmTime;       // Defined here
bool isAlarmSet = false;   // Defined here
bool alarmIsActive;
bool needsFullRedraw = false;
bool alarmJustTriggered = false;
const char *alarmSound = "/morning.mp3"; //"/buzzer.wav"

static int alarmColorIndex = 0;
static uint16_t alarmColors[] = {RED, GREEN, WHITE, BLACK}; // Using defines from CustomDef.h
static unsigned long alarmPatternMillis = 0; 
int currentVolume; // 0-21 (or your chosen range)

unsigned long alarmAreaTouchStartTime = 0;
bool alarmAreaTouchInProgress = false;

unsigned long lastWifiReconnectAttemptMillis = 0;
int wifiReconnectAttemptCount = 0; // Counter for backoff stages

// German Days/Months
const char *Wochentage[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const char *Monate[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};

const char *ClockStateNames[NUM_CLOCK_STATES] = {
    "Booting",            // STATE_BOOTING
    "Init WiFi",          // STATE_INIT_WIFI
    "WiFi Config Portal", // STATE_WIFI_CONFIG_PORTAL
    "Connecting WiFi",    // STATE_CONNECTING_WIFI
    "Waiting for NTP",    // STATE_WAITING_FOR_NTP
    "Running",            // STATE_RUNNING
    "WiFi Failed",     // STATE_WIFI_NTP_FAILED
    "NTP Failed",     // STATE_WIFI_NTP_FAILED
    "Setting Alarm",
    "Setting Clock",
    "Play Audio"
};



// Other Globals
bool firstDisplayDone = false;
// --- Colors ---
uint16_t RGB565_LIGHT_GREY;
uint16_t COLOR_BUTTON_BG_IDLE;
uint16_t COLOR_BUTTON_BG_ACTIVE;
uint16_t COLOR_BUTTON_TEXT;
uint16_t COLOR_DARK_GREY;
uint16_t COLOR_ALARM_TIME_ACTIVE;
uint16_t COLOR_ALARM_TIME_INACTIVE;

static bool lastTouchStatus=false;

struct tm timeinfo;
unsigned long previousMillis = 0;
bool timeSynchronized = false;
unsigned long lastNtpSyncMillis = 0;
ClockState currentClockState = STATE_BOOTING;
ClockState lastClockState = STATE_BOOTING;
ClockState savedClockState;
// Touch Coordinate Display
unsigned long lastTouchMillis = 0;
unsigned long elapsedTouchTime = 0;
const unsigned long touchDisplayTimeout = 1500;
bool touchCoordsVisible = true;
int lastDisplayedTouchX = 0;
int lastDisplayedTouchY = 0;
char previousTouchCoordsStr[20] = "";
uint16_t prev_touch_coords_x = 0;
uint16_t prev_touch_coords_y = 0;
uint16_t prev_touch_coords_w = 0;
uint16_t prev_touch_coords_h = 0;
bool wasTouchedPreviously = false;
int activeStationIndex = -1; // No station active initially

unsigned long lastButtonActionTime = 0; // Stores millis() when a button was last acted upon
const unsigned long buttonDebounceDelay = 250; // Cooldown in milliseconds (adjust as needed)

// Font pointers
const GFXfont *font_freesansbold18 = &FreeSansBold18pt7b;
const GFXfont *font_freesans18 = &FreeSans18pt7b;
const GFXfont *font_freesans12 = &FreeSans12pt7b;

// Display/Touch Pointers (defined as nullptr initially)
Arduino_DataBus *bus = nullptr;
Arduino_ESP32RGBPanel *rgbpanel = nullptr;
Arduino_RGB_Display *gfx = nullptr;
TAMC_GT911 *ts_ptr = nullptr;
Audio *audio_ptr = nullptr;
int32_t w = 0; // Defined here, set in InitDisplay
int32_t h = 0; // Defined here, set in InitDisplay

static const uint8_t custom_st7701_init_operations[] = {
    BEGIN_WRITE,
    WRITE_COMMAND_8, 0xFF,
    WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x13,
    WRITE_COMMAND_8, 0xEF,
    WRITE_BYTES, 1, 0x08,
    WRITE_COMMAND_8, 0xFF,
    WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x10,

    WRITE_C8_D16, 0xC0, 0x3B, 0x00,
    WRITE_C8_D16, 0xC1, 0x0D, 0x02, // VBP
    WRITE_C8_D16, 0xC2, 0x21, 0x08,
    WRITE_C8_D8, 0xCD, 0x00,

    WRITE_COMMAND_8, 0xB0, // Positive Voltage Gamma Control
    WRITE_BYTES, 16,
    0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18,

    WRITE_COMMAND_8, 0xB1, // Negative Voltage Gamma Control
    WRITE_BYTES, 16,
    0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18,

    WRITE_COMMAND_8, 0xFF,
    WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x11,

    WRITE_C8_D8, 0xB0, 0x60, // 5d
    WRITE_C8_D8, 0xB1, 0x30, // VCOM amplitude setting
    WRITE_C8_D8, 0xB2, 0x87, // VGH Voltage setting, 12V
    WRITE_C8_D8, 0xB3, 0x80,

    WRITE_C8_D8, 0xB5, 0x49, // VGL Voltage setting, -8.3V

    WRITE_C8_D8, 0xB7, 0x85,
    WRITE_C8_D8, 0xB8, 0x21,

    WRITE_C8_D8, 0xC1, 0x78,
    WRITE_C8_D8, 0xC2, 0x78,

//    WRITE_C8_D8, 0xD0, 0x88,

    WRITE_COMMAND_8, 0xE0,
    WRITE_BYTES, 3, 0x00, 0x1B, 0x02,

    WRITE_COMMAND_8, 0xE1,
    WRITE_BYTES, 11,
    0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44,
    
    WRITE_COMMAND_8, 0xE2,
    WRITE_BYTES, 13,
    0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00, 0x00,

    WRITE_COMMAND_8, 0xE3,
    WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x11,

    WRITE_C8_D16, 0xE4, 0x44, 0x44,

    WRITE_COMMAND_8, 0xE5,
    WRITE_BYTES, 16,
    0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0,

    WRITE_COMMAND_8, 0xE6,
    WRITE_BYTES, 4, 0x00, 0x00, 0x11, 0x11,

    WRITE_C8_D16, 0xE7, 0x44, 0x44,

    WRITE_COMMAND_8, 0xE8,
    WRITE_BYTES, 16,
    0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0,

    WRITE_COMMAND_8, 0xEB,
    WRITE_BYTES, 7,
    0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40,

    WRITE_COMMAND_8, 0xEC,
    WRITE_BYTES, 2,
    0x3C, 0x00,

    WRITE_COMMAND_8, 0xED,
    WRITE_BYTES, 16,
    0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA,

//    WRITE_COMMAND_8, 0xEF,
//    WRITE_BYTES, 6,
//    0x10, 0x0D, 0x04, 0x08,
//    0x3F, 0x1F,

    WRITE_COMMAND_8, 0xFF,
    WRITE_BYTES, 5, 0x77, 0x01, 0x00, 0x00, 0x00,

    WRITE_COMMAND_8, 0x3A,
    WRITE_BYTES, 1,
    0x66,

    WRITE_COMMAND_8, 0x36,
    WRITE_BYTES, 1,
    0x08,

    WRITE_COMMAND_8, 0x11, // Sleep Out
    END_WRITE,

    DELAY, 120,

    BEGIN_WRITE,
    WRITE_COMMAND_8, 0x29, // Display On
    END_WRITE,

    DELAY, 50};
const int custom_st7701_init_operations_len = sizeof(custom_st7701_init_operations);

// --- Function Implementations ---

void InstantiateGfxAndTouchObjects() {
    Serial.println("FUNCTION: InstantiateGfxAndTouchObjects");
    if (gfx || ts_ptr) { 
        Serial.println("  WARNING: Objects already instantiated!"); 
        return; 
    }

    // Instantiate Display Objects
    bus = new Arduino_SWSPI( GFX_NOT_DEFINED, 39, 48, 47, GFX_NOT_DEFINED );
    rgbpanel = new Arduino_ESP32RGBPanel( 18, 17, 16, 21, 4, 5, 6, 7, 15, 8, 20, 3, 46, 9, 10, 11, 12, 13, 14, 0,
        1, 10, 8, 50, 1, 10, 8, 20, 0, /*12000000*/6000000, false, 0, 0, 0);
    gfx = new Arduino_RGB_Display( 480, 480, rgbpanel, 0, true, bus, GFX_NOT_DEFINED,
        custom_st7701_init_operations, custom_st7701_init_operations_len);

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
    if (!gfx) { 
        Serial.println("  ERROR: gfx object not instantiated!"); 
        return; 
    }
    if (!gfx->begin()) { 
        Serial.println("  ERROR: gfx->begin() failed!"); 
        while (1); 
    }
    w = gfx->width(); // Set global dimensions
    h = gfx->height();
    RGB565_LIGHT_GREY = gfx->color565(170, 170, 170); // Init color
    Serial.printf("  GFX Initialized. Resolution: %d x %d\n", w, h);
    gfx->fillScreen(BLACK);
    #ifdef GFX_BL
        pinMode(GFX_BL, OUTPUT); digitalWrite(GFX_BL, HIGH); Serial.println("  Backlight ON.");
    #endif
    Serial.println("FUNCTION END: InitDisplay");
    centerText("Screen initialized", 100, RGB565_LIGHT_GREY, font_freesansbold18, 1);
}

void InitTouch() {
    Serial.println("FUNCTION: InitTouch");
    if (!ts_ptr) { 
        Serial.println(" ERROR: ts_ptr object not instantiated!"); 
        return; 
    }
    Wire.begin(TOUCH_GT911_SDA, TOUCH_GT911_SCL); // Init I2C
    ts_ptr->begin(); // Init communication with chip
    // ts_ptr->setRotation(...); // Add if needed
    Serial.println("  GT911 Initialized.");
    Serial.println("FUNCTION END: InitTouch");
    centerText("Touch initialized", 150, RGB565_LIGHT_GREY, font_freesansbold18, 1);
}

bool InitSD() {
      // SD card init
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }
  Serial.println("SD Card Mount successful!");
  return true;
}

void InitAudio() {
    if (!audio_ptr)
    {
        audio_ptr = new Audio();
        Serial.println("  Setting audio task core to 0...");
        audio_ptr->setAudioTaskCore(0); 
        audio_ptr->setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
        audio_ptr->setVolume(currentVolume);
        Serial.println("Audio Initialized!");
    }

}

void initializeColors() {
    RGB565_LIGHT_GREY = gfx->color565(170, 170, 170);
    //COLOR_OK_BUTTON_GREEN = gfx->color565(0, 180, 0);
    COLOR_ALARM_TIME_ACTIVE = WHITE;
    COLOR_ALARM_TIME_INACTIVE = gfx->color565(80, 80, 80); // <<< Example: Dark Grey
    COLOR_BUTTON_BG_IDLE = gfx->color565(60, 60, 60);    // Even darker?
    COLOR_BUTTON_BG_ACTIVE = gfx->color565(0, 180, 0);
    COLOR_BUTTON_TEXT = WHITE;
    COLOR_DARK_GREY = gfx->color565(80, 80, 80); // <<< Initialize the specific dark grey
}

bool InitWifiManager() {
    Serial.println("FUNCTION: InitWifiManager");
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("  Already connected to WiFi.");
        return true;
    }
    WiFiManager wifiManager;
    // wifiManager.resetSettings(); // Uncomment to force portal for testing

    // Set timeout for trying saved credentials (remains short)
    wifiManager.setConnectTimeout(10); // 10 seconds
    wifiManager.setConfigPortalTimeout(120);

    displayMessageScreen("Config WiFi", "Connect: FractalClockAP", WHITE);
    Serial.println("  Starting WiFiManager autoConnect (with portal timeout)...");

    // yield(); // Yield just before the blocking call (minor effect maybe)
    bool success = wifiManager.autoConnect("FractalClockAP");
    // yield(); // Yield just after the blocking call (minor effect maybe)


    if (!success) {
        Serial.println("  WiFiManager Failed/Timeout.");
        // Portal might have timed out, or connection failed after portal.
        displayMessageScreen("WiFi Failed", "Check Network or Portal", RED);
        delay(2000);
    } else {
        Serial.println("  WiFiManager Connected!");
        displayMessageScreen("WiFi Connected!", WiFi.localIP().toString().c_str(), GREEN);
        delay(1500);
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



// In Helper.cpp

#include "CustomDef.h"
#include "Helper.h"
#include <math.h> // Include for fabs()

void resyncNTPTime() {
    Serial.println("FUNCTION: resyncNTPTime");
    if (currentClockState != STATE_RUNNING) {
      Serial.println("  Skipped: Not RUNNING");
      return;
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("  Skipped: WiFi disconnected.");
      timeSynchronized = false; // Mark as unsynced if WiFi lost
      currentClockState = STATE_WIFI_FAILED;
      displayAdditionalInfo("WiFi Lost - Check Network", RED);
      //displayMessageScreen("WiFi Lost", "Check Network", RED); // Show message
      return;
    }

    Serial.print("  Attempting periodic NTP re-sync... ");

    // --- Capture current local time BEFORE syncing ---
    struct tm timeBeforeSync = timeinfo; // Make a copy of the current time struct
    time_t secondsBeforeSync = mktime(&timeBeforeSync); // Convert to seconds since epoch

    // --- Attempt to get accurate time from NTP ---
    struct tm tempTimeInfo; // To store the result from NTP
    if (!getLocalTime(&tempTimeInfo, 5000)) { // 5 second timeout
        Serial.println("Failed.");
        // Time remains unchanged, flag might be set to false by WiFi check later if needed
    } else {
        Serial.println("Success.");

        // --- Calculate Time Difference ---
        time_t secondsAfterSync = mktime(&tempTimeInfo); // Convert newly fetched time to seconds
        double timeDifference = difftime(secondsAfterSync, secondsBeforeSync); // Calculate difference in seconds

        // --- Update global timeinfo ---
        timeinfo = tempTimeInfo; // Update main time struct with accurate time
        timeSynchronized = true; // Ensure flag is true
        lastNtpSyncMillis = millis(); // Reset timer ONLY on success

        // --- Print results ---
        Serial.printf("  Time re-synchronized. Drift: %.2f seconds.\n", timeDifference);
        Serial.printf("  New time: %s", asctime(&timeinfo)); // asctime adds a newline

        // Optional: Only redraw static fields if the date actually changed significantly
        // Usually the time difference check is enough indication if things drifted
        // needsStaticRedraw = true;
    }
    Serial.println("FUNCTION END: resyncNTPTime");
}

// Draws static elements (Date, Weekday), initial alarm time display, AND initial button states
void ShowStaticFields(const struct tm* currentTime) {
    Serial.println("FUNCTION: ShowStaticFields");
    if (!gfx || currentTime == NULL) return;

    // --- Fonts Used ---
    const GFXfont *staticFieldFont = font_freesansbold18; // For Date/Weekday
    const GFXfont *inactiveAlarmFont = font_freesans18; // For Date/Weekday
    const GFXfont *alarmFont = font_freesans12;       // Font for Alarm Label and Time

  if (currentClockState == STATE_RUNNING)
  { 
    char buffer[40];
    snprintf(buffer, sizeof(buffer), "%s,", Wochentage[currentTime->tm_wday]);
    centerText(buffer, WEEKDAY_Y, RGB565_LIGHT_GREY, staticFieldFont, 1);

    snprintf(buffer, sizeof(buffer), "%d. %s %d", currentTime->tm_mday, Monate[currentTime->tm_mon], currentTime->tm_year + 1900);
    centerText(buffer, DATE_Y, RGB565_LIGHT_GREY, staticFieldFont, 1);
  }
    gfx->setFont(alarmFont); 
    gfx->setTextSize(1); 
    gfx->setCursor(ALARM_RECT_X+22, ALARM_RECT_Y-6);
    gfx->print(ALARM_LABEL);
    char alarmStrTime[6];
    snprintf(alarmStrTime, sizeof(alarmStrTime), "%02d:%02d", alarmTime.tm_hour, alarmTime.tm_min);
    if (isAlarmSet)
    {
      drawButtonVisual(ALARM_RECT_X, ALARM_RECT_Y, ALARM_RECT_W, ALARM_RECT_H, alarmStrTime, RED, WHITE, staticFieldFont, 1);
      //gfx->drawRect(ALARM_RECT_X, ALARM_RECT_Y, ALARM_RECT_W, ALARM_RECT_H, COLOR_ALARM_TIME_ACTIVE);
      //gfx->fillRect(ALARM_RECT_X+2, ALARM_RECT_Y+2, ALARM_RECT_W-4, ALARM_RECT_H-4, RED)
      //gfx->setTextColor(COLOR_ALARM_TIME_ACTIVE);
    }
    else
    {
      drawButtonVisual(ALARM_RECT_X, ALARM_RECT_Y, ALARM_RECT_W, ALARM_RECT_H, alarmStrTime, COLOR_BUTTON_BG_IDLE, WHITE, inactiveAlarmFont, 1);
      /*
      gfx->drawRect(ALARM_RECT_X, ALARM_RECT_Y, ALARM_RECT_W, ALARM_RECT_H, COLOR_ALARM_TIME_INACTIVE);
      gfx->setTextColor(COLOR_ALARM_TIME_INACTIVE);
      gfx->setCursor(ALARM_RECT_X+10, ALARM_RECT_Y+ALARM_RECT_H-5);
      gfx->print(alarmStrTime);
      gfx->setCursor(ALARM_RECT_X+3, ALARM_RECT_Y+ALARM_RECT_H/2-1);
      gfx->print(ALARM_LABEL);
      */
    }

  if (currentClockState == STATE_RUNNING)
  {

    // --- Draw Buttons (Bottom Part)
    Serial.println("  Drawing initial button states...");
    for (int i = 0; i < NUM_STATION_BUTTONS; ++i) {
        // Pass the current active state from the global variable
        drawStationButton(i, (i == activeStationIndex));
    }
    drawVolumeButtons(); // Draw volume buttons
  }

    Serial.println("FUNCTION END: ShowStaticFields");
}


void handleTouchInput() {
    if (!ts_ptr) return;

    unsigned long currentMillis = millis(); // Get time once
    if (currentMillis - lastButtonActionTime < buttonDebounceDelay) {
        // Still in cooldown from the last button press, ignore new touches
        return;
    }
    ts_ptr->read();
    bool currentTouchStatus = ts_ptr->isTouched; // <<< LOCAL variable, read fresh

    if (currentTouchStatus==true && lastTouchStatus == false) {
      lastButtonActionTime = currentMillis;
      Serial.println("Touch TOUCHED");
      if (isAlarmSet && alarmIsActive)
      {
        alarmIsActive=false;
        audio_ptr->stopSong();
        needsFullRedraw=true;
      }
          bool actionProcessed = false;
        // Get INVERTED coordinates
        int touchX = 480 - ts_ptr->points[0].x;
        int touchY = 480 - ts_ptr->points[0].y;

        // --- Update Global Touch Coordinate Display State ---
        // (This part can still update even if buttons are debounced)
        lastDisplayedTouchX = touchX;
        lastDisplayedTouchY = touchY;
        lastTouchMillis = currentMillis;
        if (!touchCoordsVisible) {
            touchCoordsVisible = true;
            previousTouchCoordsStr[0] = ' '; previousTouchCoordsStr[1] = '\0';
        }


        if (currentClockState == STATE_RUNNING) 
        {
          //check if radio button was touched
          int radioIndex=0;
          if (touchX >= STAT_BUTTON_0_X && touchX < (STAT_BUTTON_0_X + STAT_BUTTON_WIDTH) && 
          touchY >= STAT_BUTTON_0_Y && touchY < (STAT_BUTTON_0_Y + STAT_BUTTON_HEIGHT))
            radioIndex=0;

          else if (touchX >= STAT_BUTTON_1_X && touchX < (STAT_BUTTON_1_X + STAT_BUTTON_WIDTH) && 
          touchY >= STAT_BUTTON_1_Y && touchY < (STAT_BUTTON_1_Y + STAT_BUTTON_HEIGHT))
            radioIndex=1;

          else if (touchX >= STAT_BUTTON_2_X && touchX < (STAT_BUTTON_2_X + STAT_BUTTON_WIDTH) && 
          touchY >= STAT_BUTTON_2_Y && touchY < (STAT_BUTTON_2_Y + STAT_BUTTON_HEIGHT))
            radioIndex=2;

          else if (touchX >= STAT_BUTTON_3_X && touchX < (STAT_BUTTON_3_X + STAT_BUTTON_WIDTH) && 
          touchY >= STAT_BUTTON_3_Y && touchY < (STAT_BUTTON_3_Y + STAT_BUTTON_HEIGHT))
            radioIndex=3;

          else 
          {
            radioIndex=-1;
            
          }


            if (radioIndex!=-1)
            {
                Serial.printf("Station Button %d Action Triggered.\n", radioIndex);

                if (activeStationIndex == radioIndex) { /* ... Deactivate ... */ 
                    Serial.println("  Deactivating station.");
                    activeStationIndex = -1;
                    audio_ptr->stopSong();
                    drawStationButton(radioIndex, false); // Redraw as inactive
                    displayAdditionalInfo(NULL, WHITE);
                    drawVolumeBar();
                }
                else { /* ... Activate ... */ 
                    Serial.printf("  Activating station: %s\n", station_labels[radioIndex]);
                    int previouslyActive = activeStationIndex;
                    if (previouslyActive != -1) {
                        drawStationButton(previouslyActive, false); // Deactivate old one visually
                    }
                    activeStationIndex = radioIndex;
                    drawStationButton(radioIndex, true); // Redraw new button as active
                    drawVolumeBar();

                    Serial.printf("  Connecting to host: %s\n", station_urls[radioIndex]);
                    if (!audio_ptr->connecttohost(station_urls[radioIndex])) {
                            Serial.println("  ERROR: connecttohost failed!");
                            drawStationButton(radioIndex, false); // Revert visual
                            activeStationIndex = previouslyActive;
                            if(activeStationIndex != -1) { drawStationButton(activeStationIndex, true); }
                            // displayMessageScreen("Audio Error", "Connection Failed", RED); delay(2000);
                    } 
                    else {
                        Serial.println("  connecttohost potentially successful.");
                    }
                }
            }
        }//state is running
        if (currentClockState == STATE_RUNNING || currentClockState == STATE_SETTING_ALARM)
        { 
            // Volume Down
            if (touchX >= VOL_BUTTON_DOWN_X && touchX < (VOL_BUTTON_DOWN_X + VOL_BUTTON_WIDTH) &&
                touchY >= VOL_BUTTON_DOWN_Y && touchY < (VOL_BUTTON_DOWN_Y + VOL_BUTTON_HEIGHT))
            {
                Serial.println("Volume Down Action.");
                currentVolume = audio_ptr->getVolume();
                if (currentVolume > 0) 
                currentVolume--;
                audio_ptr->setVolume(currentVolume);
                saveSettingsToFlash();
                Serial.printf("  New Volume: %d\n", audio_ptr->getVolume());
                drawVolumeBar();
                if (currentClockState == STATE_SETTING_ALARM && audio_ptr && !audio_ptr->isRunning())
                    audio_ptr->connecttoFS(SD, alarmSound);
            }
            // Volume Up
            else if (touchX >= VOL_BUTTON_UP_X && touchX < (VOL_BUTTON_UP_X + VOL_BUTTON_WIDTH) &&
                    touchY >= VOL_BUTTON_UP_Y && touchY < (VOL_BUTTON_UP_Y + VOL_BUTTON_HEIGHT))
            {
                Serial.println("Volume Up Action.");
                currentVolume = audio_ptr->getVolume();
                if (currentVolume < MAX_VOLUME) 
                    currentVolume++;
                audio_ptr->setVolume(currentVolume);
                saveSettingsToFlash();
                Serial.printf("  New Volume: %d\n", audio_ptr->getVolume());
                drawVolumeBar();
                if (currentClockState == STATE_SETTING_ALARM && audio_ptr && !audio_ptr->isRunning())
                    audio_ptr->connecttoFS(SD, alarmSound);
            }
        }
    

        // --- Check Alarm Area Touch ---

        if (touchX >= ALARM_RECT_X && touchX < (ALARM_RECT_X + ALARM_RECT_W) &&
            touchY >= ALARM_RECT_Y && touchY < (ALARM_RECT_Y + ALARM_RECT_H))
        {
            handleAlarmAreaTouch(); // Call the dedicated handler
        }

        else if (currentClockState == STATE_SETTING_ALARM) {
            Serial.println("Checking touches in SETTING_ALARM state");
               handleSetAlarmClock(touchX,touchY);
            } // End state checks
        else if (currentClockState == STATE_SETTING_CLOCK) {
            Serial.println("Checking touches in SETTING_CLOCK state");
               handleSetClock(touchX,touchY);
            } // End state checks

    } // End if (isTouchedNow)
    lastTouchStatus = currentTouchStatus;

}

void displayClock(const struct tm* currentTime, bool showFractals) {
     if (!gfx || currentTime == NULL) return;
     //if (currentClockState != STATE_RUNNING) return; // Only draw if running normally

    static char previousTimeStr[9] = "";
    static int16_t prev_time_y = -1;
    static uint16_t prev_time_w = 0, prev_time_h = 0;
    static char previousFactorStr[MAX_FACTOR_STR_LEN] = "";
    static uint16_t prev_factor_h = 0;

    const GFXfont *timeFont = font_freesansbold18;
    const GFXfont *factorFont = font_freesans18;

    char timeStr[9];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", currentTime->tm_hour, currentTime->tm_min, currentTime->tm_sec);


    if (strcmp(timeStr, previousTimeStr) != 0) {
        gfx->fillRect(CLOCK_RECT_X, CLOCK_RECT_Y, CLOCK_RECT_W, CLOCK_RECT_H, BLACK); 
        strcpy(previousTimeStr, timeStr);
        gfx->setCursor(CLOCK_X, CLOCK_Y);
        gfx->setTextColor(WHITE);

        gfx->setFont(timeFont); 
        gfx->setTextSize(2);
        gfx->print(timeStr);
        
        //show uptime
        /*
        gfx->setTextSize(1);
        gfx->setCursor(CLOCK_X, CLOCK_Y+40);
        char uptimeStr[16];
        snprintf(uptimeStr, sizeof(uptimeStr), "Uptime %d min.", millis()/1000/60);
        gfx->print(uptimeStr);*/
        if (showFractals)
        {
            int timeNum = currentTime->tm_hour * 10000 + currentTime->tm_min * 100 + currentTime->tm_sec;
            char factorBuffer[MAX_FACTOR_STR_LEN];
            primeFactorsToString(timeNum, factorBuffer, sizeof(factorBuffer));
            char combinedOutputStr[10 + MAX_FACTOR_STR_LEN];
            snprintf(combinedOutputStr, sizeof(combinedOutputStr), "%06d=%s", timeNum, factorBuffer);
            gfx->fillRect(FRACTAL_X, FRACTAL_Y, 450, 28, BLACK); 
            gfx->setFont(factorFont); 
            gfx->setTextSize(1); 
            gfx->setTextColor(WHITE);
            uint16_t h1; 
            gfx->setCursor(FRACTAL_X, FRACTAL_Y); 
            gfx->print(combinedOutputStr);

        }
    }

}

void displaySetAlarmScreen(const struct tm* currentAlarmTime) {
    if (!gfx) return;
    gfx->fillScreen(BLACK);
    Serial.println("FUNCTION: displaySetAlarmScreen");
    const GFXfont *mainFont = font_freesansbold18; // Large time display
    const GFXfont *promptFont = font_freesans18;   // Instructions
    const GFXfont *buttonFont = font_freesansbold18;// OK button text
    char alarmSet[6]; 
    snprintf(alarmSet, sizeof(alarmSet), "%02d:%02d", currentAlarmTime->tm_hour, currentAlarmTime->tm_min);
    centerText(alarmSet, ALARM_SET_BOX_Y+5, WHITE, mainFont, 2);//in the box (5px space)
    centerText("Alarmzeit einstellen", ALARM_SET_BOX_Y+ALARM_SET_BOX_H+3, RGB565_LIGHT_GREY, promptFont, 1);//3 px below the box

    //drawButtonVisual(OK_BUTTON_X, y_ok_button_top, OK_BUTTON_W, OK_BUTTON_H, "OK", GREEN, BLACK, buttonFont, 1);
    drawButtonVisual(OK_BUTTON_X, OK_BUTTON_Y, OK_BUTTON_W, OK_BUTTON_H, "OK", GREEN, BLACK, buttonFont, 1);
    if (isAlarmSet)
      drawButtonVisual(ACTIVATE_ALARM_BUTTON_X, ACTIVATE_ALARM_BUTTON_Y, ACTIVATE_ALARM_BUTTON_W, ACTIVATE_ALARM_BUTTON_H, "AKTIV", RED, BLACK, buttonFont, 1);
    else
      drawButtonVisual(ACTIVATE_ALARM_BUTTON_X, ACTIVATE_ALARM_BUTTON_Y, ACTIVATE_ALARM_BUTTON_W, ACTIVATE_ALARM_BUTTON_H, "INAKTIV", DARKGREY, BLACK, buttonFont, 1);

    // Hour Zone Indicator
    gfx->drawRect(ALARM_SET_H_X, ALARM_SET_BOX_Y, ALARM_SET_BOX_W, ALARM_SET_BOX_H, DARKGREY);//the y is the top of the rect
    centerText("HH+", ALARM_SET_BOX_Y, DARKGREY, NULL, 1); // Center label vertically

    // Minute Zone Indicator
    gfx->drawRect(ALARM_SET_M_X, ALARM_SET_BOX_Y, ALARM_SET_BOX_W, ALARM_SET_BOX_H, DARKGREY);
    centerText("MM+", ALARM_SET_BOX_Y, DARKGREY, NULL, 1); // Center label vertically

    drawVolumeButtons();

    Serial.println("FUNCTION END: displaySetAlarmScreen");
}

void displaySetClockScreen(const struct tm* currentTime) {
    if (!gfx) return;
    gfx->fillScreen(BLACK);
    Serial.println("FUNCTION: displaySetClockScreen");
    const GFXfont *mainFont = font_freesansbold18; // Large time display
    const GFXfont *promptFont = font_freesans18;   // Instructions
    const GFXfont *buttonFont = font_freesansbold18;// OK button text
    char alarmSet[6]; 
    snprintf(alarmSet, sizeof(alarmSet), "%02d:%02d", currentTime->tm_hour, currentTime->tm_min);
    centerText(alarmSet, ALARM_SET_BOX_Y+5, WHITE, mainFont, 2);//in the box (5px space)
    centerText("Uhrzeit einstellen", ALARM_SET_BOX_Y+ALARM_SET_BOX_H+3, RGB565_LIGHT_GREY, promptFont, 1);//3 px below the box

    drawButtonVisual(OK_BUTTON_X, OK_BUTTON_Y, OK_BUTTON_W, OK_BUTTON_H, "OK", GREEN, BLACK, buttonFont, 1);

    // Hour Zone Indicator
    gfx->drawRect(ALARM_SET_H_X, ALARM_SET_BOX_Y, ALARM_SET_BOX_W, ALARM_SET_BOX_H, DARKGREY);//the y is the top of the rect
    centerText("HH+", ALARM_SET_BOX_Y, DARKGREY, NULL, 1); // Center label vertically

    // Minute Zone Indicator
    gfx->drawRect(ALARM_SET_M_X, ALARM_SET_BOX_Y, ALARM_SET_BOX_W, ALARM_SET_BOX_H, DARKGREY);
    centerText("MM+", ALARM_SET_BOX_Y, DARKGREY, NULL, 1); // Center label vertically


    Serial.println("FUNCTION END: displaySetClockScreen");
}
void handleSetClock(int touchX, int touchY)
{
// --- Check Touch Zones using DEFINED COORDINATES ---
  const GFXfont *buttonFont = font_freesansbold18;// OK button text
  if (touchX >= ALARM_SET_H_X && touchX < ALARM_SET_H_X + ALARM_SET_BOX_W &&
      touchY >= ALARM_SET_BOX_Y && touchY < ALARM_SET_BOX_Y + ALARM_SET_BOX_H) {
      Serial.println("Hour Zone Touched");
      timeinfo.tm_hour = (timeinfo.tm_hour + 1) % 24;
      displaySetClockScreen(&timeinfo); // Redraw screen
  }
  // Minute Zone Check
  else if (touchX >= ALARM_SET_M_X && touchX < ALARM_SET_M_X + ALARM_SET_BOX_W&&
          touchY >= ALARM_SET_BOX_Y && touchY < ALARM_SET_BOX_Y + ALARM_SET_BOX_H) {
      Serial.println("Minute Zone Touched");
      timeinfo.tm_min = (timeinfo.tm_min + 1) % 60;
      displaySetClockScreen(&timeinfo); // Redraw screen
  }
  // OK Button Check
  else if (touchX >= OK_BUTTON_X && touchX < (OK_BUTTON_X + OK_BUTTON_W) &&
          touchY >= OK_BUTTON_Y && touchY < (OK_BUTTON_Y + OK_BUTTON_H)) {
      Serial.println("OK Button Touched");
      currentClockState = STATE_WIFI_FAILED;
      needsFullRedraw = true; // <<< SET FLAG FOR MAIN LOOP
  }
}

void handleSetAlarmClock(int touchX, int touchY)
{
// --- Check Touch Zones using DEFINED COORDINATES ---
  const GFXfont *buttonFont = font_freesansbold18;// OK button text
  if (touchX >= ALARM_SET_H_X && touchX < ALARM_SET_H_X + ALARM_SET_BOX_W &&
      touchY >= ALARM_SET_BOX_Y && touchY < ALARM_SET_BOX_Y + ALARM_SET_BOX_H) {
      Serial.println("Alarm Hour Zone Touched");
      alarmTime.tm_hour = (alarmTime.tm_hour + 1) % 24;
      displaySetAlarmScreen(&alarmTime); // Redraw screen
  }
  // Minute Zone Check
  else if (touchX >= ALARM_SET_M_X && touchX < ALARM_SET_M_X + ALARM_SET_BOX_W&&
          touchY >= ALARM_SET_BOX_Y && touchY < ALARM_SET_BOX_Y + ALARM_SET_BOX_H) {
      Serial.println("Alarm Minute Zone Touched");
      alarmTime.tm_min = (alarmTime.tm_min + 1) % 60;
      displaySetAlarmScreen(&alarmTime); // Redraw screen
  }
  // OK Button Check
  else if (touchX >= OK_BUTTON_X && touchX < (OK_BUTTON_X + OK_BUTTON_W) &&
          touchY >= OK_BUTTON_Y && touchY < (OK_BUTTON_Y + OK_BUTTON_H)) {
      Serial.println("Alarm OK Button Touched");
      alarmJustTriggered = false;
      currentClockState = savedClockState;
      audio_ptr->stopSong();
      needsFullRedraw = true; // <<< SET FLAG FOR MAIN LOOP

      saveSettingsToFlash();
  }
  
  // Set/UnSet Alarm Button Check
  else if (touchX >= ACTIVATE_ALARM_BUTTON_X && touchX < (ACTIVATE_ALARM_BUTTON_X + ACTIVATE_ALARM_BUTTON_W) &&
          touchY >= ACTIVATE_ALARM_BUTTON_Y && touchY < (ACTIVATE_ALARM_BUTTON_Y + ACTIVATE_ALARM_BUTTON_H)) {
      Serial.println("Alarm Set Button Touched");
      isAlarmSet = !isAlarmSet;
      Serial.println(isAlarmSet);
    if (isAlarmSet)
      drawButtonVisual(ACTIVATE_ALARM_BUTTON_X, ACTIVATE_ALARM_BUTTON_Y, ACTIVATE_ALARM_BUTTON_W, ACTIVATE_ALARM_BUTTON_H, "AKTIV", RED, BLACK, buttonFont, 1);
    else
      drawButtonVisual(ACTIVATE_ALARM_BUTTON_X, ACTIVATE_ALARM_BUTTON_Y, ACTIVATE_ALARM_BUTTON_W, ACTIVATE_ALARM_BUTTON_H, "INAKTIV", DARKGREY, BLACK, buttonFont, 1);
  }
  
}
void displayMessageScreen(const char* line1, const char* line2, uint16_t color) {
    Serial.printf("FUNCTION: displayMessageScreen - L1: %s, L2: %s\n", line1, line2 ? line2 : "NULL");
    if (!gfx) return;
    gfx->fillScreen(BLACK);
    gfx->setFont(NULL);
    gfx->setTextSize(2);
    gfx->setTextColor(color);

    int16_t x1, y1;
    uint16_t w1, h1, w2 = 0, h2 = 0;
    int lineSpacing = 10;

    gfx->getTextBounds(line1, 0, 0, &x1, &y1, &w1, &h1);
    if (line2) gfx->getTextBounds(line2, 0, 0, &x1, &y1, &w2, &h2);

    int totalDrawHeight = h1 + (line2 ? (h2 + lineSpacing) : 0);
    int startY = (h - totalDrawHeight) / 2;
    if (startY < 0) startY = 0;

    centerText(line1, startY, color, NULL, 2);
    if (line2) centerText(line2, startY + h1 + lineSpacing, color, NULL, 2);

    Serial.println("FUNCTION END: displayMessageScreen");
}

void centerText(const char *text, int y, uint16_t color, const GFXfont *font, uint8_t size) {
    if (!gfx || !text || strlen(text) == 0) return;

    int16_t x1, y1;
    uint16_t text_w, text_h;

    gfx->setFont(font);
    gfx->setTextSize(size);
    gfx->getTextBounds(text, 0, 0, &x1, &y1, &text_w, &text_h);

    int cursor_x = (w - text_w) / 2 - x1;
    int cursor_y = y - y1;
    if (cursor_x < 0) cursor_x = 0;

    gfx->setCursor(cursor_x, cursor_y);
    gfx->setTextColor(color);
    gfx->print(text);
}

void handleAlarmAreaTouch() {
    Serial.println("FUNCTION: handleAlarmAreaTouch");
    Serial.println("  Alarm Area Touched! Transitioning to Set Alarm state..."); // Example action

    // --- Action: Go to Set Alarm State ---
    savedClockState= currentClockState;
    currentClockState = STATE_SETTING_ALARM;
    needsFullRedraw = true;             // Need to redraw screen for set alarm mode
    audio_ptr->stopSong();
    activeStationIndex=-1;
    displaySetAlarmScreen(&alarmTime); // Display the setting screen immediately
}


void alarmActivated() {
    if (!gfx) return; // Safety check
    static unsigned long alarmPatternMillis = 0; // Stays here, initialized once
    unsigned long currentMillis = millis();

    if (!alarmIsActive) { // First time condition is met this minute

        Serial.println("!!! ALARM ACTIVATED - Starting Pattern !!!");
        alarmIsActive = true;
        alarmColorIndex = 0; // Reset color pattern
        alarmPatternMillis = currentMillis; // Start pattern timer
        // Optionally, stop any other audio
        if (audio_ptr && audio_ptr->isRunning()) {
            activeStationIndex = -1;
            audio_ptr->stopSong();
        }
        audio_ptr->connecttoFS(SD, alarmSound);
        // Force an immediate first flash and time display
        gfx->fillScreen(alarmColors[alarmColorIndex]); // Initial background color
        alarmColorIndex = (alarmColorIndex + 1) % (sizeof(alarmColors) / sizeof(alarmColors[0]));
    }

    // Handle flashing background and re-drawing time every ~500ms
    if (alarmIsActive && (currentMillis - alarmPatternMillis >= 1500)) { // Interval for flashing
        alarmPatternMillis = currentMillis;

        // --- Flashing Background ---
        gfx->fillScreen(alarmColors[alarmColorIndex]);
        Serial.printf("  Alarm Flash: Color Index %d\n", alarmColorIndex);
        // (Beep logic would go here later)

        alarmColorIndex = (alarmColorIndex + 1) % (sizeof(alarmColors) / sizeof(alarmColors[0]));
       
    }

    if (alarmIsActive) {
        const GFXfont *alarmTimeFont = font_freesansbold18; // Use a large bold font
        uint8_t alarmTimeFontSize = 3; // << CHOOSE A LARGE SIZE (e.g., 2, 3, or 4)

        char timeStr[6]; // HH:MM + null
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

        // Determine text color for contrast against current background
        uint16_t textColor;

        int currentBgColorIndex = (alarmColorIndex == 0) ? ((sizeof(alarmColors) / sizeof(alarmColors[0])) - 1) : (alarmColorIndex - 1);
        if (alarmColors[currentBgColorIndex] == BLACK || alarmColors[currentBgColorIndex] == RED) {
            textColor = WHITE;
        } else {
            textColor = BLACK;
        }
        centerText(timeStr, 230, textColor, alarmTimeFont, alarmTimeFontSize);
    }
}


void UpdateTouchCoordsDisplay() {
    if (!gfx) return;
    char currentCoordsStr[20];
    bool currentlyVisibleTarget = false;
    unsigned long currentMillis = millis();
    // Determine target visibility and string content
    if (touchCoordsVisible) { 
        if (currentMillis - lastTouchMillis <= touchDisplayTimeout) {
            // Timer active and not expired
            currentlyVisibleTarget = true;
            snprintf(currentCoordsStr, sizeof(currentCoordsStr), "T:%3d,%3d", lastDisplayedTouchX, lastDisplayedTouchY);
        } else {
            currentlyVisibleTarget = false;
            currentCoordsStr[0] = '\0';
            touchCoordsVisible = false; 
        }
    } else {
        // Timer not active
        currentlyVisibleTarget = false;
        currentCoordsStr[0] = '\0';
    }
    if (strcmp(currentCoordsStr, previousTouchCoordsStr) != 0) {
        // --- 1. Always Clear the Fixed Area ---
        gfx->fillRect(TOUCH_COORD_AREA_X, TOUCH_COORD_AREA_Y,
                      TOUCH_COORD_AREA_W, TOUCH_COORD_AREA_H, BLACK);

        // --- 2. Draw New Text 
        if (currentlyVisibleTarget) {
            gfx->setFont(NULL); // Default font
            gfx->setTextSize(1); // Small size
            gfx->setTextColor(DARKGREY);

            // simply draw at the top-left of the area
            int16_t x1, y1; uint16_t w1, h1;
            gfx->getTextBounds(currentCoordsStr, 0, 0, &x1, &y1, &w1, &h1);

            // Draw near top-left of the defined area
            int cursorX = TOUCH_COORD_AREA_X - x1; // Adjust slightly for font's internal padding
            int cursorY = TOUCH_COORD_AREA_Y - y1; // Adjust for font's top bearing
            gfx->setCursor(cursorX, cursorY);
            //gfx->print(currentCoordsStr);
            char uptimeStr[32];
            snprintf(uptimeStr, sizeof(uptimeStr), "%s, Uptime %d min.", currentCoordsStr, millis()/1000/60);
            gfx->print(uptimeStr);
        }

        strcpy(previousTouchCoordsStr, currentCoordsStr);
    }
}

void incrementLocalTime() {
    if (!timeSynchronized) return;

    time_t currentTimeSec = mktime(&timeinfo);
    currentTimeSec += 1;
    localtime_r(&currentTimeSec, &timeinfo);
}

void primeFactorsToString(int n, char* buffer, size_t bufferSize) {
    if (!buffer || bufferSize == 0) return;

    buffer[0] = '\0';
    char tempBuf[20];  // temporary buffer to hold one factor string

    if (n <= 0) {
        strncpy(buffer, "0", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    } else if (n == 1) {
        strncpy(buffer, "1", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    } else {
        while (n % 2 == 0) {
            snprintf(tempBuf, sizeof(tempBuf), "2*");
            strncat(buffer, tempBuf, bufferSize - strlen(buffer) - 1);
            n /= 2;
            yield(); // If you're using Arduino or similar environment
        }
        for (int i = 3; i * i <= n; i += 2) {
            while (n % i == 0) {
                snprintf(tempBuf, sizeof(tempBuf), "%d*", i);
                strncat(buffer, tempBuf, bufferSize - strlen(buffer) - 1);
                n /= i;
                yield();
            }
        }
        if (n > 1) {
            snprintf(tempBuf, sizeof(tempBuf), "%d*", n);
            strncat(buffer, tempBuf, bufferSize - strlen(buffer) - 1);
        }
        // Remove trailing '*'
        int len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '*') {
            buffer[len - 1] = '\0';
        }
    }
}
//---AUDIO
// --- Button Definitions ---
const char* station_labels[NUM_STATION_BUTTONS] = {"EGO.FM", "BR3", "ANT", "BR24"}; // Example Labels
const char* station_urls[NUM_STATION_BUTTONS] = { // Example URLs - REPLACE WITH ACTUAL URLs
    "http://streams.egofm.de/egoFM-hq/",
    "http://streams.br.de/bayern3_2.m3u",
    "http://stream.antenne.de/antenne/stream/mp3", // Example, check format
    "http://streams.br.de/br24_2.m3u" // Example, check format
};


// Generic function to draw a button's visual appearance
// Generic function to draw a button's visual appearance
void drawButtonVisual(int x, int y, int w, int h, const char* label, uint16_t bgColor, uint16_t textColor, const GFXfont* font, uint8_t size) {
    if (!gfx) return;
    // Draw background
    gfx->fillRoundRect(x, y, w, h, 5, bgColor);

    // Optional: Draw border (e.g., slightly darker than background or fixed color)
    // gfx->drawRoundRect(x, y, w, h, 5, DARKGREY); // Example: Dark grey border
    // Draw Label centered
    if (label && strlen(label) > 0) {
        int16_t tx, ty; uint16_t tw, th;
        gfx->setFont(font);
        gfx->setTextSize(size);
        gfx->setTextColor(textColor); // <<< Set color BEFORE getTextBounds/print
        gfx->getTextBounds(label, 0, 0, &tx, &ty, &tw, &th);
        int cursorX = x + (w - tw) / 2 - tx;
        int cursorY = y + (h - th) / 2 - ty;
        if (cursorX < 0) cursorX = x; // Align left at least
        gfx->setCursor(cursorX, cursorY);
        gfx->print(label);
    }
}

// Helper function to draw a single station button based on its state
void drawStationButton(int index, bool isActive) {
    if (index < 0 || index >= NUM_STATION_BUTTONS) return;

    int btnX, btnY; 
    if (index==0 || index==2)
      btnX=STAT_BUTTON_0_X;
    else
      btnX=STAT_BUTTON_1_X;
    
    if (index==0 || index==1)
      btnY=STAT_BUTTON_0_Y;
    else
      btnY=STAT_BUTTON_2_Y;


    uint16_t bgColor = isActive ? COLOR_BUTTON_BG_ACTIVE : COLOR_BUTTON_BG_IDLE;
    drawButtonVisual(btnX, btnY, STAT_BUTTON_WIDTH, STAT_BUTTON_HEIGHT,
                     station_labels[index], bgColor, COLOR_BUTTON_TEXT,
                     font_freesans18, 1); // Using FreeSans18pt font
}

void drawVolumeButtons() {

      drawVolumeBar();

    drawButtonVisual(VOL_BUTTON_DOWN_X, VOL_BUTTON_DOWN_Y, VOL_BUTTON_WIDTH, VOL_BUTTON_HEIGHT,
                     VOL_DOWN_LABEL, COLOR_BUTTON_BG_IDLE, COLOR_BUTTON_TEXT,
                     font_freesansbold18, 2); // Larger font for +/-

    // Draw Volume Up
    drawButtonVisual(VOL_BUTTON_UP_X, VOL_BUTTON_UP_Y, VOL_BUTTON_WIDTH, VOL_BUTTON_HEIGHT,
                     VOL_UP_LABEL, COLOR_BUTTON_BG_IDLE, COLOR_BUTTON_TEXT,
                     font_freesansbold18, 2); // Larger font for +/-
}
void drawVolumeBar()
{
      Serial.println("  Drawing Volume Buttons and Bar...");


    // fill of volume depending if radio active or not
    int volumeColor=DARKGREY;
    if (activeStationIndex!=-1)
      volumeColor=GREEN;
    // --- 1. Draw Volume Bar 
    // Outline

      gfx->drawRect(VOL_BAR_X, VOL_BAR_Y, VOL_BAR_WIDTH, VOL_BAR_HEIGHT, WHITE);

      // Calculate Filled Height for the Bar
      int bar_inner_margin = 2; // Small margin inside the outline
      int bar_fillable_width = VOL_BAR_WIDTH - (bar_inner_margin * 2);
      int bar_fillable_height = VOL_BAR_HEIGHT - (bar_inner_margin * 2);
      int fill_height = 0;

      if (MAX_VOLUME > 0 && bar_fillable_height > 0) { // Avoid division by zero and ensure positive height
          fill_height = map(currentVolume, 0, MAX_VOLUME, 0, bar_fillable_height);
          if (fill_height < 0) fill_height = 0; // Sanity check
          if (fill_height > bar_fillable_height) fill_height = bar_fillable_height;
      }

      // Clear previous fill inside the bar (only the fillable area)
      gfx->fillRect(VOL_BAR_X + bar_inner_margin,
                    VOL_BAR_Y + bar_inner_margin,
                    bar_fillable_width,
                    bar_fillable_height,
                    BLACK);

      // Draw Current Volume Fill (from the bottom up)
      if (fill_height > 0) {
          gfx->fillRect(VOL_BAR_X + bar_inner_margin,              // X (inside outline)
                        VOL_BAR_Y + bar_inner_margin + (bar_fillable_height - fill_height), // Y (from bottom up)
                        bar_fillable_width,                        // Width (inside outline)
                        fill_height,                               // Calculated fill height
                        volumeColor);                                    // Fill color
      }
}
void displayAdditionalInfo(const char *info, int color)
{
    if (currentClockState==STATE_SETTING_ALARM)
        return;
    int letterPos = 295;
    int boxPos = letterPos - 20;
    if (info==NULL)
    {
      gfx->fillRect(0, boxPos, 480, 50, BLACK);
      return;
    }
    const int MaxStringLength = 40;
    String interpret;
    String song;
    
    Serial.print("streamtitle ");
    String infoStr = info;

    // Replace umlauts
    infoStr.replace("ü", "ue");
    infoStr.replace("ö", "oe");
    infoStr.replace("ä", "ae");
    infoStr.replace("ß", "ss");
    infoStr.replace("Ü", "Ue");
    infoStr.replace("Ö", "Oe");
    infoStr.replace("Ä", "Ae");

    Serial.println(infoStr);

    // Use last index of '-' as delimiter
    int delim = infoStr.lastIndexOf('-');
    if (delim == -1)
        delim = infoStr.lastIndexOf(':');

    if (delim == -1 || delim > MaxStringLength)
    {
        // No delimiter: split into two parts manually
        interpret = infoStr.substring(0, MaxStringLength);
        song = infoStr.substring(MaxStringLength, MaxStringLength * 2);
    }
    else
    {
        // Split based on delimiter (trim removes surrounding spaces)
        interpret = infoStr.substring(0, delim);
        song = infoStr.substring(delim + 1);
        
        // Limit length
        interpret = interpret.substring(0, MaxStringLength);
        song = song.substring(0, MaxStringLength);
    }



    // Clear the area first
    gfx->fillRect(0, boxPos, 480, 50, BLACK);

    // Display Radio Info
    const GFXfont *radioFont = font_freesans12;
    gfx->setCursor(5, letterPos);
    gfx->setFont(radioFont);
    gfx->setTextSize(1);
    gfx->setTextColor(color);
    gfx->print(interpret);
    gfx->setCursor(5, letterPos + 24);
    gfx->print(song);
}

void loadSettingsFromFlash() {
    Serial.println("FUNCTION: loadSettingsFromFlash");
    preferences.begin(PREFERENCES_NAMESPACE, false); // false = read/write mode

    // Load Volume (with a default if not found)
    currentVolume = preferences.getInt(KEY_VOLUME, 10); // Default to volume 10
    Serial.printf("  Loaded Volume: %d\n", currentVolume);

    // Load Alarm Set status (with a default)
    isAlarmSet = preferences.getBool(KEY_ALARM_SET, false); // Default to alarm off
    Serial.printf("  Loaded isAlarmSet: %s\n", isAlarmSet ? "true" : "false");

    // Load Alarm Time (with defaults if not found)
    alarmTime.tm_hour = preferences.getInt(KEY_ALARM_HOUR, 7);  // Default 07:00
    alarmTime.tm_min = preferences.getInt(KEY_ALARM_MIN, 0);
    alarmTime.tm_sec = 0; // Seconds usually not set for alarms
    // You might want to load/save tm_wday, tm_mday, tm_mon, tm_year if you have more complex alarms
    // For now, these will be set by mktime() when the clock is running or if you add a date setting feature.
    mktime(&alarmTime); // Normalize the struct (calculates weekday, etc. for the default date)

    Serial.printf("  Loaded Alarm Time: %02d:%02d, Set: %s\n", alarmTime.tm_hour, alarmTime.tm_min, isAlarmSet ? "ON" : "OFF");

    preferences.end(); // Close the preferences
    Serial.println("FUNCTION END: loadSettingsFromFlash");
}

void saveSettingsToFlash() {
    Serial.println("FUNCTION: saveSettingsToFlash");
    preferences.begin(PREFERENCES_NAMESPACE, false); // false = read/write mode

    preferences.putInt(KEY_VOLUME, currentVolume);
    preferences.putBool(KEY_ALARM_SET, isAlarmSet);
    preferences.putInt(KEY_ALARM_HOUR, alarmTime.tm_hour);
    preferences.putInt(KEY_ALARM_MIN, alarmTime.tm_min);
    // preferences.putInt(KEY_ALARM_WDAY, alarmTime.tm_wday); // If saving weekday

    Serial.printf("  Saved Volume: %d\n", currentVolume);
    Serial.printf("  Saved Alarm Time: %02d:%02d, Set: %s\n", alarmTime.tm_hour, alarmTime.tm_min, isAlarmSet ? "ON" : "OFF");

    preferences.end(); // Close the preferences
    Serial.println("FUNCTION END: saveSettingsToFlash");
}
