#define MY_NODE_ID AUTO
#define MY_PARENT_NODE_ID AUTO

//#define M25P40 // Flash type

#define Min_Batt 595
#define Max_Batt 850

const int SmokeTestInterval = 15*1000ul;    // Smole test intervak 15 sec

//====
#define SKETCH_NAME "MSMD Smoke & Motion"
#define SKETCH_MAJOR_VER "0"
#define SKETCH_MINOR_VER "1"

#define MY_DEBUG 
#define DEBUG

#define MY_RADIO_RF24
#define MY_OTA_FLASH_SS 7 // 11, PD7

#define MY_OTA_FIRMWARE_FEATURE
#define MY_SIGNING_ATSHA204

#ifdef M25P40
  #define MY_OTA_FLASH_JDECID 0x2020
#endif

#include <MySensors.h>
#include <SPI.h>
#include <avr/wdt.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Sodaq_SHT2x.h>
#include <BH1750.h>

// NODE_SENSORS
#define SensMotion   1    // S_MOTION,      V_TRIPPED
#define SensSmoke    2    // S_SMOKE,       V_ARMED
#define SensTemp     5    // S_TEMP,        V_TEMP
#define SensHum      6    // S_HUM,         V_HUM
#define SensLight    7    // S_LIGHT_LEVEL, V_LEVEL
#define SensTest     150  // S_CUSTOM,      V_VAR1

#define SensUID      200
#define SensNewID    201  // V_VAR1

// Pins
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

MyMessage msgMotion(SensMotion, V_TRIPPED);
MyMessage msgSmoke(SensSmoke, V_ARMED);
MyMessage msgTemp(SensTemp, V_TEMP);
MyMessage msgHum(SensHum, V_HUM);
MyMessage msgLight(SensLight, V_LEVEL);

void(* resetFunc) (void) = 0;

BH1750 lightMeter;

// Last values
float LastTemp = 0;
float LastHum = 0;
float LastLLevel = 0;

void before() {
  analogReference(INTERNAL);
  
  pinMode(CONNECT_PIN, INPUT_PULLUP);
  
  pinMode(BAT_PIN, INPUT); 
  pinMode(BUZ_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_S_TX, OUTPUT);
  pinMode(LED_S_RX, INPUT);
  
  pinMode(MOVE_PIN, INPUT);
  digitalWrite(MOVE_PIN, LOW);

  ADCSRA |= (1 << ADPS2 | 1 << ADPS0); //Биту ADPS2 присваиваем единицу - коэффициент
  ADCSRA &= ~ ((1 << ADPS1)); // | (1 << ADPS0)); //Битам ADPS1 и ADPS0 присваиваем нули
}

void setup() {
  Wire.begin();
  lightMeter.begin();
  
  wdt_enable(WDTO_8S);
}

void presentation(){
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER);

  present(SensUID,      S_CUSTOM,       "f5d6884d454871bf");
  present(SensNewID,    S_CUSTOM,       "Node new ID");

  present(SensMotion,   S_MOTION,       "Motion");
  present(SensSmoke,    S_SMOKE,        "Smoke");
  present(SensTemp,     S_TEMP,         "Temperature");
  present(SensHum,      S_HUM,          "Humidity");
  present(SensLight,    S_LIGHT_LEVEL,  "Light level");
  present(SensTest,     S_CUSTOM,       "Test");
}

void loop() {
  wdt_reset();

  // Test temp
  //!
  
  switch (sleep(digitalPinToInterrupt(MOVE_PIN), CHANGE, SmokeTestInterval)) {
  case 0:
    // Motion
    break;
    
  default:
     SendDevInfo();
  }
}

void SendDevInfo(){
  // Temp/Hum
  float Temp = SHT2x.GetTemperature();
  float Hum  = SHT2x.GetHumidity();

  // Light
  float lux = lightMeter.readLightLevel(); 
  
  // battery
  int BattP = constrain(map(analogRead(BAT_PIN), Min_Batt, Max_Batt, 0, 100), 0, 100);
  
  // Temp
  if (LastTemp != Temp) {
    send(msgTemp.set(Temp, 1));
    LastTemp = Temp;
    wait(1);
  }
  
  // Humidity
  if (LastHum != Hum) {
    send(msgHum.set(Hum, 0));
    LastHum = Hum;
    wait(1);
  }

  // Light
  if (LastLLevel != lux) {
    send(msgLight.set(lux, 2));
    LastLLevel = lux;
    wait(1);
  }
  
  // Battery
  sendBatteryLevel(BattP);
}
