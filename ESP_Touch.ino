#include "CustomDef.h" // Includes, Defines, Extern Globals
#include "Helper.h"    // Function Declarations

// --- NO GLOBAL VARIABLE DEFINITIONS HERE ---

// --- Setup ---
void setup() {
    Serial.begin(115200);
    Serial.println("Fractal Clock Skeleton - Setup");
    currentClockState = STATE_BOOTING;

    // --- Call Initialization Functions ---
    InstantiateGfxAndTouchObjects(); // Creates gfx, bus, rgbpanel, ts_ptr objects
    InitDisplay();                   // Initializes gfx, sets w/h, clears screen, backlight
    InitTouch();                     // Initializes touch controller via I2C

    // --- Network and Time Setup ---
    currentClockState = STATE_INIT_WIFI;
    if (InitWifiManager()) { // Tries saved creds or runs portal
        currentClockState = STATE_WAITING_FOR_NTP;
        if (syncNTPTime()) { // Tries NTP sync, seeds timeinfo on success
            currentClockState = STATE_RUNNING;
            gfx->fillScreen(BLACK);      // Clear message screen
            ShowStaticFields(&timeinfo); // Draw initial date/weekday
        } else {
            currentClockState = STATE_WIFI_NTP_FAILED;
            // Error message shown by syncNTPTime
        }
    } else { // WiFiManager failed
        currentClockState = STATE_WIFI_NTP_FAILED;
        // Error message shown by InitWifiManager
    }

    previousMillis = millis(); // Start the 1-second display timer
    Serial.println("Setup Complete.");
}

// --- Loop ---
void loop() {
    unsigned long currentMillis = millis();

    // --- Read Touch Input & Update Visibility State ---
    handleTouchInput(); // Call the helper function to read hardware

    // --- Main State Machine ---
    switch (currentClockState) {
        case STATE_RUNNING:
            // Periodic NTP Re-sync Check
            if (timeSynchronized && (currentMillis - lastNtpSyncMillis >= ntpSyncInterval)) {
                 resyncNTPTime();
            }

            // Update Display every second
            if (currentMillis - previousMillis >= interval) {
                previousMillis = currentMillis;
                if (timeSynchronized) {
                    incrementLocalTime();
                    displayClock(&timeinfo); // Update dynamic clock/factors
                    firstDisplayDone = true;
                } else {
                    // Should have transitioned state in resyncNTPTime if sync lost
                    currentClockState = STATE_WIFI_NTP_FAILED;
                    displayMessageScreen("NTP Sync Lost", "Check Network", RED);
                    firstDisplayDone = false;
                }
            }
            // Handle first display after setup
            else if (timeSynchronized && !firstDisplayDone) {
                 displayClock(&timeinfo);
                 firstDisplayDone = true;
            }
            break; // End STATE_RUNNING

        case STATE_WIFI_NTP_FAILED:
            // Display error message (already shown) and wait
            // Could add retry logic here based on touch or timer
            break;

        case STATE_BOOTING:
        case STATE_INIT_WIFI:
        case STATE_WIFI_CONFIG_PORTAL: // Managed by WiFiManager
        case STATE_CONNECTING_WIFI:    // Managed by WiFiManager
        case STATE_WAITING_FOR_NTP:
            // Transient states, do nothing specific in loop
            break;
    }

    // --- Update Touch Coordinates Display ---
    // Update display unless setting time manually (if feature re-added)
     if (currentClockState != STATE_BOOTING && currentClockState != STATE_INIT_WIFI && currentClockState != STATE_CONNECTING_WIFI && currentClockState != STATE_WIFI_CONFIG_PORTAL && currentClockState != STATE_WAITING_FOR_NTP) {
        UpdateTouchCoordsDisplay();
     }


    yield(); // Yield for system tasks
}
