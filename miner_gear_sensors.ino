#include <SPI.h>
#include <MFRC522.h>

#include "DHT.h"

#define DHTPIN 6 // DHT11 connected to pin 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//#define MQ2pin (3)// MQ2 connected to pin 3
//float gassensorValue;

long pulval;
int PulseSensorpin = A0; //Pulse sensor connected to pin A0v

#include <Wire.h>
#include <Adafruit_BMP085.h>
#define seaLevelPressure_hPa 1013.25

Adafruit_BMP085 bmp;

 
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid1(SS_PIN, RST_PIN);// Create MFRC522 instance.

void setup()
{
  Serial.begin(9600);
  
  SPI.begin();      // Initiate  SPI bus
  rfid1.PCD_Init();   // Initiate MFRC522
  Serial.println("Approximate your card to the reader...");
  Serial.println();
  
  dht.begin();
  
  
  Serial.println();
//  Serial.println("Gas sensor warming up!");
//  pinMode(2, OUTPUT);

  if (!bmp.begin()) {
  Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  while (1) {}
  }
 
  delay(1000);
}

void ser()
{
  // Look for new cards
  if ( ! rfid1.PICC_IsNewCardPresent()) 
  {
    Serial.println("No worker in mine.");
    return;
  }
   //Select one of the cards
  if ( ! rfid1.PICC_ReadCardSerial()) 
  {
    Serial.println("Unable to read card. Possible unauthorized entry.");
    return;
  }
   //rfid1.PICC_ReadCardSerial();
  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < rfid1.uid.size; i++) 
  {
     Serial.print(rfid1.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(rfid1.uid.uidByte[i], HEX);
     content.concat(String(rfid1.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(rfid1.uid.uidByte[i], HEX));
  }
  Serial.println();
}

void loop()
{
   ser();
   //dht11
  float temp= dht.readTemperature();
  float hum = dht.readHumidity();
  Serial.print(F("Humidity in %: "));
  Serial.println(hum);
  Serial.println(F("temp: "));
  Serial.println(temp);
  delay(1000);
  
  //mq2
//  gassensorValue = analogRead(MQ2pin);
//  digitalWrite(2, LOW);
//  Serial.print("Gas Sensor Value: ");
//  Serial.print(gassensorValue);
  
//  if(gassensorValue > 1000)
//  {
//    Serial.println("Smoke detected!");
//    digitalWrite(2, HIGH);  
//  }
//  
//  Serial.println("");
//  delay(1000);

  //pulse
  pulval = analogRead(PulseSensorpin);
  Serial.print("Pulse Sensorvalue= "); 
  long pulse = pulval*100/1023;
  Serial.println(pulse);
  delay(1000);

  //bmp180
//  Serial.print("Temperature = ");
//  float temp2 = bmp.readTemperature();
//  Serial.print(temp2);
//  Serial.println(" *C");
  
  Serial.print("Pressure = ");
  long pres = bmp.readPressure();
  Serial.print(pres);
  Serial.println(" Pa");

  Serial.print("Altitude = ");
  float alt = bmp.readAltitude(seaLevelPressure_hPa * 100);
  Serial.print(alt);
  Serial.println(" meters");

  Serial.println();
  
  delay(3000);
}
