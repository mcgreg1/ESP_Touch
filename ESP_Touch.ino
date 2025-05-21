//TODO: 
//1. FTP Server
//2. Local File Playback
//3. Bluetooth
//4. Web Interface

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
    InitSD();
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
    if (currentClockState==STATE_WIFI_FAILED) //set the time manually
    {
      currentClockState=STATE_SETTING_CLOCK;
      lastWifiReconnectAttemptMillis=millis();
      isAlarmSet=false;
      displaySetClockScreen(&timeinfo);
    }
    Serial.println("Setup Complete.");
    elapsedTouchTime=0;
     
}

//after setup we are in running (all ok), ntp failed or wifi failed

// --- Loop ---
void loop() {
    unsigned long currentMillis = millis();

    bool isTouchedNow = false;
    int currentTouchX = -1, currentTouchY = -1;
    handleTouchInput();
    audio_ptr->loop();
    if (touchCoordsVisible)
      UpdateTouchCoordsDisplay();

    if (currentClockState != lastClockState) 
    { // Print if state changed during init
         Serial.printf("State Change: %s -> %s\n", ClockStateNames[lastClockState], ClockStateNames[currentClockState]);
         lastClockState=currentClockState;
    }

    if (needsFullRedraw && currentClockState != STATE_SETTING_ALARM) 
    {
      gfx->fillScreen(BLACK);
      //delay(100);
      ShowStaticFields(&timeinfo);
      //delay(100);
      needsFullRedraw = false;
    }


    switch (currentClockState) {
      case STATE_RUNNING: //all good
      case STATE_NTP_FAILED:
        if (timeSynchronized && (currentMillis - lastNtpSyncMillis >= ntpSyncInterval)) 
        {
            resyncNTPTime();
        }
        break;
      case STATE_WIFI_FAILED:
        unsigned long currentInterval;
        if (wifiReconnectAttemptCount < NUM_WIFI_RECONNECT_INTERVALS) 
            currentInterval = WIFI_RECONNECT_INTERVALS[wifiReconnectAttemptCount];
       else
            currentInterval = WIFI_RECONNECT_INTERVALS[NUM_WIFI_RECONNECT_INTERVALS - 1];

        if (currentMillis - lastWifiReconnectAttemptMillis >= currentInterval) 
        {
            Serial.printf("Attempting WiFi reconnect (attempt %d, interval %lu ms)...\n",
                          wifiReconnectAttemptCount + 1, currentInterval);
            lastWifiReconnectAttemptMillis = currentMillis; // Update time of this attempt

            // Display message on screen
            char msgBuffer[50];
            snprintf(msgBuffer, sizeof(msgBuffer), "WiFi Reconnect (%d)", wifiReconnectAttemptCount + 1);
            displayAdditionalInfo(msgBuffer, YELLOW);
            //displayMessageScreen("Connection Lost", msgBuffer, YELLOW);

            WiFi.begin(); // Uses previously configured SSID/password if available from WiFiManager
                          // Or you might need to explicitly call WiFi.begin(saved_ssid, saved_pass);
                          // if WiFiManager doesn't automatically handle it after disconnect.

            unsigned long startConnectAttempt = millis();
            bool connected = false;
            // Try connecting for a shorter duration (e.g., 15-20 seconds)
            while (millis() - startConnectAttempt < 20000) { // 20-second timeout for this attempt
                if (WiFi.status() == WL_CONNECTED) {
                    connected = true;
                    break;
                }
                delay(500); // Check status every 500ms
                Serial.print(".");
                // You could also add a yield() here if your loop is very tight
                // yield();
            }
            Serial.println();
        }
      break;
    }

    if (currentClockState==STATE_RUNNING || currentClockState==STATE_NTP_FAILED || currentClockState==STATE_WIFI_FAILED)
    {
      // --- Update Display via Local Time Increment (Every Second) ---
      if (currentMillis - previousMillis >= 1000) {
          previousMillis = currentMillis; // Reset the 1-second tick timer

        // --- Increment Local Time ---
        time_t currentTimeSec = mktime(&timeinfo); // Convert struct tm to seconds since epoch
        int dayBeforeIncrement = timeinfo.tm_mday;
        currentTimeSec += 1;                       // Add one second
        localtime_r(&currentTimeSec, &timeinfo);   // Convert back to struct tm (thread-safe), updating global timeinfo
        if (timeinfo.tm_mday != dayBeforeIncrement) //daychange
            needsFullRedraw = true;

        displayClock(&timeinfo);
        if (isAlarmSet)
        {
          if ((timeinfo.tm_hour == alarmTime.tm_hour &&
                      timeinfo.tm_min == alarmTime.tm_min && timeinfo.tm_sec == alarmTime.tm_sec) || alarmIsActive)
                  {
                    alarmActivated();
                  }
        }
      }
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
  displayAdditionalInfo(info, WHITE);

}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);

    audio_ptr->connecttoFS(SD, alarmSound);

}

