#include <LiquidCrystal.h>
#include "DHT.h"
#include <MFRC522.h>
#include <SPI.h>

LiquidCrystal lcd(7,8,9,10,11,12);

#define DHTTYPE DHT11
#define DHTPIN A0
#define RST_PIN 2
#define SS_PIN 53

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
  lcd.begin(16,2);
  lcd.createChar(0, lock);
  lcd.createChar(1, unlock);
  tone(3, 1000, 200);
  dht.begin();
  pinMode(22, INPUT);
  pinMode(23, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();	
  mfrc522.PCD_DumpVersionToSerial();
	Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop() 
{
  // put your main code here, to run repeatedly:
   
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    digitalWrite(6, LOW);
    digitalWrite(5, HIGH);
    LockedStatusScreen();
    delay(5000);
    TemperatureScreen();
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

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(humid);
  lcd.print("%");
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

void engaged()
{
  tone(3, 1000, 250);
  delay(1000);
}