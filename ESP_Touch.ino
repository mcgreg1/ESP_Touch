#include "CustomDef.h" // Includes, Defines, Extern Globals
#include "Helper.h"    // Function Declarations

void setup() {
    Serial.begin(115200);
    Serial.println("Fractal Clock Skeleton - Setup");
    currentClockState = STATE_BOOTING;
    loadSettingsFromFlash();
    InstantiateGfxAndTouchObjects(); // Creates gfx, bus, rgbpanel, ts_ptr objects
    InitDisplay();                   // Initializes gfx, sets w/h, clears screen, backlight
    InitTouch();                     // Initializes touch controller via I2C
    //InitSD();
    InitAudio();
    initializeColors();
    Serial.println("INIT WIFI");
    currentClockState = STATE_INIT_WIFI;

    if (InitWifiManager()) { // Tries saved creds or runs portal
        currentClockState = STATE_WAITING_FOR_NTP;
        if (syncNTPTime()) { // Tries NTP sync, seeds timeinfo on success
            currentClockState = STATE_RUNNING;
            gfx->fillScreen(BLACK);      // Clear message screen
            ShowStaticFields(&timeinfo); // Draw initial date/weekday
            
        } 
        else {
            currentClockState = STATE_NTP_FAILED;
            // Error message shown by syncNTPTime
        }
    } 
    else { // WiFiManager failed
        currentClockState = STATE_WIFI_FAILED;
        // Error message shown by InitWifiManager
    }
    
    previousMillis = millis(); // Start the 1-second display timer
    Serial.println("Setup Complete.");
    elapsedTouchTime=0;
}

//after setup we are in running (all ok), ntp failed or wifi failed

// --- Loop ---
void loop() {
    unsigned long currentMillis = millis();

    // --- 1. Read Touch Hardware State ---
    bool isTouchedNow = false;
    int currentTouchX = -1, currentTouchY = -1;
    handleTouchInput();
    audio_ptr->loop();
    
// --- Periodic NTP Re-sync Check ---
    // Check if time has been synchronized at least once and the interval has passed
    if (timeSynchronized && (currentMillis - lastNtpSyncMillis >= ntpSyncInterval)) 
    {
        resyncNTPTime();
    }

    // --- Update Display via Local Time Increment (Every Second) ---
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; // Reset the 1-second tick timer

        if (timeSynchronized) {
            // --- Increment Local Time ---
            time_t currentTimeSec = mktime(&timeinfo); // Convert struct tm to seconds since epoch
            int dayBeforeIncrement = timeinfo.tm_mday;
            currentTimeSec += 1;                       // Add one second
            localtime_r(&currentTimeSec, &timeinfo);   // Convert back to struct tm (thread-safe), updating global timeinfo
            if (timeinfo.tm_mday != dayBeforeIncrement) //daychange
                needsFullRedraw = true;

            displayClock(&timeinfo);
        }
      if (isAlarmSet)
      {
        if ((timeinfo.tm_hour == alarmTime.tm_hour &&
                    timeinfo.tm_min == alarmTime.tm_min && timeinfo.tm_sec == alarmTime.tm_sec) || alarmIsActive)
                {
                  alarmActivated();
                }
      }
    }

    if (touchCoordsVisible)
        UpdateTouchCoordsDisplay();

    if (currentClockState != lastClockState) { // Print if state changed during init
         Serial.printf("State Change: %s -> %s\n", ClockStateNames[lastClockState], ClockStateNames[currentClockState]);
         lastClockState=currentClockState;
    }
    if (currentClockState==STATE_RUNNING)
    {
        if (needsFullRedraw) {
            gfx->fillScreen(BLACK);
            //delay(100);
            ShowStaticFields(&timeinfo);
            //delay(100);
            needsFullRedraw = false;
        }
    }
    else if (currentClockState==STATE_NTP_FAILED)
    {
      resyncNTPTime();
    }
    else if (currentClockState==STATE_WIFI_FAILED)
    {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi lost, trying to reconnect...");
      WiFi.begin();  // Uses saved SSID and password
      unsigned long startAttemptTime = millis();

      // Wait for connection or timeout after 10 seconds
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(100);
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Reconnected to WiFi!");
      } else {
        Serial.println("Reconnect failed.");
      }
    }

      resyncNTPTime();
    }

    //last touch time update
    elapsedTouchTime = currentMillis - lastTouchMillis;
    //Serial.printf("Last touch time: %d ms\n", elapsedTouchTime);

    yield(); // Yield for system tasks
}
// optional

void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_info(const char *info)
{
    Serial.print("info        "); 
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
  displayAdditionalInfo(info);

}

