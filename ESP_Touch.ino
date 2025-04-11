#include "CustomDef.h" // Includes, Defines, Extern Globals
#include "Helper.h"    // Function Declarations

void setup() {
    Serial.begin(115200);
    Serial.println("Fractal Clock Skeleton - Setup");
    currentClockState = STATE_BOOTING;

    InstantiateGfxAndTouchObjects(); // Creates gfx, bus, rgbpanel, ts_ptr objects
    InitDisplay();                   // Initializes gfx, sets w/h, clears screen, backlight
    InitTouch();                     // Initializes touch controller via I2C
    //InitSD();
    //InitAudio();
    Serial.println("Setup Complete.");
}

// --- Loop ---
void loop() {
    currentClockState = STATE_INIT_WIFI;
    unsigned long currentMillis = millis();
    handleTouchInput();
    if (touchCoordsVisible)
        UpdateTouchCoordsDisplay();


    // --- Main State Machine ---
    switch (currentClockState) {
        case STATE_INIT_WIFI:
            InitWifiManager();
            break;
        case STATE_RUNNING:
            // Periodic NTP Re-sync Check
            if (timeSynchronized && (currentMillis - lastNtpSyncMillis >= ntpSyncInterval)) {
                 resyncNTPTime();
            }
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
        case STATE_WIFI_CONFIG_PORTAL: // Managed by WiFiManager
        case STATE_CONNECTING_WIFI:    // Managed by WiFiManager
        case STATE_WAITING_FOR_NTP:
            // Transient states, do nothing specific in loop
            break;
    }

    yield(); // Yield for system tasks
}
