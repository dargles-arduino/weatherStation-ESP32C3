/**
 * Program: weatherStation-ESP32C3
 * Purpose:
 *   This code sets up a weather station. It uses deep sleep to save power. On waking, it checks 
 *   the battery, takes readings from a BME280 or BMP280+AHT20, connects to the local wireless, 
 *   and submits the results to a webserver presumed to be waiting for a connection. It then 
 *   goes back to sleep for an hour. Some notes on this version:
 *   - There's some legacy code that I'm loathe to delete. This includes code to put the chip 
 *     back to sleep immediately after wake-up if the battery voltage is too low. We now rely  
 *     on a hardware battery protection circuit, but the check is still there, in case.
 *   - Theres a sound channel on GPIO5, not currently used.
 *   - This version (weatherStation-ESP32C3) is designed to work especially with that chip. It 
 *     has a very low deep sleep current by design (43uA claimed), which is significantly less 
 *     that the ESP8266 versions on the market, and a match for home-made 8266 versions based 
 *     on the 12F.
 *   - For the ESP32C3, there appears to be a clash between the Serial I/O lines and the I2C 
 *     lines. We need I2C for the weather sensor(s) so runtime feedback will have to be via an 
 *     OLED screen, hence the need to add in OLED code.
 *   - We use info.h to define SSID and PASSWORD, while not showing it on GitHUb ;) 
 *     So, if you're downloading this from GitHub, you need to add an info.h with: 
 *       #define LOCAL_SSID <your ssid>
 *       #define LOCAL_PWD <your password>
 *     Or if you prefer, you could just include these two lines in the code below and remove 
 *     the #include "info.h"...
 *   - I allow for multiple weather stations on different "channels". The channel is set in 
 *     hardware, so the software now has to read hardware links to determine which channel to   
 *     report to.
 *   For more info, see the GitHub repo.
 * @author: David Argles, d.argles@gmx.com
 */

/* Program identification */ 
#define PROG    "weatherStation-ESP32C3"
#define VER     "1.00"
#define BUILD   "22oct2023 @ 17:32h"

// Set trace to be false if you don't want diagnostic output. You can't have
// both Serial o/p and OLED connectivity at the same time. Choose EITHER:
#define serialTrace false  // requires Xiao_ESP32C3 board def
#define oledTrace   true   // requires ESP32C3 Dev Module board def

/* Necessary includes */
#include "flashscreen.h"
#include "OLEDscreen.h"
// #include "info.h"       // Allows definition of SSID and password without it showing on GitHub ;)
/* These includes are for the BME280 sensor */
// #include <Wire.h>
// #include <BMx280I2C.h>
// #include <AHT20.h>
/* These are for the WiFi & webclient */
// #include <ESP8266WiFi.h>
// #include <ESP8266WiFiMulti.h>
// #include <ESP8266HTTPClient.h>

/* Global "defines" - some may have to look like variables because of type */
// The following are pin definitions
#define ADC_0                 0   // Monitors battery health
#define ADC_1                 1   // Plan is for this to monitor light levels
#define SOUND_OUT             3   // Not currently used
#define LED                   5   // For the LED
#define CONFIG_PIN1          21   // =msb; 0 = car, 1 = weather
#define CONFIG_PIN2          20   //       0 = in,  1 = out
#define CONFIG_PIN3          10   // =lsb; 0 = 1,   1 = 2
// Cutoff was used before hardware battery protection was intoduced, now is legacy code
#define CUTOFF                0   // 6V0 = 738. For 18650, we want 3V -> 369
// The following are I2C address definitions
#define SENSOR_ADDRESS      0x76  // Defines the expected I2C address (0x76) for the BMx280...
#define ALT_I2C_ADDRESS     0x77  // - but is 0x77 on combined AHT20/BMP280
// The following are error code definitions
#define ERROR_LOWBAT           1
#define ERROR_NO_BMx           2
#define ERROR_NO_WIFI          4
#define ERROR_NO_BMx_READING   8
#define ERROR_NO_WEB_UPLOAD   16

