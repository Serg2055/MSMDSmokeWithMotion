// #define MY_SIGNING_ATSHA204_PIN  17 // A3 - 26, PC3 (Default)
// #define MY_RF24_CE_PIN 9 // 13, PB1 (Default)
#define MY_OTA_FLASH_SS 7 // 11, PD7

//#define M25P40 // Flash type

// No change
#define MY_DEBUG

#define MY_OTA_FIRMWARE_FEATURE

#define MY_RADIO_NRF24
#define MY_CORE_ONLY
#define MY_SIGNING_ATSHA204

#ifdef M25P40
  #define MY_OTA_FLASH_JDECID 0x2020
#endif

#include <SPI.h>
#include <MySensors.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <BH1750.h>

#define CONNECT_PIN  3  // 1,  PDB3
#define BAT_PIN      A0 // 23, ADC9
#define BUZ_PIN      5  // 9,  PD5
#define LED_PIN      6  // 10, PB6
#define LED_S_TX     4  // 2,  PD4
#define LED_S_RX     A1 // 24, PC1
#define MOVE_PIN     2  // 32, PD2 
// i2C - SHT20, BH1750

#define cSmokeOffLevel  0
#define cSmokeOnLevel   0

#ifdef M25P40
  #define SPIFLASH_BLOCKERASE_32K   0xD8
#endif

BH1750 lightMeter;

bool testSha204(){
  uint8_t rx_buffer[SHA204_RSP_SIZE_MAX];
  uint8_t ret_code;
  if (Serial) {
    Serial.print("SHA204: ");
  }
  atsha204_init(MY_SIGNING_ATSHA204_PIN);
  ret_code = atsha204_wakeup(rx_buffer);

  if (ret_code == SHA204_SUCCESS) {
    ret_code = atsha204_getSerialNumber(rx_buffer);
    if (ret_code != SHA204_SUCCESS) {
      if (Serial) {
        Serial.println(F("Failed to obtain device serial number. Response: "));
      }
      Serial.println(ret_code, HEX);
    } else {
      if (Serial) {
        Serial.print(F("OK (serial : "));
        for (int i = 0; i < 9; i++) {
          if (rx_buffer[i] < 0x10) {
            Serial.print('0'); // Because Serial.print does not 0-pad HEX
          }
          Serial.print(rx_buffer[i], HEX);
        }
        Serial.println(")");
      }
      return true;
    }
  } else {
    if (Serial) {
      Serial.println(F("Failed to wakeup SHA204"));
    }
  }
  return false;
} 

void setup() {
  int BatVal = 0;
  
  Wire.begin();
  
  Serial.begin(115200);
  Serial.println(F(MYSENSORS_LIBRARY_VERSION "\n")); 

  // Clear eeprom
  for (int i=0;i<512;i++) {
    EEPROM.write(i, 0xff);
  }

  // Setup pins
  pinMode(CONNECT_PIN, INPUT_PULLUP);
  pinMode(BAT_PIN, INPUT); 
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_S_TX, OUTPUT);
  pinMode(LED_S_RX, INPUT);
  pinMode(MOVE_PIN, INPUT_PULLUP);
  
  // CPU
  Serial.println("Test CPU: OK");

  // NRF24
  if (transportInit()) { // transportSanityCheck
    Serial.println("Radio: OK");
  } else {
    Serial.println("Radio: ERROR");
  }

  // Flash
  if (_flash.initialize()) {
    Serial.println("Flash: OK");
  } else {    
    Serial.println("Flash: ERROR ");
  }

  // ATASHA
  testSha204();

  // Test smoke without lithing
  int fSmoke = analogRead(LED_S_RX);
  if (fSmoke < cSmokeOffLevel) {
    Serial.print("Smoke off: Error - ");
    Serial.println(fSmoke); 
  } else {
    Serial.println("Smoke off: ok");
  }

  // Test smoke with light
  digitalWrite(LED_S_TX, HIGH);
  delay(200);
  fSmoke = analogRead(LED_S_RX);
  if (fSmoke < cSmokeOnLevel) {
    Serial.print("Smoke on: Error - ");
    Serial.println(fSmoke); 
  } else {
    Serial.println("Smoke on: ok");
  }

  // SHT20
  if (SHT2x.GetHumidity()==0){
    Serial.println("STH: Error");
  } else {
    Serial.println("STH: OK");
  }

  // BH1750
  lightMeter.begin();

  bool fFlag;
  while (true) {
    unsigned long currentMillis = millis();     
    
    // Battery
    BatVal = analogRead(BAT_PIN);
    //TODO: Check value
    Serial.print("Battery: ");
    Serial.println(BatVal);

    // Button
    Serial.print("Connect: ");
    Serial.println(digitalRead(CONNECT_PIN));
    
    digitalWrite(BUZ_PIN, fFlag); // Buzz
    digitalWrite(LED_PIN, fFlag); // Led

    // Motion
    Serial.print("Motion: ");
    Serial.println(digitalRead(MOVE_PIN));

    // SHT20
    Serial.print("SHT (%RH): ");
    Serial.print(SHT2x.GetHumidity());
    Serial.print("     SHT (C): ");
    Serial.println(SHT2x.GetTemperature());

    // BH1750
    uint16_t lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    
    fFlag = !fFlag;
    delay(1000);
  }

}

void loop() {

}
