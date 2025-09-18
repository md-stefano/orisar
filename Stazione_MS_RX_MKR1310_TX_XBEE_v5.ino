#include <SPI.h>
#include <LoRa.h>
#include <XBee.h>

// Sistema OriSar ideato e realizzato da Stefano Mocci Demartis 2017 - tutti i diritti sono riservati - riproduzione vietata
// stazione ricezione RF24 e trasmissione XBEE E35
// V5 Maggio 2025

String versione = "OriSar V5 2025 stazione MS - ricezione MKR1310 e trasmissione XBee";

XBee xbee = XBee();
int ixbee = 6;
String carattere;
uint8_t buf[16];
uint8_t buflen = sizeof(buf);


byte localAddress = 0xFF;     // address of this device
byte destination = 0xBB;      // destination to send to

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  LoRa.setSpreadingFactor(12);
  xbee.setSerial(Serial1);
  pinMode(ixbee, OUTPUT);  // put XBee to sleep0
  digitalWrite(ixbee, HIGH);
  Serial.println(versione);
}

void loop() {
 
  if (LoRa.parsePacket()) {
    carattere = "";
    // received a packet
    // read packet
    while (LoRa.available()) {
      carattere += (char)LoRa.read();
    }
    Serial.println(carattere);
    for (int i = 0; i < buflen; i++) {
      buf[i] = char(carattere[i]);
    }

    digitalWrite(ixbee, LOW);
    XBeeAddress64 addr64 = XBeeAddress64(0x0, 0xFFFF);
    ZBTxRequest zbTx = ZBTxRequest(addr64, buf, sizeof(buf));
    xbee.send(zbTx);
    digitalWrite(ixbee, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    // conferma ricezione
    //delay(1000);
    LoRa_txMode();
    LoRa.beginPacket();
    carattere = "OK";
    LoRa.write(destination);
    LoRa.endPacket(false);
    carattere = "";
  }
}

void LoRa_rxMode(){
  LoRa.disableInvertIQ();               // normal mode
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.enableInvertIQ();                // active invert I and Q signals
}
