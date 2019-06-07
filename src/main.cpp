#define _DEBUG_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ThingerESP8266.h>
#include <ThingerSmartConfig.h>

#define usuario "mhuertas"
#define device_Id "ProMeteo"
#define device_credentials "7no8ne10cs&K"

const char WiFi_ssid[]="AndroidAP7b99";
const char WiFi_password[]="dcei8501";

ThingerESP8266 thing(usuario, device_Id, device_credentials);

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  while(!Serial) {}

  thing.add_wifi(WiFi_ssid, WiFi_password);

  thing["Meteo"] >> [](pson& out){
    out["Temperatura"] =0;
    out["Humedad"] =0;
    out["Pressure"] =0;
    out["Altitude"] =0;
  };

  thing.handle();
  thing.write_bucket("Meteo", "Meteo");

  Serial.println("Going to deep sleep");
  ESP.deepSleep(1000000 * 30, WAKE_RF_DEFAULT); // 60 seconds
}

void loop() { 
}