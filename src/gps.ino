#define ESP32_RTOS  // Uncomment this line if you want to use the code with freertos only on the ESP32
#include "OTA.h"
#include "GPS.hpp"
#include "SIM.hpp"

#define mySSID "Mars"
#define myPASSWORD "counterclockwise"
SIM mysim(&Serial);
 
void traker(void* nth){
  int c = 0;
  unsigned long now = millis();
  char dataBuff[] = "{\"location\":\"http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=40.46703292882606+-79.99033157528754\",\"number of Satalites\":\"4\"}";
  double lat = 0.0, lon = 0.0;
  while (true){
    while (Serial2.available()) {  
     if( millis()-now>=1000){
        now = millis();
        int msgType = processGPS();
        if ( msgType == MT_NAV_POSLLH ) {
          lat = ubxMessage.navPosllh.lat/10000000.000000f;
          lon = ubxMessage.navPosllh.lon/10000000.000000f;  
          sprintf(dataBuff,"{\"location\":\"http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=%0.9f+%0.9f\",\"number of Satalites\":\"%f\"}",lat,lon,int(ubxMessage.navStatus.gpsFix)); 
          //sprintf(dataBuff,"{\"location\":\"http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=40.46703292882606+-79.99033157528754\",\"number of Satalites\":\"4\"}"); 
        }
        mysim.POST(String("https://b-tracker-6a038-default-rtdb.firebaseio.com/locatin.json"), String(dataBuff));
        c++;
        TelnetStream.println("posted some data");
        if (c > 4){
          digitalWrite(2, LOW);
          esp_deep_sleep_start();
        }
      }
    } 
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ArduinoOTA.handle();
    //TelnetStream.println("In tracker ...");
 }
}

void setup(){
  pinMode(15, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,HIGH);
  pinMode(2,OUTPUT);
  digitalWrite(2, HIGH);
  TelnetStream.println("Code Started");
  setupOTA("Alis", mySSID, myPASSWORD);
  mysim.StratGPRS();
  Serial2.begin(9600); //for gps
  mysim.POST(String("https://b-tracker-6a038-default-rtdb.firebaseio.com/locatin.json"), String("{\"started\":\"yes\"}"));
  xTaskCreate(traker,"traking_code",1024*8,NULL,1,NULL);
  TelnetStream.println("Leaving Setup!");
  
}

volatile int shake = 0;
void loop() {
  ArduinoOTA.handle(); 
  delay(100);
}
