#ifndef CUSTOMDEF_H
#define CUSTOMDEF_H


#define TOUCH_COORD_AREA_X 1
#define TOUCH_COORD_AREA_Y 1
#define TOUCH_COORD_AREA_W 180 // Adjust width as needed (e.g., for "T:480,480")
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

/*
// Include fonts here as they define types (GFXfont) needed by extern declarations
#include "/home/mcgreg/Arduino/libraries/Adafruit_GFX_Library/Fonts/FreeSansBold18pt7b.h"
#include "/home/mcgreg/Arduino/libraries/Adafruit_GFX_Library/Fonts/FreeSans18pt7b.h"
#include "/home/mcgreg/Arduino/libraries/Adafruit_GFX_Library/Fonts/FreeSans12pt7b.h"
*/
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

extern Audio *audio_ptr;
extern const char *alarmSound; 
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

enum ClockState {
    STATE_BOOTING,
    STATE_INIT_WIFI,
    STATE_WIFI_CONFIG_PORTAL,
    STATE_CONNECTING_WIFI,
    STATE_WAITING_FOR_NTP,
    STATE_RUNNING,
    STATE_WIFI_FAILED,
    STATE_NTP_FAILED,
    STATE_SETTING_ALARM,
    STATE_SETTING_CLOCK,
    STATE_PLAY_AUDIO,
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

extern unsigned long lastWifiReconnectAttemptMillis;
extern int wifiReconnectAttemptCount;

// Colors
extern uint16_t RGB565_LIGHT_GREY;
// extern uint16_t COLOR_OK_BUTTON_GREEN; // Add if manual set re-added

// Touch Object
extern TAMC_GT911 *ts_ptr; // Pointer Declaration

// Touch Coordinate Display
extern unsigned long lastTouchMillis;
extern unsigned long elapsedTouchTime;
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


extern int activeStationIndex; // Index of the active station button (-1 for none)


const unsigned long WIFI_RECONNECT_INTERVALS[] = {
    5 * 60 * 1000UL,   // 5 minutes
    15 * 60 * 1000UL,  // 15 minutes
    60 * 60 * 1000UL,  // 1 hour
    6 * 60 * 1000UL,   // 6 hours
    24 * 60 * 1000UL   // 24 hours (max interval)
};
const int NUM_WIFI_RECONNECT_INTERVALS = sizeof(WIFI_RECONNECT_INTERVALS) / sizeof(WIFI_RECONNECT_INTERVALS[0]);

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
extern bool alarmIsActive;
extern bool needsFullRedraw;
extern bool alarmJustTriggered;

// --- Touch Zones & Dimensions ---

// General Zones (used by Set Time and Set Alarm)
#define HOUR_TOUCH_X      (w * 0.1) // Base X for Hour adjust zone
#define HOUR_TOUCH_W      (w * 0.3) // Base Width for Hour adjust zone
#define MINUTE_TOUCH_X    (w * 0.6) // Base X for Minute adjust zone
#define MINUTE_TOUCH_W    (w * 0.3) // Base Width for Minute adjust zone
#define SETTING_TOUCH_ZONE_H  60    // Common Height for HH/MM touch zones in setting screens

// OK Button (Common for Set Time/Alarm screens)
#define OK_BUTTON_W       100
#define OK_BUTTON_H       50
#define OK_BUTTON_X       ((w - OK_BUTTON_W) / 2) // Centered X
#define OK_BUTTON_Y       280
// OK Button (Common for Set Time/Alarm screens)
#define ACTIVATE_ALARM_BUTTON_W       200
#define ACTIVATE_ALARM_BUTTON_H       50
#define ACTIVATE_ALARM_BUTTON_X       ((w - ACTIVATE_ALARM_BUTTON_W) / 2) // Centered X
#define ACTIVATE_ALARM_BUTTON_Y       360



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

//alarm text 160
//alarm set 227


// --- Touch Zones for ALARM SETTING Screen (Specific Coordinates) ---
#define ALARM_SET_H_X 150 // Hour zone top-left X
#define ALARM_SET_M_X 248 // Minute zone top-left X
#define ALARM_SET_BOX_Y 155
#define ALARM_SET_BOX_H 65
#define ALARM_SET_BOX_W 80


extern unsigned long alarmAreaTouchStartTime; // millis() when touch in alarm area started
extern bool alarmAreaTouchInProgress;         // Is touch currently held down in the area?
extern const unsigned long alarmLongPressDuration; // Duration for long press



//---------------fixed size
#define WEEKDAY_Y   12
#define DATE_Y      48

#define CLOCK_X 100
#define CLOCK_Y 170

//Alarm Time Box
#define ALARM_RECT_X 363
#define ALARM_RECT_Y 210
#define ALARM_RECT_W 107
#define ALARM_RECT_H 50


#define CLOCK_RECT_X CLOCK_X
#define CLOCK_RECT_Y CLOCK_Y-50
#define CLOCK_RECT_W 270
#define CLOCK_RECT_H 60

#define FRACTAL_X 2
#define FRACTAL_Y 210

//Radio Buttons
#define NUM_STATION_BUTTONS 4
#define STAT_BUTTON_WIDTH 160
#define STAT_BUTTON_HEIGHT 60
#define STAT_BUTTON_0_X 10
#define STAT_BUTTON_0_Y 340
#define STAT_BUTTON_1_X 180
#define STAT_BUTTON_1_Y 340
#define STAT_BUTTON_2_X 10
#define STAT_BUTTON_2_Y 410
#define STAT_BUTTON_3_X 180
#define STAT_BUTTON_3_Y 410


// Volume Buttons
#define VOL_BUTTON_UP_X 420
#define VOL_BUTTON_UP_Y STAT_BUTTON_0_Y
#define VOL_BUTTON_DOWN_X VOL_BUTTON_UP_X
#define VOL_BUTTON_DOWN_Y STAT_BUTTON_2_Y

#define VOL_BUTTON_WIDTH 50
#define VOL_BUTTON_HEIGHT STAT_BUTTON_HEIGHT // Same height as station buttons
#define VOL_BUTTON_MARGIN_RIGHT 10
#define VOL_UP_LABEL "+"
#define VOL_DOWN_LABEL "-"
#define ALARM_LABEL "Alarm"
// Define placeholder labels and corresponding stream URLs
extern const char* station_labels[NUM_STATION_BUTTONS];
extern const char* station_urls[NUM_STATION_BUTTONS];

// --- New Volume Bar Defines ---
// Position the bar to the left of the volume buttons
#define VOL_BAR_GAP 5      // Gap between bar and buttons
#define VOL_BAR_WIDTH (VOL_BUTTON_WIDTH / 2)
// Bar should span the height of both buttons and the gap between them
// Assuming VOL_BUTTON_UP_Y is above VOL_BUTTON_DOWN_Y
#define VOL_BAR_HEIGHT ( (VOL_BUTTON_DOWN_Y + VOL_BUTTON_HEIGHT) - VOL_BUTTON_UP_Y )
#define VOL_BAR_X      (VOL_BUTTON_DOWN_X - VOL_BAR_WIDTH - VOL_BAR_GAP)
#define VOL_BAR_Y      VOL_BUTTON_UP_Y // Align top of bar with top of Vol Up button

// --- Volume Level ---
extern int currentVolume; // 0-21 (or your chosen range)
#define MAX_VOLUME 21     // Max volume level

#define PREFERENCES_NAMESPACE "fractalClock" // Namespace for your app's settings
#define KEY_VOLUME "volume"
#define KEY_ALARM_HOUR "alarmHour"
#define KEY_ALARM_MIN "alarmMin"
#define KEY_ALARM_SET "alarmSet"

#endif
