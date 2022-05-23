#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <MPU6050.h>
#include<SoftwareSerial.h>
#include <SPI.h>

int sendPin = D1;

MPU6050 mpu;

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

void setup()
{

  pinMode(D2, OUTPUT);
  pinMode(D8, OUTPUT);
  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);

  Serial.begin(115200);
  Wire.begin(D6, D7);
  delay(10);
  mpu.begin();

  InitWiFi();

  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;

  pinMode(sendPin, OUTPUT);
  digitalWrite(sendPin, LOW);
}

void loop()
{
  if ( !client.connected() )
  {
    reconnect();
  }
  digitalWrite(D2, LOW);
  Vector rawAccel = mpu.readRawAccel();
  Vector normAccel = mpu.readNormalizeAccel();
  float combine = normAccel.XAxis + normAccel.YAxis + normAccel.ZAxis; //Vibration data from MPU6050
  float temp = mpu.readTemperature(); //Temperature data from MPU6050
  Serial.print("Temperature: ");
  Serial.print(temp);

  if (normAccel.XAxis >= 6.50 && normAccel.XAxis <= 10.00) {
    digitalWrite(sendPin, HIGH);
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    delay(100);
    digitalWrite(sendPin, LOW);
  }
  else if (normAccel.XAxis >= 29.56 && normAccel.XAxis <= 33.40) {
    digitalWrite(sendPin, HIGH);
    digitalWrite(D2, HIGH);
    digitalWrite(D5, HIGH);
    delay(100);
    digitalWrite(sendPin, LOW);
  }
  digitalWrite(D2, LOW);
  digitalWrite(D5, LOW);

  String payload = "{";  //Sting formation to upload the data
  payload += "\"temperature\":";
  payload += String(temp);
  payload += ",";
  payload += "\"XAxis\":";
  payload += String(normAccel.XAxis);
  payload += ",";
  payload += "\"YAxis\":";
  payload += String(normAccel.YAxis);
  payload += ",";
  payload += "\"ZAxis\":";
  payload += String(normAccel.ZAxis);
  payload += "}";
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );


  client.loop();
  delay(1000);
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
    digitalWrite(D8, HIGH); //connnecting to data server
    Serial.print("Connecting to Thingsboard node ...");
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) // Attempt to connect (clientId, username, password)
    {
      Serial.println( "[DONE]" );
      // connected to the server
      digitalWrite(D8, LOW);
    }
    else
    {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      digitalWrite(D2, HIGH); //failed reconnecting to server
      delay( 5000 ); // Wait 5 seconds before retrying
    }
  }
}