/* ----- Initialisation ------------------------------------------------- */

/* Global stuff that must happen outside setup() */
RTC_DATA_ATTR int readingNo = 0;            // RTC data survives deep sleep; readingNo numbers the readings
RTC_DATA_ATTR int error     = 0;            // error is the error# this run, available at startup after deep sleep
OLEDscreen        screen;                   // Creates the screen (OLED) object
//BMx280I2C         bmx280(SENSOR_ADDRESS);  // Creates a BMx280I2C object using I2C
//AHT20             aht20;                // Creates AHT20 sensor object
//ESP8266WiFiMulti  WiFiMulti;            // Creates a WiFiMulti object
int pin1;
int pin2;
int pin3;
String channel    = "";
String configValues[]   = {"test", "mx5", "test", "test", "in1", "in2", "bat", "out2"};

void setup() {
  // initialise objects
  flashscreen flash;
  screen.begin();
  
  // declare variables
  long int  baudrate  = 115200;     // Baudrate for serial output
  int       prevError;              // error code for the previous run
  boolean   batteryOK = false;      // Checks whether our battery has sufficient charge
  uint64_t  deepSleepTime = 60e6; // Deep sleep delay (millionths of sec): 3600e6=1h, 300e6=5m, 60e6=1m
  bool      BMElive = false;        // records whether BME280 initialised properly
  int       adc  = 0;               // records battery voltage
  int       light = 0;              // records light level
  float     temp = 0;               // records: temperature...
  float     pres = 0;               // ...pressure...
  float     hum  = 0;               // ...and humidity
  String    readings = "";          // For the parameter string in the http upload
  boolean   successful = false;     // If we fail, let's record the value and try next time
  String    urlRequest;             // String for contacting server
  pinMode(LED, OUTPUT);             // Only use LED in case of error; but initialise it now
  digitalWrite(LED, LOW);           // Turn off LED; it seems to come on by default
  pinMode(SOUND_OUT, OUTPUT);       // For sound o/p if required
  pinMode(CONFIG_PIN1, INPUT_PULLUP); // {set up the configure pins to be input_pullup,
  pinMode(CONFIG_PIN2, INPUT_PULLUP); // {this will make them inverse logic
  pinMode(CONFIG_PIN3, INPUT_PULLUP);

  // Start up the serial output port
  if(serialTrace) Serial.begin(baudrate);
  // Serial.setDebugOutput(true);

  // Send program details to output device
  if(serialTrace) flash.message(PROG, VER, BUILD);
  if(oledTrace) screen.banner(PROG, VER, BUILD);
  
  // Set up the ESP deep sleep time
  esp_sleep_enable_timer_wakeup(deepSleepTime);
  String message = "Deep sleep time set  to " + String(deepSleepTime/60e6) + " minutes\n";
  if(serialTrace) Serial.print(message);
  if(oledTrace) screen.print(message);

  // Set up the reading no for this set of readings
  readingNo++;
  message = "Starting run " + String(readingNo) + "\n";
  if(serialTrace) Serial.print(message);
  if(oledTrace){
    delay(2000);
    screen.clear();
    screen.print(message);
  }
  // Remember the error no for the previous run and reset for this run
  prevError = error;
  error = 0;

  // Check the ADC to see what the battery voltage is
  if(serialTrace) Serial.println("Checking battery...");
  if(oledTrace) screen.print("Checking battery...\n");
  batteryOK = false;
  adc = analogRead(ADC_0);
  message = "ADC reading: " + String(adc) + "\n";
  if(serialTrace) Serial.print(message);
  if(oledTrace) screen.print(message);
  if(adc<CUTOFF) error += ERROR_LOWBAT;
  else batteryOK = true;
  if(batteryOK){
    if(serialTrace) Serial.println("Battery OK");
    if(oledTrace) screen.print("Battery OK\n");
    // Determine which channel we're using
    channel = readChannel();
    message = "Channel read as: " + String(channel) + "\n";
    if(serialTrace) Serial.print(message);
    if(oledTrace) screen.print(message);

    // Get the sensor(s) going
    if(serialTrace) Serial.println("Initialising sensor...");
    if(oledTrace) screen.print("Initialising sensor\n");

    /* Now initialise the BME280 */
    //Wire.begin();
    /* begin() checks the Interface, reads the sensor ID (to differentiate between 
     BMP280 and BME280) and reads compensation parameters.*/\
    /*int attempts = 5;
    while(!bmx280.begin() && (attempts>0))
    {
      Serial.print("Attempt ");
      Serial.print(6-attempts--);
      Serial.println(": begin() failed. Trying again."); //check your BMx280 Interface and I2C Address.");
      delay(1000);
    }
    if(attempts<1) 
    {
      // Sensor initialisation failed
      Serial.println("begin() failed. Check your BMx280 Interface and I2C Address.");
      temp = -1;
      pres = -1;
      hum  = -1;
      error += ERROR_NO_BMx; // error #2 means we failed to connect to the weather sensor 
    }
    else
    {
      // Sensor initialised; set up sensor parameters
      BMElive = true;
      if (bmx280.isBME280()) Serial.println("sensor is a BME280");
      else Serial.println("sensor is a BMP280");
      //reset sensor to default parameters.
      bmx280.resetToDefaults();
      //by default sensing is disabled and must be enabled by setting a non-zero
      //oversampling setting.
      //set an oversampling setting for pressure and temperature measurements. 
      bmx280.writeOversamplingPressure(BMx280MI::OSRS_P_x16);
      bmx280.writeOversamplingTemperature(BMx280MI::OSRS_T_x16);
      //if sensor is a BME280, set an oversampling setting for humidity measurements.
      if (bmx280.isBME280()) bmx280.writeOversamplingHumidity(BMx280MI::OSRS_H_x16);
    }*/

    // If we're up and running get the sensor readings (ADC has already been recorded)
    /*if(BMElive)
    {
      //start a measurement
      if (!bmx280.measure()){
        Serial.println("could not start measurement, is a measurement already running?");
        error += ERROR_NO_BMx_READING; // Error #8 means although the sensor initialised, we couldn't get a reading
      }
      else
      {
        //wait for the measurement to finish
        delay(1000);
        if(bmx280.hasValue())
        {
          temp = bmx280.getTemperature();
          pres = bmx280.getPressure()/100;
          if(bmx280.isBME280()) hum = bmx280.getHumidity();
        }
      }
    }*/

    /* See if we have an AHT20 */
    // We've already got Wire going => Wire.begin(); //Join I2C bus
    //Check if the AHT20 will acknowledge
    /*if (aht20.begin() == false) Serial.println("AHT20 not detected.");
    else
    {
      Serial.println("AHT20 detected.");
      temp = aht20.getTemperature();
      hum = aht20.getHumidity();
    }*/

    // Start the sound
    //analogWrite(SOUND_OUT, TONE);

    // Set up the parameter string for the web exchange
    /*int prevError = store.error();
    Serial.print("Previous error # was: ");
    Serial.println(prevError);
    readings = "?"+String(channel)+"="+String(adc)+"&serialNo="+String(serialNo)+"&temp="+String(temp)+"&pres="+String(pres)+"&hum="+String(hum)+"&error="+String(store.error());
    Serial.print("Param String is: ");
    Serial.println(readings);
    Serial.print("Current error is: ");
    Serial.println(error);*/

    // Register WiFi extender
    /*WiFiMulti.addAP(SSID_2, PWD_2);
    // Now start up the wifi and attempt to submit the data
    wifiConnect();
    //Serial.print("Connecting to WiFi");
    // Check for WiFi connection 
    successful = false;
    if(WiFiMulti.run() == WL_CONNECTED)
    {
      Serial.print("WiFi connected: ");
      Serial.println(WiFi.SSID());
      WiFiClient client;
      HTTPClient http; // Must be declared after WiFiClient for correct destruction order, because used by http.begin(client,...)
      //trace("\n[HTTP]", "");

      urlRequest = URL_W; //"http://argles.org.uk/homelog.php";
      urlRequest += readings;
      // trace("empty = ", String(adcStore.empty));   
      // Now make the request
      String message = "Requesting: " + urlRequest;
      Serial.println(message);
      http.begin(client, urlRequest);
      // start connection and send HTTP header
      int httpCode = http.GET();
      
      // If the www URL failed, try the direct IP address
      if(httpCode != HTTP_CODE_OK)
      {
        Serial.println("URL request failed, trying IP address.");
        // Set up request url with reading parameter(s)
        urlRequest = URL_D + readings; //"http://192.168.1.76";
        http.begin(client, urlRequest);
        httpCode = http.GET();
        http.end();        
      }
      
      if (httpCode > 0) 
      {
        // HTTP header has been sent and Server response header has been handled
        // trace("Return code: ", String(httpCode));
        // file found at server
        if (httpCode == HTTP_CODE_OK) 
        {
          // Serial.println(http.getString());  // Gets the actual page returned
          Serial.println("Request successful");
          //trace("Connection closed", "");
          successful = true;
        }
      }
      // HTTP request failed 
      else{
        error += ERROR_NO_WEB_UPLOAD;  // error #16 means the upload to the server failed
        Serial.printf("Request failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
      http.end();
    }*/
    // If no WiFi...
    //else trace("WiFi failed, code: ", String(WiFiMulti.run()));
  }

  /*if(error!=0){
    Serial.print("Error #");
    Serial.print(error);
    Serial.println(" occured");
    blink(error);
  }
  // Save the error code
  store.setError(error);
  Serial.print("Error set to: ");
  Serial.println(store.error());
  // write the data back to rtc memory
  store.writeData();
  // Turn off the sound
  analogWrite(SOUND_OUT, 0);*/
  if(serialTrace)
  {
    Serial.println("Going to sleep...");
    //Serial.flush();
  }
 if(oledTrace) screen.print("zzz...");

  // Whether successful or not, we're going to sleep for an hour before trying again!
  esp_deep_sleep_start();
}

