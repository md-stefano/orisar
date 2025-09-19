// Sistema OriSar ideato e realizzato da Stefano Mocci Demartis 2017 - tutti i diritti sono riservati - riproduzione vietata
// stazione scarico WEMOS WIFI
// Driver WeMos D1 R2 & mini
String versione = "OriSar V4 2025 stazione scarico WEMOS WIFI Display v4";

char server1[] = "www.xxx.it";

#include <ESP8266WiFi.h>
WiFiServer server(80);
const char* ssid = "AndroidAP"; const char* password = "password";

#include <SPI.h>
#include "MFRC522.h"
#include <RFID.h>
#define RST_PIN        D1          // Configurable, see typical pin layout above
#define SS_PIN         D2         // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
int LEDR = D3;
int LEDV = D4;
int BUZ = D0;


#include <Wire.h>
#include <Ethernet.h>
EthernetClient client;

#define DEBUG true
#define Serial if(DEBUG)Serial
#define DEBUG_OUTPUT Serial
extern "C" {
#include "user_interface.h"
}



byte blockAddr      = 1;
byte dataBlock[]    = {
  0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
  0x05, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
  0x08, 0x09, 0xff, 0x0b, //  9, 10, 255, 12,
  0x0c, 0x0d, 0x0e, 0x0f  // 13, 14,  15, 16
};
byte trailerBlock   = 7;

long tempo1[45], stazione1[45];
int TTot;
int contastazione = 0;
String OriTag = "";
String strURL = "";

void connessioni()
{

  if (WiFi.status() != WL_CONNECTED)
  {
    strcpy(server1, "www.xxx.it");
    ssid = "sssss"; password = "pppp"; 
    Serial.print("@");
    Serial.print("Conn. a ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    for (int i = 0; i < 55; i++)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        delay(200);
        Serial.print(".");
      }
      else exit;
    }
  }
  // aggiungere altri access point WiFi
  else
  {
    Serial.println("NON Connesso");
  }

}


void setup() {
  pinMode (LEDV, OUTPUT);
  pinMode (LEDR, OUTPUT);
  pinMode (BUZ, OUTPUT);
  digitalWrite(LEDV, LOW);
  digitalWrite(LEDR, LOW);
  //nSerial.begin(74880);
  Serial.begin(9600);        // Initialize serial communications with the PC
  Serial.println(versione);
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card

  connessioni();

  tone(BUZ, 2000, 2000);
  delay(300);
  noTone(BUZ);
  digitalWrite(LEDV, HIGH );
  Serial.println(F("In attesa di  OriTag"));

  Serial.println();

}


