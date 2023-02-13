#include <LiquidCrystal.h>
#include "DHT.h"
#include <MFRC522.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal lcd(7,8,9,10,11,12);
LiquidCrystal_I2C lcd2 = LiquidCrystal_I2C(0x27, 20, 4);

#define DHTTYPE DHT11
#define DHTPIN A0
#define RST_PIN 2
#define SS_PIN 53

int sensorPin = 36;

int relaystate = 0;

DHT dht = DHT(DHTPIN,  DHTTYPE);
MFRC522 mfrc522(SS_PIN, RST_PIN);

byte lock[8] = {
  B01110,
  B10001,
  B10001,
  B11111,
  B11011,
  B11011,
  B11111,
  B00000
};
byte unlock[8] = {
  B01111,
  B10000,
  B10000,
  B11111,
  B11011,
  B11011,
  B11111,
  B00000
};

int buttonState = 0;

void setup() 
{
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, INPUT);
  pinMode(26, OUTPUT);
  pinMode(28, OUTPUT);
  pinMode(30, OUTPUT);
  lcd.begin(16,2);
  lcd.createChar(0, lock);
  lcd.createChar(1, unlock);
  tone(3, 1000, 200);
  dht.begin();
  pinMode(22, INPUT);
  pinMode(23, OUTPUT);
  pinMode(38, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();	
  mfrc522.PCD_DumpVersionToSerial();
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  lcd2.init();
  lcd2.backlight();
}

void loop() 
{
  // put your main code here, to run repeatedly:
   int light = digitalRead(sensorPin);

  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    digitalWrite(38, relaystate); //relay
    digitalWrite(6, LOW); //red LED
    digitalWrite(5, HIGH); //green LED
    LockedStatusScreen();
    TemperatureScreen();
    delay(5000);
    //digitalWrite(38, HIGH); //relay
    delay(3000);

     // Clears the trigPin
    digitalWrite(23, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(23, HIGH);
    delayMicroseconds(10);
    digitalWrite(23, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(22, HIGH);
    // Calculating the distance
    int distance = duration * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.println(distance);
    Serial.print("Light: ");    
    Serial.println(light);

    if  (distance <= 30)
    {
      while  (distance <= 30)
      {
        tooclose();
        digitalWrite(23, LOW);
        delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(23, HIGH);
        delayMicroseconds(10);
        digitalWrite(23, LOW);
        // Reads the echoPin, returns the sound wave travel time in microseconds
        long duration = pulseIn(22, HIGH);
        // Calculating the distance
        int distance = duration * 0.034 / 2;
        // Prints the distance on the Serial Monitor
        Serial.print("Distance: ");
        Serial.println(distance);
        if (distance > 30)
        {
          return;
        }
      }
    }

    if (light == 0)
    {
      if (relaystate == 0)
      {
        relaystate = 1;
      }
      else if (relaystate == 1)
      {
        relaystate = 0;        
      }
      while (light == 0)
      {
          
          flashcaught();
          return;
      }
    }
    
    return;
  }

  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }

  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  
  if (content.substring(1) == "FC FA 68 CD") //change here the UID of the card/cards that you want to give access
  {
    Serial.println("Authorized access");
    Serial.println();
    toneaccept();

    int i = 0;
    while (i < 119)
    {
      Serial.println(i);
      digitalWrite(6, HIGH);
      digitalWrite(5, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Museum Systems ");
      lcd.setCursor(1, 1);
      lcd.write(byte(1));
      lcd.setCursor(4, 1);
      lcd.print("Disengaged");
      i++;
      if (mfrc522.PICC_IsNewCardPresent() && i > 5)
      {
        engaged();
        return;
      }
      delay(1000);
      digitalWrite(6, LOW);
      digitalWrite(5, LOW);
      delay(1000);
    }
    engaged();
  }
 else   
 {
    Serial.println(" Access denied");
    deny();
    return;
  }

}

void LockedStatusScreen() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Museum Systems");
  lcd.setCursor(3, 1);
  lcd.write(byte(0));
  lcd.setCursor(6, 1);
  lcd.print("Engaged");
}

void TemperatureScreen() 
{
  float t = dht.readTemperature();
  int temp = round(t);
  float h = dht.readHumidity();
  int humid = round(h);
  Serial.println(temp);

  lcd2.clear();
  lcd2.setCursor(0, 0);
  lcd2.print("Temp: ");
  lcd2.print(temp);
  lcd2.print((char)223);
  lcd2.setCursor(0, 1);
  lcd2.print("Humidity: ");
  lcd2.print(humid);
  lcd2.print("%");

  if  (temp == 24 || temp == 20)
    {
      while  (temp == 30 || temp == 20)
      {
        temperaturechange(temp);
        if (temp == 20)
        {
          return;
        }
      }
    }
}

void toneaccept()
{
  tone(3,1000,100);
  delay(200);
  tone(3,1000,100);
  delay(200);
  tone(3,1000,100);
  delay(200);
}

void deny()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Access Denied!");
  digitalWrite(6, HIGH);
  digitalWrite(5, LOW);
  tone(3,440,1000);
  delay(500);
  digitalWrite(6, LOW);
  delay(500);
  digitalWrite(6, HIGH);
  delay(500);
  digitalWrite(6, LOW);
  delay(500);
}

void tooclose()
{
  lcd2.clear();
  TemperatureScreen();
  lcd2.setCursor(0, 3);
  lcd2.print("Too Close!");
  setColour(0, 0, 255);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(0, 0, 255);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
}

void flashcaught()
{
  lcd2.clear();
  TemperatureScreen();
  lcd2.setCursor(0, 3);
  lcd2.print("Flash Detected!");
  setColour(255,105,180);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(255,105,180);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(255,105,180);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(255,105,180);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
}

void temperaturechange(int temp)
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Too Hot/Too Cold!");
  lcd.setCursor(1, 1);
  lcd.print(temp);
  setColour(255, 255, 0);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(255, 255, 0);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
}

void touchortilt()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Exhibit has been Touched/Tilted");
  setColour(0, 0, 255);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
  setColour(0, 0, 255);  
  delay(500);
  setColour(0, 0, 0);
  delay(500);
}

void engaged()
{
  tone(3, 1000, 250);
  delay(1000);
}

void setColour(int redValue, int greenValue, int blueValue) {
  analogWrite(26, redValue);
  analogWrite(28, greenValue);
  analogWrite(30, blueValue);
}