//
//  920MHz LoRa/FSK RF module ES920LR3 
//
//  MaiaR create
//

//#define VIA_GROVE // connect to LoRa UNIT via GROVE

#ifdef ARDUINO_M5Stick_C_PLUS
#include <M5StickCPlus.h>
#ifdef VIA_GROVE
#define RX_pin 33 // LoRa UNIT via GROVE
#define TX_pin 32 // LoRa UNIT via GROVE
#else
#define RX_pin 26 // LoRa HAT
#define TX_pin 0  // LoRa HAT
#endif

#elif defined ARDUINO_M5Stick_C
#include <M5StickC.h>$$
#ifdef VIA_GROVE
#define RX_pin 33 // LoRa UNIT via GROVE
#define TX_pin 32 // LoRa UNIT via GROVE
#else
#define RX_pin 26 // LoRa HAT
#define TX_pin 0  // LoRa HAT
#endif

#elif defined ARDUINO_M5Stack_ATOMS3
#include <M5AtomS3.h>
#ifdef VIA_GROVE
#define RX_pin 1 // LoRa UNIT via GROVE
#define TX_pin 2 // LoRa UNIT via GROVE
#else
#define RX_pin 5 // LoRa KIT
#define TX_pin 6 // LoRa KIT
#define RESET_pin 7 // LoRa KIT
#define BOOT_pin 39 // LoRa KIT
#endif

#elif defined ARDUINO_M5Stack_ATOM
#include <M5Atom.h>
#ifdef VIA_GROVE
#define RX_pin 32 // LoRa UNIT via GROVE
#define TX_pin 26 // LoRa UNIT via GROVE
#else
#define RX_pin 22 // LoRa KIT
#define TX_pin 19 // LoRa KIT
#define RESET_pin 23 // LoRa KIT
#define BOOT_pin 21 // LoRa KIT
#endif

#elif defined ARDUINO_M5STACK_Core2
#include <M5Core2.h>
#ifdef VIA_GROVE
#define RX_pin 33 // LoRa UNIT via GROVE
#define TX_pin 32 // LoRa UNIT via GROVE
#else
#define RX_pin 13 // stack LoRa MODULE
#define TX_pin 14 // stack LoRa MODULE
#define RESET_pin 19 // stack LoRa MODULE
#define BOOT_pin 22 // stack LoRa MODULE
#endif

#elif defined ARDUINO_M5Stack_Core_ESP32
#include <M5Stack.h>
#ifdef VIA_GROVE
#define RX_pin 22 // LoRa UNIT via GROVE
#define TX_pin 21 // LoRa UNIT via GROVE
#else
#define RX_pin 16 // stack LoRa MODULE
#define TX_pin 17 // stack LoRa MODULE
#define RESET_pin 13 // stack LoRa MODULE
#define BOOT_pin 22 // stack LoRa MODULE
#endif
#endif


#include "LoRa.h"

#ifdef RESET_pin
void LoRaReset(){
  pinMode(BOOT_pin, OUTPUT); // BOOT "L"
  digitalWrite(BOOT_pin, LOW);
  pinMode(RESET_pin, OUTPUT); // NRST "L"
  digitalWrite(RESET_pin, LOW);
  delay(10);
  pinMode(RESET_pin, INPUT);  // NRST open
}
#else
void LoRaReset(){
  pinMode(RX_pin, OUTPUT);
  pinMode(TX_pin, OUTPUT);
  digitalWrite(RX_pin, LOW);
  digitalWrite(TX_pin, LOW);
  delay(1000);
  pinMode(RX_pin, INPUT);
  pinMode(TX_pin, INPUT);
}
#endif



void LoRaConfigMode(){
  String rx;
  Serial.begin(115200);
  while(1){
    LoRaReset();
    Serial2.begin(115200, SERIAL_8N1, RX_pin, TX_pin);
    Serial2.setTimeout(50);
    delay(50);
    rx = Serial2.readString();
    Serial.println(rx);
    if (rx.indexOf("Select Mode [") >= 0) break;
    LoRaCommand("config");
    delay(100); 
  }
}


int LoRaCommand(String s){
  String rx;
  Serial2.print(s);
  Serial2.print("\r\n");
  Serial2.flush();
  Serial.println(s);
  Serial.flush();
  delay(10);
  rx = Serial2.readString();
  Serial.print(rx);
  return (rx.indexOf("OK"));
}


int LoRaInit(){
  LoRaConfigMode();
  if (LoRaCommand("2"     ) < 0) return(-1); // processor mode
  if (LoRaCommand("x"     ) < 0) return(-1); // default
  if (LoRaCommand("c 12"  ) < 0) return(-1); // spread factor
  if (LoRaCommand("d 3"   ) < 0) return(-1); // channel
  if (LoRaCommand("e 2345") < 0) return(-1); // PAN ID
  if (LoRaCommand("f 0"   ) < 0) return(-1); // own ID
  if (LoRaCommand("g ffff") < 0) return(-1); // broadcast
  if (LoRaCommand("p 1"   ) < 0) return(-1); // RSSI on
  if (LoRaCommand("z"     ) < 0) return(-1); // start
  return(0);
}
