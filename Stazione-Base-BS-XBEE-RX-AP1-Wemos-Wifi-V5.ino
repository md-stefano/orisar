// Sistema OriSar ideato e realizzato da Stefano Mocci Demartis 2017 - tutti i diritti sono riservati - riproduzione vietata
// stazione passaggio
// V4.2 MARZO 2018
// Stazione Base BS WEMOS D1 R2 + XBEE Coordinatore in ricezione
//#include <Printers.h>
#include <XBee.h>

#include <SoftwareSerial.h>

int LEDR = D5;
int LEDV = D7;
int BUZ = D0;

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBRxResponse rx = ZBRxResponse();

#include <ESP8266WiFi.h>
WiFiServer server(80);
#include <Ethernet.h>
EthernetClient client;

String server3 = "192.x.x.x";
char server1[] = "192.x.x.x";
String strURL = "";
const char* ssid = "x"; const char* password = "x";


void connessioni()
{
  int tempo_connessione = 30;
  if (WiFi.status() != WL_CONNECTED)
  {
    strcpy(server1, "www.xxx.yyy");
    ssid = "xxx"; password = "xxxx"; // Portatile
    Serial.print("@");
    Serial.print("Conn. a ");
    Serial.print(ssid);
    WiFi.begin(ssid, password);
    for (int i = 0; i < tempo_connessione; i++)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        delay(200);
        Serial.print(".");
      }
      else exit;
    }
  }
  Serial.println();


  WiFiClient client = server.available();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("@");
    Serial.print("WiFi connesso a: ");
    server.begin();
    Serial.println(ssid);
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("NON Connesso");
  }

}

void strasmissione(String stazione, String tempo, String card, String strURL, char server1[])
{
  WiFiClient client = server.available();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Connesso");
  }
  else
  {
    Serial.println("NON Connesso");
    connessioni();
  }

  if (client.connect(server1, 80))
  {
    //Serial.print("@");
    Serial.println("Connessione: OK");
    String c;
    do {
      Serial.println(strURL);
      client.println(strURL);
      client.println("Host: www.xxx.it");
      client.println("User-Agent: Arduino/1.0");
      client.println();
      c = client.readString();
      Serial.println(c.substring(0, 15));
    } while (c.substring(0, 15) != "HTTP/1.1 200 OK");
    Serial.println("Connessione: CHIUSA");
    delay(1);

    Serial.println();
    //chiudo la connessione
    client.stop();
    delay(500);
    Serial.print("@");
    Serial.println("Conn: ok");
    Serial.println("scarico ok");
    digitalWrite( LEDR, LOW );
    digitalWrite( LEDV, HIGH );
    delay(200);
    digitalWrite( LEDV, LOW );
    delay(200);
    digitalWrite( LEDV, HIGH );
  }
  else
  {
    Serial.print("@");
    Serial.println("NO connessione");
    digitalWrite( LEDV, HIGH );
    delay(200);
    digitalWrite( LEDV, LOW );
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode (LEDV, OUTPUT);
  pinMode (LEDR, OUTPUT);
  pinMode (BUZ, OUTPUT);
  digitalWrite(LEDV, LOW);
  digitalWrite(LEDR, LOW);
  xbee.setSerial(Serial);
  Serial.println();
  connessioni();
  tone(BUZ, 2000, 2000);
  delay(300);
  noTone(BUZ);
  digitalWrite(LEDV, HIGH );
  Serial.println(F("In attesa di  OriTag"));

  Serial.println();
}



void loop()
{
  String sample;
  xbee.readPacket();
  if (xbee.getResponse().isAvailable())
  {
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

      if (stazione == "62")
      {
        strURL = "GET /xxxxx.php?oritag=" + card + "&Tfine=" + tempo + " HTTP/1.1";
        strasmissione(stazione, tempo, card, strURL, server1);
      }
      if (stazione == "19")
      {
        strURL = "GET /xxxxx.php?oritag=" + card + "&Tpartenza=" + tempo + " HTTP/1.1";
        strasmissione(stazione, tempo, card, strURL, server1);
      }
      if ((atoi(stazione.c_str())>0)&&(atoi(stazione.c_str())<62))
      if ((stazione != "19")&&(stazione != "62"))
      {
        strURL = "GET /xxxxx.php?oritag=" + card + "&ST1="+ stazione+"&Tempo1=" + tempo + " HTTP/1.1";
        strasmissione(stazione, tempo, card, strURL, server1);
      }
     
      digitalWrite( LEDR, LOW );digitalWrite( LEDV, HIGH );
    }
  } else if (xbee.getResponse().isError()) {
    Serial.println("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  }

  byte block;
}