void loop() {


  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())    return;
  Serial.print("@");
  digitalWrite(LEDV, HIGH);
  delay(500);
  String Tinizio = "", Tfine = "", Tempotot = "";
  Serial.print(F("Card:"));    //Dump UID
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], DEC);
  }
  Serial.println();
  //Serial.print(F(" PICC type: "));   // Dump PICC type
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //Serial.println(mfrc522.PICC_GetTypeName(piccType));
  byte len;
  byte b = 0;
  byte punti = 1;
  String t1 = "";
  String t2 = "";
  int contaST = 1;
  String ST;

  for (byte g = 0; g < 16; g++) // 15 settori
  {
    byte sector = g;
    int contast = 0;

    MFRC522::StatusCode status;
    byte buffer[20];
    byte size = sizeof(buffer);

    for (byte h = 0; h < 4; h++)
    {
      blockAddr = b; // inizialmente =0

      status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        //digitalWrite(7, LOW);
        return;
      }
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
      if (status != MFRC522::STATUS_OK)
      {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        digitalWrite(LEDV, LOW);
      }

      digitalWrite(LEDV, LOW);
      digitalWrite(LEDR, LOW);
      if (h != 3)
      {
        if (b >= 0 && b <= 3)
        {
          //***** TAG
          if (blockAddr == 1)
          {
            Serial.print("@");
            OriTag = "";
            //digitalWrite(LED, HIGH);
            Serial.print("OriTag:");
            for (byte i = 0; i < 4; i++)
            {
              Serial.print((char)buffer[i]);
              OriTag = OriTag + ((char)buffer[i]);
            }
            Serial.println();
            Tinizio = ""; Tfine = ""; contastazione = 0;
          }
          //***** Giorno
          if (blockAddr == 2)
          {
            Serial.print("Day:");
            String giorno;
            for (byte i = 0; i < 10; i++)
            {
              Serial.print((char)buffer[i]);
            }
            Serial.println();
            strURL = "GET /cartella/importa-tempo.php?oritag=";
            strURL += OriTag;
          }
        }
        if ((blockAddr != 60) && (blockAddr > 3))
        {
          //***** 4 START

          String starttime = "";
          if (blockAddr == 4)
          {
            Serial.println("Stazione:Start:19");
            Serial.print("Ora:");
            String ttime = "";
            for (byte i = 0; i < 8; i++)
            {
              ttime = ttime + (char)buffer[i];
            }
            starttime = ttime.substring(0, 2) + ttime.substring(3, 5) + ttime.substring(6, 8);
            t1 = ttime;
            Serial.print(ttime.substring(0, 2) + ":" + ttime.substring(3, 5) + ":" + ttime.substring(6, 8));
            Serial.println(":");
            Tinizio = ttime.substring(0, 2) + ":" + ttime.substring(3, 5) + ":" + ttime.substring(6, 8);
            if (Tinizio == "")
              strURL += "&Tpartenza=no-time";
            else
              strURL += "&Tpartenza=" + Tinizio;
          }

          //***** altre
          String subst;
          String Stazione;

          if (blockAddr >= 4 && blockAddr <= 6) Stazione = blockAddr + 15; // 4=19=start
          if (blockAddr >= 8 && blockAddr <= 10) Stazione = blockAddr + 14;
          if (blockAddr >= 12 && blockAddr <= 14) Stazione = blockAddr + 13;
          if (blockAddr >= 16 && blockAddr <= 18) Stazione = blockAddr + 12;
          if (blockAddr >= 20 && blockAddr <= 22) Stazione = blockAddr + 11;
          if (blockAddr >= 24 && blockAddr <= 26) Stazione = blockAddr + 10;
          if (blockAddr >= 28 && blockAddr <= 30) Stazione = blockAddr + 9;
          if (blockAddr >= 32 && blockAddr <= 34) Stazione = blockAddr + 8;
          if (blockAddr >= 36 && blockAddr <= 38) Stazione = blockAddr + 7;
          if (blockAddr >= 40 && blockAddr <= 42) Stazione = blockAddr + 6;
          if (blockAddr >= 44 && blockAddr <= 46) Stazione = blockAddr + 5;
          if (blockAddr >= 48 && blockAddr <= 50) Stazione = blockAddr + 4;
          if (blockAddr >= 52 && blockAddr <= 54) Stazione = blockAddr + 3;
          if (blockAddr >= 56 && blockAddr <= 58) Stazione = blockAddr + 2;
          if (blockAddr >= 61 && blockAddr <= 62) Stazione = blockAddr;
          starttime = "";
          for (byte i = 0; i < 12; i++)
          {
            starttime = starttime + (char)buffer[i];
          }
          if (Stazione == "62")
          {
            Serial.println("Stazione:Finish:62:");
            Serial.println("Ora:" + starttime.substring(0, 2) + ":" + starttime.substring(3, 5) + ":" + starttime.substring(6, 8) + ":");

            Tfine = starttime.substring(0, 2) + ":" + starttime.substring(3, 5) + ":" + starttime.substring(6, 8);
            if (Tfine == "")
              strURL += "&Tfine=no-time HTTP/1.1";
            else
              strURL += "&Tfine=" + Tfine + " HTTP/1.1";
          }
          else
            int tempo;
          if (Stazione != "19")
          { Serial.print("Punto:");
            Serial.print(punti);
            Serial.println();
            punti++;
          }
          stazione1[contastazione] = (Stazione.toInt());
          if (Stazione == "19" || Stazione == "62")
          {
            subst = starttime.substring(0, 2) + starttime.substring(3, 5) + starttime.substring(6, 8);
            if (Stazione == "19") t1 = subst;
          }
          else
          {
            if (starttime != "")
            {
              subst = starttime.substring(3, 5) + starttime.substring(6, 8) + starttime.substring(9, 11);
              t2 = starttime.substring(3, 5) + starttime.substring(6, 8) + starttime.substring(9, 11);
              String st3 = starttime.substring(0, 2);
              if (st3 == "61")
              {
                Serial.println("Stazione:100");
                ST = "100";
              }
              else
              {
                Serial.println("Stazione:" + starttime.substring(0, 2) + "");
                ST = starttime.substring(0, 2);
              }
              Serial.println("Ora:" + starttime.substring(3, 5) + ":" + starttime.substring(6, 8) + ":" + starttime.substring(9, 11) + "");
              String Tempo = starttime.substring(3, 5) + ":" + starttime.substring(6, 8) + ":" + starttime.substring(9, 11);
              strURL += "&ST" + String(contaST) + "=" + ST + "&Tempo" + String(contaST) + "=" + Tempo;
              contaST++;
              int ore1 = ((int)(t1[0]) * 10 + (int)(t1[1]));
              int minuti1 = ((int)(t1[2]) * 10 + (int)(t1[3]));
              int secondi1 = ((int)(t1[4]) * 10 + (int)(t1[5]));
              int ore2, minuti2, secondi2;
              ore2 = ((int)(t2[0]) * 10 + (int)(t2[1])) - ore1;
              minuti2 = ((int)(t2[2]) * 10 + (int)(t2[3])) - minuti1;
              secondi2 = ((int)(t2[4]) * 10 + (int)(t2[5])) - secondi1;
              if (secondi2 < 0)
              {
                minuti2--;
                secondi2 = secondi2 + 60;
              }
              if (minuti2 < 0)
              {
                ore2--;
                minuti2 = minuti2 + 60;
              }
              String secondi3, minuti3;
              if (secondi2 < 10) secondi3 = "0" + String(secondi2);
              else secondi3 = String(secondi2);
              if (minuti2 < 10) minuti3 = "0" + String(minuti2);
              else minuti3 = String(minuti2);         // scrittura split fino alla 100
              t1 = t2;
              t2 = "";
            }
          }
          if (starttime != "")
          {
            tempo1[contastazione] = (subst.toInt());
            contast++;
          }

          else tempo1[contastazione] = 0;

          contastazione++;

          if (Stazione == "19") Tinizio = starttime;
          if (Stazione == "62")
          {
            Tfine = starttime;

            if ((Tinizio == "") || (Tfine == "")) Tempotot = "Errore";
            else
            {

              int ore, minuti, secondi;
              ore = (((int)(Tfine[0]) * 10 + (int)(Tfine[1])) - ((int)(Tinizio[0]) * 10 + (int)(Tinizio[1])));
              minuti = (((int)(Tfine[3]) * 10 + (int)(Tfine[4])) - ((int)(Tinizio[3]) * 10 + (int)(Tinizio[4])));
              secondi = (((int)(Tfine[6]) * 10 + (int)(Tfine[7])) - ((int)(Tinizio[6]) * 10 + (int)(Tinizio[7])));

              if (secondi < 0)
              {
                minuti--;
                secondi = secondi + 60;
              }
              if (minuti < 0)
              {
                ore--;
                minuti = minuti + 60;
              }
              String secondi3, minuti3;
              if (secondi < 10) secondi3 = "0" + String(secondi);
              else secondi3 = String(secondi);
              if (minuti < 10) minuti3 = "0" + String(minuti);
              else minuti3 = String(minuti);
              Tempotot = (String)(ore) + ":" + minuti3 + ":" + secondi3;
            }


            long temp;

            int ore1 = ((int)(Tinizio[0]) * 10 + (int)(Tinizio[1]));
            int minuti1 = ((int)(Tinizio[3]) * 10 + (int)(Tinizio[4]));
            int secondi1 = ((int)(Tinizio[6]) * 10 + (int)(Tinizio[7]));
            int ore2, minuti2, secondi2;

            ore1 = ((int)(t1[0]) * 10 + (int)(t1[1]));
            minuti1 = ((int)(t1[2]) * 10 + (int)(t1[3]));
            secondi1 = ((int)(t1[4]) * 10 + (int)(t1[5]));
            ore2 = ((int)(Tfine[0]) * 10 + (int)(Tfine[1])) - ore1;
            minuti2 = ((int)(Tfine[3]) * 10 + (int)(Tfine[4])) - minuti1;
            secondi2 = ((int)(Tfine[6]) * 10 + (int)(Tfine[7])) - secondi1;
            if (secondi2 < 0)
            {
              minuti2--;
              secondi2 = secondi2 + 60;
            }
            if (minuti2 < 0)
            {
              ore2--;
              minuti2 = minuti2 + 60;
            }
            String secondi4, minuti4;
            if (secondi2 < 10) secondi4 = "0" + String(secondi2);
            else secondi4 = String(secondi2);
            if (minuti2 < 10) minuti4 = "0" + String(minuti2);
            else minuti4 = String(minuti2);
          }
        }
        //Serial.println(":");
        // e cosi via fino all ultimo blocco da leggere e comparare
      }
      b++;

    }
    if (blockAddr >= 62)
    {


      digitalWrite(LEDV, HIGH );

      //       delay(500);
      WiFiClient client = server.available();
      while (!client.connect(server1, 80))
      {
        connessioni();
      }
      //creo l'url utilizzando una stringa
      Serial.print("@");
      if (client.connect(server1, 80))
      {
        Serial.print("@");
        Serial.println("Connessione: OK");
        Serial.println(strURL);
        client.println(strURL);
        client.println("Host: xxx.xxx.it");
        client.println("User-Agent: Arduino/1.0");
        Serial.println("Connessione: CHIUSA");
        client.println();
        delay(1);

        Serial.println();
        //chiudo la connessione
        client.stop();
        delay(1000);
        Serial.print("@");
        Serial.println(server1);
        Serial.println("Conn: ok");
        //Serial.println();
        Serial.println("scarico ok");
        delay(500);
        tone(BUZ, 2000, 2000);
        delay(300);
        noTone(BUZ);
        digitalWrite(LEDV, HIGH );
      }
      else
      {
        digitalWrite(LEDV, LOW );
        Serial.print("@");
        Serial.println("Conn: fallita");
        delay(500);
        tone(BUZ, 2000, 2000);
        delay(300);
        noTone(BUZ);
        digitalWrite(LEDR, HIGH );
      }
      delay(2000);

      Serial.println("In attesa di  OriTag");

    }
  }

  digitalWrite(LEDV, LOW);
  digitalWrite(LEDR, LOW);
  byte block;
  //MFRC522::StatusCode status;
  Serial.println(" ");
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD

}