void blink(int code){
  for (int count=0;count<code;count++){
    digitalWrite(LED, !digitalRead(LED));
    delay(500);
  }
}
/*
void wifiConnect()
{
  Serial.println("Connecting to Wifi...");
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(LOCAL_SSID, LOCAL_PWD);*/
  /* NOTE: the example sketch has this wrong. We need to wait -after- doing
           the WiFiMulti.addAP to give it time to register! */
/*  Serial.print("[SETUP]\nWAIT ");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("%d...", t);
    Serial.flush();
    delay(1500);
  }
  Serial.println();

  int attempts = 5;
  
  while(WiFiMulti.run() != WL_CONNECTED && (attempts-- >0))
  {
    Serial.println("WiFi not connected");
    delay(1000);
  }
  
  if (WiFiMulti.run() == WL_CONNECTED) 
  {
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else{
    error += ERROR_NO_WIFI; // error #4 means we couldn't connect to WiFi
    Serial.println("WiFi connection failed!");
  }
  return;
}
*/
/*boolean checkBattery(){
  adc = analogRead(ADC_0);
  if(adc<=CUTOFF) store.setError(1);
  return (adc>CUTOFF);
}*/ 

void loop() {
  // If we get here, something's gone wrong! Let's flash a warning
  digitalWrite(LED, !digitalRead(LED));
  delay(1000);
}

String readChannel(void)
{
  String value = "";
  // Remember the config pins are reverse logic
  pin1 = 1 - digitalRead(CONFIG_PIN1);
  pin2 = 1 - digitalRead(CONFIG_PIN2);
  pin3 = 1 - digitalRead(CONFIG_PIN3);
  if(serialTrace) Serial.printf("pin1: %i, pin2: %i. pin3: %i\n", pin1, pin2, pin3);
  int configVal = (4*pin1) + (2*pin2) + (pin3);
  if(serialTrace) Serial.printf("configVal: %i\n", configVal);
  value = configValues[configVal];
  return (value);
}
