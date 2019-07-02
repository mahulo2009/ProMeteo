#define _DEBUG_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include <ThingerSmartConfig.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

#define USER                "***"
#define DEVICE_ID           "***"
#define DEVICE_CREDENTIALS  "***"

#define BUCKET_ID           "***"
#define RESOURCE_NAME       "***"

#define SDA_PIN             D3
#define SCL_PIN             D2

#define BME_ID              0x76
#define SEALEVELPRESSURE    (1013.25)

#define MODE_DEEP_SLEEP     0
#define MODE_LCD_DISPLAY    1

#define DEEP_SLEEP_SECONDS  10
#define AWAKE_SLEEP_SECONDS 10

const char WIFI_SSID[]     = "***";
const char WIFI_PASSWORD[] = "***";

/**
 * Running mode
 * 
 * - Deep Sleep to save battery
 * - Lcd dipslay to show data on display
 */
int mode = MODE_DEEP_SLEEP;

/**
 * A variable to decide when to go to deep sleep again
 */
int counter = 0;

/**
 * BM3280 sensor: temperature, humidity, pressure and altitude
 */
Adafruit_BME280       bme;    

/**
 * Display: 16 columns x 2 rows
 * 
 *  Parameters:
 *    - @param[in] I2C id.
 *    - @param[in] Number of columns
 *    - @param[in] Number of rows
 */
LiquidCrystal_I2C     lcd(0x27, 16, 2);

/**
 * ThingerIO API 
 * 
 *  Parameters:
 *    -@param[in] USER user name, previouly craated in thinger.io account
 *    -@param[in] DEVICE_ID, the device id in the thinger.io plataform to associte this 
 *    physical device.
 *    -@param[in] DEVICE_CREDENTIALS, the device credentials proviced by the thinger.io device
 *    configuration 
 */
ThingerESP8266        thing(USER, DEVICE_ID, DEVICE_CREDENTIALS);

/**
 * Setup method, it is called every time the microcontroller is switched on
 */
void setup() {

  //Configure Serial port for debugging proposses
  Serial.begin(115200);
  Serial.setTimeout(2000);
  //Wait for the Serial port to be ready
  while(!Serial) {}

  //Configure the I2C pins
  Wire.pins (SDA_PIN, SCL_PIN);
  Wire.begin(SDA_PIN, SCL_PIN); 

  //Start communication with the BME sensor
  bme.begin(BME_ID);

  //Connect thinger.io API to access point
  thing.add_wifi(WIFI_SSID, WIFI_PASSWORD);

  //Create the lambda function to read values
  thing["Meteo"] >> [](pson& out){
    out["Temperatura"]  = bme.readTemperature();
    out["Humedad"]      = bme.readHumidity();
    out["Pressure"]     = bme.readPressure() / 100.0F;
    out["Altitude"]     = bme.readAltitude(SEALEVELPRESSURE); 
  };

  //Discriminate if we are awake from deep sleep
  //or from user reset  
  rst_info * rst_info_var = ESP.getResetInfoPtr();
  if (rst_info_var->reason==REASON_DEEP_SLEEP_AWAKE) {
    mode = MODE_DEEP_SLEEP;
    //Write the values to bucket
    thing.handle();
    thing.write_bucket(BUCKET_ID, RESOURCE_NAME);
    //Goes to deep sleep again
    Serial.println("Going to deep sleep");
    ESP.deepSleep(1000000 * DEEP_SLEEP_SECONDS, WAKE_RF_DEFAULT); 
  } else {
    mode = MODE_LCD_DISPLAY;
    //Start communication with the LCD Display sensor
    lcd.begin(SDA_PIN,SCL_PIN);
    lcd.backlight();
    lcd.display();    
  }  

}

/**
* Diplay pattern:

*     0123456789012345
*     XXXXXXXXXXXXXXXX
*     Meteo Station
*     25Cº 100% 1404mb
*     XXXXXXXXXXXXXXXX
*     0123456789012345
*/
void displayLCD() {

  lcd.setCursor(1,0);
  lcd.print("Meteo Station");
  lcd.setCursor(0,1);
  lcd.print((int)bme.readTemperature());
  lcd.setCursor(2,1);
  lcd.print("C\xDF");
  lcd.setCursor(6,1);
  lcd.print((int)bme.readHumidity());
  lcd.setCursor(8,1);
  lcd.print("%");
  lcd.setCursor(10,1);
  lcd.print((int)(bme.readPressure() / 100.0F));
  lcd.setCursor(14,1);
  lcd.print("mb");

}

void loop() {

  if ( mode == MODE_LCD_DISPLAY) {
    displayLCD();
    
    //Write the values to bucket
    thing.handle();
    thing.write_bucket(BUCKET_ID, RESOURCE_NAME);

    //Delay 1 second
    delay(1000);

    if (counter++ == AWAKE_SLEEP_SECONDS) {
      mode = MODE_DEEP_SLEEP;

      //Disable LCD to save battery
      lcd.noBacklight();
      lcd.noDisplay();
      //Goes to deep sleep again
      Serial.println("Going to deep sleep");
      ESP.deepSleep(1000000 * DEEP_SLEEP_SECONDS, WAKE_RF_DEFAULT); 
    }
  }

}