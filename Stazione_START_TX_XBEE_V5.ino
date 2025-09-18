
// Sistema OriSar ideato e realizzato da Stefano Mocci Demartis 2017 - tutti i diritti sono riservati - riproduzione vietata
// stazione passaggio
// V5 Settembre 2025
String versione = "OriSar START TX XBee V5 Settembre 2025";
byte stazione = 19; // 20----58 la prima e la 4=19 start - l'ultima e la 61=100 -  62=101 e il finish
byte sector = 15;
byte blockAddr = 4;

byte dataBlock[]    = {
  0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
  0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
  0x08, 0x09, 0xff, 0x0b, //  9, 10, 255, 12,
  0x0c, 0x0d, 0x0e, 0x0f  // 13, 14,  15, 16
};
byte trailerBlock   = 7;

#include <SPI.h>
const byte address[6] = "00001";
#include <MFRC522.h>

const unsigned long interval = 2000; //ms  // How often to send 'hello world to the other unit

unsigned long last_sent;             // When did we last send?
unsigned long packets_sent;          // How many have we sent already
#include "RTClib.h"
RTC_DS3231 rtc;

MFRC522 mfrc522(10, 9); //ss rst
MFRC522::StatusCode status;
MFRC522::MIFARE_Key key;

char *inputString = "Hello";

#include <XBee.h>
XBee xbee = XBee();
#include <SoftwareSerial.h>
int pin_RX=7;
int pin_TX=8;
SoftwareSerial PortaSeriale = SoftwareSerial(pin_RX, pin_TX);

void setup() {


  Serial.begin(115200);
  SPI.begin();      // Avvia il bus SPI
  Serial.print(versione); Serial.print(" - stazione:"); Serial.println((String)stazione);
  //digitalWrite(6, HIGH); SPI.begin(); rtc.begin();    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));Serial.println("Set Time");
  pinMode (6, OUTPUT);
  digitalWrite(6, HIGH);
  pinMode (4, OUTPUT);
  digitalWrite(4, HIGH);
  tone(3, 2000);
  delay(300);
  digitalWrite(4, LOW);
  noTone(3);
  pinMode(pin_RX, INPUT);
  pinMode(pin_TX, OUTPUT);
  PortaSeriale.begin(9600);
  xbee.setSerial(PortaSeriale);
}

void errore()
{
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}


void status1(byte block) {
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    errore();
  }
}



void loop()
{
  digitalWrite(6, LOW);
  delay(500);
  pinMode (6, OUTPUT);
  digitalWrite(6, HIGH);

  SPI.begin();
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  mfrc522.PCD_ClearRegisterBitMask(mfrc522.RFCfgReg, (0x07 << 4));
  mfrc522.PCD_SetRegisterBitMask(mfrc522.RFCfgReg, (0x05 << 4));     // Imposta Rx Gain su max

  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  if ( ! mfrc522.PICC_IsNewCardPresent())  return;
  if ( ! mfrc522.PICC_ReadCardSerial())    return;

  char buffer[20];
  byte size = sizeof(buffer);
  String codiceLetto = "";
  char code1[4];

  status1(1);
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(1, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    errore();
    return;
  }
  for (byte i = 0; i < 4; i++)
  {
    codiceLetto += buffer[i];
    code1[i] = buffer[i];
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  status1(trailerBlock);
  rtc.begin();

  if (codiceLetto == "9999")
  {
    //mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, 15);
    status1(62);
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(62, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
      errore();
    }
    String buffer1;
    for (byte i = 0; i < 16; i++) buffer1 += buffer[i];
    //buffer1 = buffer;

    String ora = buffer1.substring(0, 2);
    String minu = buffer1.substring(3, 5);
    String seco = buffer1.substring(6, 8);
    String giorno = buffer1.substring(8, 10);
    String mese = buffer1.substring(10, 12);
    String anno = buffer1.substring(12, 16);
    Serial.println("Set Time: " + ora + ":" + minu + ":" + seco + " " + giorno + "/" + mese + "/" + anno);
    rtc.adjust(DateTime(anno.toInt(), mese.toInt(), giorno.toInt(), ora.toInt(), minu.toInt(), seco.toInt()));
    tone(3, 2000);
    pinMode (4, OUTPUT);
    digitalWrite(4, HIGH);
    delay(300);
    noTone(3);
    digitalWrite(4, LOW);
  }
  else  // ************scrittura passaggio stazione:ora:minuto:secondo:
  {

    DateTime now = rtc.now();
    char buffer1[18];

    sprintf(buffer1,  "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    for (byte i = 9; i < 18; i++) buffer1[i] = ' ';
    status1(blockAddr);

    status = mfrc522.MIFARE_Write(blockAddr, buffer1, 16);
    if (status != MFRC522::STATUS_OK)
    {
      errore();
      return;
    }

    char trasmettere[20];
    sprintf(trasmettere,  "%2d:%s:%s", stazione, buffer1, code1);
    for (byte i = 16; i < 20; i++) trasmettere[i] = ' ';
    Serial.println(trasmettere);
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();

    XBeeAddress64 addr64 = XBeeAddress64(0x0, 0xFFFF);
    ZBTxRequest zbTx = ZBTxRequest(addr64, trasmettere, sizeof(trasmettere));
    delay(100);

    xbee.send(zbTx);

    pinMode (5, OUTPUT);
    tone(3, 2000);

    digitalWrite(5, HIGH);
    delay(500);
    noTone(3);
    digitalWrite(5, LOW);
  }
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
