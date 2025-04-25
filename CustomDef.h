#ifndef CUSTOMDEF_H
#define CUSTOMDEF_H


#define TOUCH_COORD_AREA_X 1
#define TOUCH_COORD_AREA_Y 1
#define TOUCH_COORD_AREA_W 80 // Adjust width as needed (e.g., for "T:480,480")
#define TOUCH_COORD_AREA_H 12 // Adjust height as needed (default font size 1 height is ~8px + padding)
// --- Library Includes ---
// Include libraries needed for types used in declarations below
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <time.h>
#include <Wire.h>
#include <TAMC_GT911.h>
#include <Arduino_GFX_Library.h>
#include "Audio.h"
#include "SD.h"

// --- Font Includes ---
// Include fonts here as they define types (GFXfont) needed by extern declarations
#include "C:\Users\mcgre\Documents\Arduino\libraries\Adafruit_GFX_Library\Fonts\FreeSansBold18pt7b.h"
#include "C:\Users\mcgre\Documents\Arduino\libraries\Adafruit_GFX_Library\Fonts\FreeSans18pt7b.h"
#include "C:\Users\mcgre\Documents\Arduino\libraries\Adafruit_GFX_Library\Fonts\FreeSans12pt7b.h"

// --- Display Configuration ---
#define GFX_BL 38
extern Arduino_DataBus *bus;         // Pointer Declaration
extern Arduino_ESP32RGBPanel *rgbpanel; // Pointer Declaration
extern Arduino_RGB_Display *gfx;      // Pointer Declaration
extern const uint8_t st7701_type5_init_operations[]; // Array Declaration
extern const int st7701_type5_init_operations_len;  // Length Declaration

// --- WiFi Credentials ---
extern const char *ssid;
extern const char *password;

// --- NTP Settings ---
extern const char *ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern const long ntpSyncInterval;

extern Audio audio;
// --- Touch Screen Pin Configuration ---
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_SCL 45
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST -1

// --- SD Card and Audio Pin Configuration ---
#define I2S_DOUT      40
#define I2S_BCLK      1
#define I2S_LRC       2

#define SD_CS         42
#define SPI_MOSI      47
#define	SPI_SCK       48
#define SPI_MISO      41

// --- Clock States ---
enum ClockState {
    STATE_BOOTING,
    STATE_INIT_WIFI,
    STATE_WIFI_CONFIG_PORTAL,
    STATE_CONNECTING_WIFI,
    STATE_WAITING_FOR_NTP,
    STATE_RUNNING,
    STATE_WIFI_NTP_FAILED,
    STATE_PLAY_AUDIO,
    STATE_SETTING_ALARM,
    NUM_CLOCK_STATES        // just a counter of the total states, leave it last!!!      
};

extern const char *ClockStateNames[NUM_CLOCK_STATES]; // Use NUM_CLOCK_STATES for size
// --- Factor String Buffer Size ---
#define MAX_FACTOR_STR_LEN 512

// --- Global Variables (Declarations) ---
extern int32_t w;
extern int32_t h;
extern struct tm timeinfo;
extern unsigned long previousMillis;
extern const long interval;
extern bool timeSynchronized;
extern unsigned long lastNtpSyncMillis;
extern ClockState currentClockState;
extern ClockState lastClockState;
extern bool firstDisplayDone;
extern bool wasTouchedPreviously;
// German days/months
extern const char *Wochentage[];
extern const char *Monate[];
extern unsigned long lastButtonActionTime;
extern const unsigned long buttonDebounceDelay; // Make const accessible if needed elsewhere

// Colors
extern uint16_t RGB565_LIGHT_GREY;
// extern uint16_t COLOR_OK_BUTTON_GREEN; // Add if manual set re-added

// Touch Object
extern TAMC_GT911 *ts_ptr; // Pointer Declaration

// Touch Coordinate Display
extern unsigned long lastTouchMillis;
extern const unsigned long touchDisplayTimeout;
extern bool touchCoordsVisible;
extern int lastDisplayedTouchX;
extern int lastDisplayedTouchY;
extern char previousTouchCoordsStr[20];
extern uint16_t prev_touch_coords_x;
extern uint16_t prev_touch_coords_y;
extern uint16_t prev_touch_coords_w;
extern uint16_t prev_touch_coords_h;

// Font pointers
extern const GFXfont *font_freesansbold18;
extern const GFXfont *font_freesans18;
extern const GFXfont *font_freesans12;
//Radio def

#define NUM_STATION_BUTTONS 4
#define BUTTON_COLS 2
#define BUTTON_ROWS 2
#define BUTTON_WIDTH (w / 3) // Width is 1/3 of screen
#define BUTTON_HEIGHT 60     // Fixed height (adjust as needed)
#define BUTTON_H_SPACE 10    // Horizontal space between buttons
#define BUTTON_V_SPACE 10    // Vertical space between buttons
#define BUTTON_MARGIN_LEFT 10 // Margin from left edge
#define BUTTON_MARGIN_BOTTOM 10 // Margin from bottom edge

// Define placeholder labels and corresponding stream URLs
extern const char* station_labels[NUM_STATION_BUTTONS];
extern const char* station_urls[NUM_STATION_BUTTONS];

extern int activeStationIndex; // Index of the active station button (-1 for none)
extern bool touchRegisteredThisPress;

// Volume Buttons
#define VOL_BUTTON_WIDTH 50
#define VOL_BUTTON_HEIGHT BUTTON_HEIGHT // Same height as station buttons
#define VOL_BUTTON_V_SPACE BUTTON_V_SPACE
#define VOL_BUTTON_MARGIN_RIGHT 10
#define VOL_UP_LABEL "+"
#define VOL_DOWN_LABEL "-"

