#include "Arduino.h"
#include <EMailSender.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include<SoftwareSerial.h>
#include<TinyGPS.h>
#include <SPI.h>

TinyGPS gps;
SoftwareSerial sgps(D2, D3); //rx, tx

int receivePin = D1;

float lat, lon;

#define WIFI_AP "majorProject" //Wifi Name to be set
#define WIFI_PASSWORD "Project123"  //Wifi password to be set
#define TOKEN "iW6emu2pQIXfU9EzsUc2" //Token key to send the data to cloud
char thingsboardServer[] = "demo.thingsboard.io"; //cloud server

WiFiClient wifiClient;
PubSubClient client(wifiClient);
void reconnect();
void InitWiFi();
unsigned long lastSend;
int status = WL_IDLE_STATUS;

EMailSender emailSend("projectmajor04@gmail.com", "major@2021");

void setup()
{

  pinMode(D8, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);

  Serial.begin(115200);
  delay(10);
  sgps.begin(9600);

  InitWiFi();

  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;

  pinMode(receivePin, INPUT);
  digitalWrite(receivePin, LOW);
}

void loop()
{
  if ( !client.connected() )
  {
    reconnect();
  }
  digitalWrite(D5, LOW);
  while (sgps.available())  //GPS status
  {
    int c = sgps.read();
    if (gps.encode(c))
    {
      gps.f_get_position(&lat, &lon);
      String payload = "{";  //Sting formation to upload the data
      payload += "\"latitude\":";
      payload += String(lat, 6);
      payload += ",";
      payload += "\"longitude\":";
      payload += String(lon, 6);
      payload += "}";
      Serial.print("Latitude=");
      Serial.print(lat);
      Serial.print("Longitude=");
      Serial.println(lon);
      char attributes[100];
      payload.toCharArray( attributes, 100 );
      client.publish( "v1/devices/me/telemetry", attributes );
      Serial.println( attributes );
    }
  }

  int temp = digitalRead(receivePin);
  if (temp == 1) {
    digitalWrite(D5, HIGH);
    EMailSender::EMailMessage message;
    message.subject = "Accident Alert";
    String sendMessage = (String)"Accident has been detected on location " + (String)"https://www.google.com/maps/?q=" + String(lat, 6) + ',' + String(lon, 6);
    message.message = sendMessage;

    EMailSender::Response resp = emailSend.send("saurabhgirpunje1998@gmail.com", message);

    Serial.println("Sending status: ");
    digitalWrite(D5, LOW);
  }

  client.loop();
}


void InitWiFi()
{
  digitalWrite(D8, HIGH); //Connecting to wifi
  Serial.println("Connecting to AP ..."); // attempt to connect to WiFi network

  WiFi.begin(WIFI_AP, WIFI_PASSWORD);   //Parsing the wifi AP and password to get connect to wifi
  while (WiFi.status() != WL_CONNECTED) // checking if connected
  {
    delay(500);
    Serial.print("."); // if not then print
  }
  Serial.println("Connected to AP");
  digitalWrite(D8, LOW);//Connected to wifi
}


void reconnect()
{
  while (!client.connected()) // Loop until we're reconnected
  {
    status = WiFi.status();
    if ( status != WL_CONNECTED)
    {
      digitalWrite(D8, HIGH); //connecting to wifi
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
      digitalWrite(D8, LOW);// wifi connected
    }
    digitalWrite(D5, HIGH); //connnecting to data server
    Serial.print("Connecting to Thingsboard node ...");
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) // Attempt to connect (clientId, username, password)
    {
      Serial.println( "[DONE]" );
      // connected to the server
      digitalWrite(D5, LOW);
    }
    else
    {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      digitalWrite(D8, HIGH); //failed reconnecting to server
      delay( 5000 ); // Wait 5 seconds before retrying
    }
  }
}
