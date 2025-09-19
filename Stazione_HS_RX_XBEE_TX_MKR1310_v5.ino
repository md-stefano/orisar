#include <SPI.h>
#include <LoRa.h>
#include <XBee.h>

// Sistema OriSar ideato e realizzato da Stefano Mocci Demartis 2017 - tutti i diritti sono riservati - riproduzione vietata
// V5 Maggio 2025

String versione = "OriSar V5 2025 stazione HS - ricezione XBee e Trasmissione MKR1310";

XBee xbee = XBee();
int ixbee = 6;
String carattere;
uint8_t buf[16];
uint8_t buflen = sizeof(buf);
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();


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
  String sample;
  xbee.readPacket();
  if (xbee.getResponse().isAvailable())
  {
    //Serial.println(xbee.getResponse().getApiId());
    if (xbee.getResponse().getApiId() == ZB_EXPLICIT_RX_RESPONSE)
    {
      digitalWrite( LEDV, LOW );
      delay(200);
      digitalWrite( LEDR, HIGH );
      xbee.getResponse().getZBRxResponse(rx);
      for (int i = 0; i < rx.getDataLength(); i++)
      {
        sample += (char)rx.getData(i);
      }
      String stazione = sample.substring(6, 8);
      String tempo = sample.substring(9, 17);
      String card = sample.substring(18, 22);
      Serial.println(stazione + ":" + tempo + ":" + card);
      LoRa_txMode();
      // send packet via MKR1310
      LoRa.beginPacket();
      LoRa.print(sample);
      LoRa.endPacket();
      LoRa_rxMode();
      //LoRa.receive();
      LoRa.parsePacket();
      byte sender = LoRa.read();
      while (sender != 0xBB) {
        Serial.println("conferma:" + sender);
        LoRa.beginPacket();
        LoRa.print(sender);
        LoRa.endPacket();
      }
      conferma = "";
      Serial.println("trasmesso");
      LoRa_txMode();
    }
    else if (xbee.getResponse().isError()) {
      Serial.println("Error reading packet.  Error code: ");
      Serial.println(xbee.getResponse().getErrorCode());
    }
  }

  void LoRa_rxMode() {
    LoRa.disableInvertIQ();               // normal mode
    LoRa.receive();                       // set receive mode
  }

  void LoRa_txMode() {
    LoRa.idle();                          // set standby mode
    LoRa.enableInvertIQ();                // active invert I and Q signals
  }