// --- Global Variables (Declarations) ---
// ... existing externs ...
extern uint16_t COLOR_BUTTON_BG_IDLE;   // Color for inactive button
extern uint16_t COLOR_BUTTON_BG_ACTIVE; // Color for active button
extern uint16_t COLOR_BUTTON_TEXT;      // Color for button text



// Standard GFX Colors (useful defines)
#define BLACK 0x0000
#define NAVY 0x000F
#define DARKGREEN 0x03E0
#define DARKCYAN 0x03EF
#define MAROON 0x7800
#define PURPLE 0x780F
#define OLIVE 0x7BE0
#define LIGHTGREY 0xD69A
#define DARKGREY 0x7BEF
#define BLUE 0x001F
#define GREEN 0x07E0
#define CYAN 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define ORANGE 0xFD20
#define GREENYELLOW 0xAFE5
#define PINK 0xF81F // Same as MAGENTA

// Alarm State
extern struct tm alarmTime;       // Holds the HH:MM for the alarm
extern bool isAlarmSet;           // Is the alarm currently active? (Keep for later)
extern bool needsFullRedraw;
extern bool alarmJustTriggered;

#define ALARM_LABEL "Alarm: "        // Text prefix
#define ALARM_AREA_PADDING 5       // Pixels around text inside rectangle
#define ALARM_AREA_MARGIN_RIGHT 10 // Pixels from right edge of screen
#define ALARM_TIME_Y_OFFSET 15      // Space below factor line
#endif // CUSTOMDEF_H

// --- Touch Zones & Dimensions ---

// General Zones (used by Set Time and Set Alarm)
#define HOUR_TOUCH_X      (w * 0.1) // Base X for Hour adjust zone
#define HOUR_TOUCH_W      (w * 0.3) // Base Width for Hour adjust zone
#define MINUTE_TOUCH_X    (w * 0.6) // Base X for Minute adjust zone
#define MINUTE_TOUCH_W    (w * 0.3) // Base Width for Minute adjust zone
#define SETTING_TOUCH_ZONE_H  60    // Common Height for HH/MM touch zones in setting screens

// Offsets for placing touch zones relative to displayed time in setting screens
#define ALARM_SET_Y_OFFSET  20      // Offset below alarm time display in SET_ALARM screen

// OK Button (Common for Set Time/Alarm screens)
#define OK_BUTTON_W       100
#define OK_BUTTON_H       50
#define OK_BUTTON_X       ((w - OK_BUTTON_W) / 2) // Centered X

// Alarm Display Area (Main Screen - coordinates/dimensions for hitting the alarm display itself)
#define ALARM_TIME_Y_OFFSET 15      // Approx space below factor line on main screen
#define ALARM_TIME_TOUCH_W (w * 0.4) // Approx Width for touching the time HH:MM on main screen
#define ALARM_ICON_WIDTH   48       // Use define from alarm_icon.h if different
#define ALARM_ICON_HEIGHT  48       // Use define from alarm_icon.h if different
#define ALARM_ICON_X_OFFSET 10      // Space between displayed alarm time and icon
#define ALARM_ICON_TOUCH_W (ALARM_ICON_WIDTH + 10) // Touch area width around icon

// Alarm Setting Screen Specific Touch Zones (can reuse base defines)
#define ALARM_SET_HOUR_X    HOUR_TOUCH_X
#define ALARM_SET_HOUR_W    HOUR_TOUCH_W
#define ALARM_SET_MINUTE_X  MINUTE_TOUCH_X
#define ALARM_SET_MINUTE_W  MINUTE_TOUCH_W
#define ALARM_SET_TOUCH_H   SETTING_TOUCH_ZONE_H // Use the common height

// === END OF ADDED/RESTORED DEFINES ===


// --- Global Variables (Declarations) ---
// ... existing externs ...
extern uint16_t COLOR_OK_BUTTON_GREEN; // Ensure this is declared if used

// --- Touch Zones for ALARM SETTING Screen (Specific Coordinates) ---
#define ALARM_SET_H_X1 150 // Hour zone top-left X
#define ALARM_SET_H_Y1 175 // Hour zone top-left Y
#define ALARM_SET_H_X2 230 // Hour zone bottom-right X
#define ALARM_SET_H_Y2 230 // Hour zone bottom-right Y

#define ALARM_SET_M_X1 250 // Minute zone top-left X
#define ALARM_SET_M_Y1 175 // Minute zone top-left Y (Corrected to match Hours Y1 based on usual layout)
#define ALARM_SET_M_X2 330 // Minute zone bottom-right X
#define ALARM_SET_M_Y2 230 // Minute zone bottom-right Y

extern unsigned long alarmAreaTouchStartTime; // millis() when touch in alarm area started
extern bool alarmAreaTouchInProgress;         // Is touch currently held down in the area?
extern const unsigned long alarmLongPressDuration; // Duration for long press



//---------------fixed size




#define ALARM_RECT_X 363
#define ALARM_RECT_Y 194
#define ALARM_RECT_W 107
#define ALARM_RECT_H 63

#define CLOCK_X 100
#define CLOCK_Y 140
#define CLOCK_RECT_X 100
#define CLOCK_RECT_Y 90
#define CLOCK_RECT_W 270
#define CLOCK_RECT_H 60

#define FRACTAL_X 2
#define FRACTAL_Y 210


