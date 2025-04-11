#ifndef CUSTOMDEF_H
#define CUSTOMDEF_H


#define TOUCH_COORD_AREA_X 5
#define TOUCH_COORD_AREA_Y 5
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
    STATE_WIFI_NTP_FAILED
};

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
extern bool firstDisplayDone;

// German days/months
extern const char *Wochentage[];
extern const char *Monate[];

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

#endif // CUSTOMDEF_H
