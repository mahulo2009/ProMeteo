#define _DEBUG_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include <ThingerSmartConfig.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

#define usuario "mhuertas"
#define device_Id "ProMeteo"
#define device_credentials "7no8ne10cs&K"
#define SEALEVELPRESSURE_HPA (1013.25)

const char WiFi_ssid[]="MOVISTAR_D659";
const char WiFi_password[]="gQK9NJ6amoPbTMfcqz67";

ThingerESP8266 thing(usuario, device_Id, device_credentials);

Adafruit_BME280 bme;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(2000);

  while(!Serial) {}

  Wire.pins(D3, D2);
  Wire.begin(D3, D2); 
  bme.begin(0x76);
  lcd.begin(D3,D2);
  lcd.noBacklight();
  lcd.noDisplay();
 
  thing.add_wifi(WiFi_ssid, WiFi_password);

  thing["Meteo"] >> [](pson& out){
    out["Temperatura"] = bme.readTemperature();
    out["Humedad"] = bme.readHumidity();
    out["Pressure"] = bme.readPressure() / 100.0F;
    out["Altitude"] = bme.readAltitude(SEALEVELPRESSURE_HPA);
  };

  thing.handle();
  thing.write_bucket("Meteo", "Meteo");

  Serial.println("Going to deep sleep");
  ESP.deepSleep(1000000 * 30, WAKE_RF_DEFAULT); // 60 seconds
}

void debug() {

  Serial.print(" temperature\t");
  Serial.print(bme.readTemperature());
  Serial.print(" humidity\t");
  Serial.print(bme.readHumidity());
  Serial.print(" pressure\t");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.print(" altitude\t");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.print("\n");

}

void loop() { 
  //debug();
  //delay(1000);
}