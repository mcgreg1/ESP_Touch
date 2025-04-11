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
    InitAudio();
    Serial.println("INIT WIFI");
    currentClockState = STATE_INIT_WIFI;

    //    WiFi.begin(ssid, password);
    //    while(WiFi.status() != WL_CONNECTED){ delay(500); Serial.print("."); yield(); }
    

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
    } 
    else { // WiFiManager failed
        currentClockState = STATE_WIFI_NTP_FAILED;
        // Error message shown by InitWifiManager
    }
    

    previousMillis = millis(); // Start the 1-second display timer
    Serial.println("Setup Complete.");
    audio.connecttohost("http://streams.egofm.de/egoFM-hq/");
}

// --- Loop ---
void loop() {
    unsigned long currentMillis = millis();
    handleTouchInput();
    audio.loop();
    if (touchCoordsVisible)
        UpdateTouchCoordsDisplay();

    if (currentClockState != lastClockState) { // Print if state changed during init
         Serial.printf("State Change: %s -> %s\n", ClockStateNames[lastClockState], ClockStateNames[currentClockState]);
         lastClockState=currentClockState;
    }
    if (currentClockState==STATE_RUNNING)
    {
        //audio.connecttohost("http://streams.egofm.de/egoFM-hq/");
        currentClockState=STATE_PLAY_AUDIO;
    }
    if (currentClockState==STATE_PLAY_AUDIO)
    {
            
    }
    /*
    // --- Main State Machine ---
    switch (currentClockState) {
        case STATE_INIT_WIFI:
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
    */
    yield(); // Yield for system tasks
}
// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}
