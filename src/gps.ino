#include <Arduino.h>
#include "GPS.hpp"
#include "SIM.hpp"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "ffffffff-ffff-ffff-ffff-ffffffffffff"
#define CHARACTERISTIC_UUID "ffffffff-ffff-ffff-ffff-fffffffffffe"

SIM mysim(&Serial);

class BLESleepCallback : public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *c){
   digitalWrite(2, LOW);
   esp_deep_sleep_start();
  }
};
 
void traker(void* nth){
  unsigned long now = millis();
  char dataBuff[] = "{\"started\":\"%s\",\"lat\":%f,\"lng\":%f}";
  double lat = 0.0, lon = 0.0;
  while (true){
    while (Serial2.available()) {  
     if( millis()-now>=1000){
        now = millis();
        int msgType = processGPS();
        if ( msgType == MT_NAV_POSLLH ) {
          lat = ubxMessage.navPosllh.lat/10000000.000000f;
          lon = ubxMessage.navPosllh.lon/10000000.000000f;  
          sprintf(dataBuff,"{\"started\":\"%s\",\"lat\":%f,\"lng\":%f}","yes",lat,lon); 
        }
        mysim.POST(String("https://b-tracker-6a038-default-rtdb.firebaseio.com/locatin.json"), String(dataBuff));
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
 }
}

void setup(){
  pinMode(15, INPUT);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,HIGH);
  pinMode(2,OUTPUT);
  digitalWrite(2, HIGH);
  mysim.StratGPRS();
  Serial2.begin(9600); //for gps
  mysim.POST(String("https://b-tracker-6a038-default-rtdb.firebaseio.com/locatin.json"), String("{\"started\":\"yes\"}"));
  xTaskCreate(traker,"traking_code",1024*8,NULL,1,NULL);
  
  BLEDevice::init("B-Tracker");
  BLEServer *pServer = BLEDevice::createServer();
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new BLESleepCallback);
  pService->start();
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  
  pAdvertising->start();
}

void loop() {
  delay(100);
}
