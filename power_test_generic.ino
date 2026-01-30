/*
  power-test
  Basic sketch to do minimal functionality to enable testing of power consumption

  (c) 2026 John Mueller / MIT License
  https://github.com/softplus/esp32-power-test
*/

/*
  1  = empty loop
  2  = delay 1s loop (does delay affect power?)
  3  = empty loop with SIG enabled as output, not set
  4  = empty loop with SIG enabled and set HIGH
  5  = empty loop with SIG enabled and set LOW
  6  = delay 1s loop, alternating SIG (does pin toggling affect power?)
  7  = delay 1s loop, enable Serial 9600, print "." (does Serial affect power?)
  17 = wait 3s, power down all pins, delay 1s loop (do pins affect power?)
  16 = wait 3s, 160MHz CPU, delay 1s loop
  8  = wait 3s, 80MHz CPU, delay 1s loop.
  9  = wait 3s, 40MHz CPU, delay 1s loop.
  10 = wait 3s, 20MHz CPU, delay 1s loop.
  11 = wait 3s, 10MHz CPU, delay 1s loop.
  12 = wait 3s, light sleep for 15s, empty loop
  13 = wait 3s, deep sleep for 15s, empty loop
  14 = wait 3s, disable stuff, deep sleep for 15s, empty loop
  15 = wait 3s, 10MHz, pause, deep sleep for 15s, empty loop (does speed affect deep sleep power?)
  18 = wait 3s, disable stuff, power down pins, deep sleep for 15s, empty loop
*/
#define TEST 1

#define SIGNAL_PIN 8
#define ENABLE_OUTPUT (TEST==3 || TEST==4 || TEST==5 || TEST==6)
#define LOOP_DELAY (TEST==2 || TEST==6 || TEST==7 || TEST==8 || TEST==9 || TEST==10 || TEST==11 || TEST==16 || TEST==17)
#define START_3DELAY (TEST>7)
#define PINS_OFF (TEST==17 || TEST==18)
#define MS_TO_US 1000ULL

#include "driver/gpio.h" // for disabling output pins for power testing
#include "soc/soc_caps.h" // Provides SOC_GPIO_PIN_COUNT

// the setup function runs once when you press reset or power the board
void setup() {
  if (ENABLE_OUTPUT) {
    pinMode(SIGNAL_PIN, OUTPUT);
    if (TEST==4) digitalWrite(SIGNAL_PIN, HIGH);
    if (TEST==5) digitalWrite(SIGNAL_PIN, LOW);
  }
  if (START_3DELAY) delay(3000); // makes it easier to reprogram with sleep modes
  if (TEST==7) {
    Serial.begin(9600); // open the serial port at 9600 bps:
    Serial.println("ok");
    Serial.flush();
  }
  if (TEST==16) setCpuFrequencyMhz(160);
  if (TEST==8) setCpuFrequencyMhz(80);
  if (TEST==9) setCpuFrequencyMhz(40);
  if (TEST==10) setCpuFrequencyMhz(20);
  if (TEST==11) setCpuFrequencyMhz(10);
  if (PINS_OFF) silenceAllPinsGeneric();
  if (TEST==12) {
    esp_sleep_enable_timer_wakeup(15000 * MS_TO_US); // sleep 15s
    esp_light_sleep_start();
  }
  if (TEST==13) {
    esp_sleep_enable_timer_wakeup(15000 * MS_TO_US); // sleep 15s
    esp_deep_sleep_start();
  }
  if (TEST==14||TEST==18) {
    esp_sleep_enable_timer_wakeup(15000 * MS_TO_US); // sleep 15s
    fancyDeepSleepSetup();
    esp_deep_sleep_start();
  }
  if (TEST==15) {
    setCpuFrequencyMhz(10);
    delay(100); // confirm speed change
    esp_sleep_enable_timer_wakeup(15000 * MS_TO_US); // sleep 15s
    esp_deep_sleep_start();
  }
}

// the loop function runs over and over again forever
void loop() {
  if (LOOP_DELAY) {
    delay(1000);
  }
  if (TEST==6) {
    static int state = 0;
    state = 1-state;
    digitalWrite(SIGNAL_PIN, state?HIGH:LOW);
  }
  if (TEST==7) {
    Serial.print(".");
  }
}

/**
 * Disables various potentially power-consuming parts of the MCU
 */
void fancyDeepSleepSetup() {
    // Basic Power Domains (Available on ~95% of chips)
    #ifdef ESP_PD_DOMAIN_RTC_PERIPH
      esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
    #endif
    
    #ifdef ESP_PD_DOMAIN_CPU
      esp_sleep_pd_config(ESP_PD_DOMAIN_CPU, ESP_PD_OPTION_OFF);
    #endif

    // High-Speed Crystal (Found on S-Series and Original)
    #ifdef ESP_PD_DOMAIN_XTAL
      esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);
    #endif

    // Modern "Top" Domain (Found on C6, H2)
    #ifdef ESP_PD_DOMAIN_TOP
      esp_sleep_pd_config(ESP_PD_DOMAIN_TOP, ESP_PD_OPTION_OFF);
    #endif

    // Flash Power (Critical for most)
    #ifdef ESP_PD_DOMAIN_VDDSDIO
      esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_OFF);
    #endif

    // ONLY use these if you don't need RTC memory/fast wakeup
    #ifdef ESP_PD_DOMAIN_RTC_SLOW_MEM
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
    #endif
    #ifdef ESP_PD_DOMAIN_RTC_FAST_MEM
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
    #endif
    
    // Lock the "Silenced" pin states so they don't float during sleep
    if (TEST==18) {
      #if !CONFIG_IDF_TARGET_ESP32C6 && !CONFIG_IDF_TARGET_ESP32H2
        gpio_deep_sleep_hold_en();
      #endif
    }

    esp_sleep_config_gpio_isolate();
}

/**
 * Sets all safe GPIO pins to INPUT_PULLDOWN to minimize leakage current.
 * Avoids flash, USB, and system-critical pins per chip variant.
 */
void silenceAllPinsGeneric() {
  // Define only the pins that are actually broken out on your dev board 
  // and NOT used for system/flash.
  
  #if CONFIG_IDF_TARGET_ESP32S3
    const int safePins[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 15}; // Flash Pins (18-23), USB 12, 13
  #elif CONFIG_IDF_TARGET_ESP32C6
    const int safePins[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 15}; // Flash Pins (18-23), USB 12, 13
  #elif CONFIG_IDF_TARGET_ESP32C3
    const int safePins[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11}; // USB Pins
  #elif CONFIG_IDF_TARGET_ESP32
    const int safePins[] = {0, 1, 2, 3, 4, 5, 12, 13, 15}; // Original ESP32 Flash (6-11)
  #else
    const int safePins[] = {0, 1, 2, 3, 4, 5, 6, 7, 10, 11, 15};
  #endif

  for (int i = 0; i < (sizeof(safePins) / sizeof(safePins[0])); i++) {
    pinMode(safePins[i], INPUT_PULLDOWN); // Set to INPUT_PULLDOWN for lowest leakage
  }
}
