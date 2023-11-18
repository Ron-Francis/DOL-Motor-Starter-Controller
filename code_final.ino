#include <SoftwareSerial.h>
#include <EEPROM.h>

#define STRING_SIZE 50

String message;

byte modeValue;
int timeDelay=5;

int i;
int modeAddress = 0;
int motorState = 0;
int initialTime,finalTime;

const int buttonRight = 7;
const int buttonCenter = 6;
const int buttonLeft = 5;
const int buttonRegister = 4;

const int mode1LED = 8;
const int mode2LED = 9;
const int mode3LED = 10;

SoftwareSerial mySerial(3, 2);

void setup() {

  Serial.begin(9600);

  mySerial.begin(9600);

  pinMode(13,OUTPUT);//pin for relay 1
  pinMode(12,OUTPUT);//pin for relay 2
  pinMode(11,OUTPUT);//pin for relay 3

  pinMode(buttonRight,INPUT);
  pinMode(buttonCenter,INPUT);
  pinMode(buttonLeft,INPUT);
  pinMode(buttonRegister,INPUT);

  pinMode(mode1LED,OUTPUT);
  pinMode(mode2LED,OUTPUT);
  pinMode(mode3LED,OUTPUT);

  digitalWrite(mode1LED,HIGH);
  digitalWrite(mode2LED,HIGH);
  digitalWrite(mode3LED,HIGH);
  delay(500);
  digitalWrite(mode1LED,LOW);
  digitalWrite(mode2LED,LOW);
  digitalWrite(mode3LED,LOW);
  delay(500);
  

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT");
  updateSerial();

  if (isNetworkAvailable()){}
  else{
    Serial.println("Reconnecting");
    setup();}
  
  mySerial.println("AT+CMGF=1");
  updateSerial();

  mySerial.println("AT+CNMI=1,2,0,0,0");
  updateSerial();

  modeFinder();
  if (modeValue == 0){digitalWrite(mode1LED,HIGH);}
  else if (modeValue == 1){digitalWrite(mode2LED,HIGH);}
  else if (modeValue == 2){digitalWrite(mode3LED,HIGH);}

}

bool isNetworkAvailable() {
  mySerial.println("AT+CREG?");
  delay(500);

  if (mySerial.available()) {
    String response = mySerial.readString();
    if (response.indexOf("+CREG: 0,1") != -1 || response.indexOf("+CREG: 0,5") != -1) {
      Serial.println("Network available");
      return true;
    }
  }

  Serial.println("Network not available");
  return false;
}

void loop() {
  updateSerial();
  buttonPress();
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());
  }
  while(mySerial.available()) 
  {
    textfinder();
  }
}

void textfinder()
{
  message = mySerial.readString();
  Serial.println(message);

  int senderStartIndex = message.indexOf(":") + 3;
  Serial.println(senderStartIndex);
  int senderEndIndex = message.indexOf(",", senderStartIndex)-1;
  String senderNumber = message.substring(senderStartIndex, senderEndIndex);
  Serial.print("Sender Number: ");
  Serial.println(senderNumber);
  String phoneNumber = readStringFromEEPROM(2);

  if (phoneNumber == senderNumber){
    if(message.indexOf("Motor on")>=0){
      String phoneNumber = readStringFromEEPROM(2);
      mySerial.print("AT+CMGS=\"");
      mySerial.print(phoneNumber);
      mySerial.println("\"");
      updateSerial();
      mySerial.print("Motor Start complete");
      updateSerial();
      mySerial.write(26);
      motorON();
    }
    else if(message.indexOf("Motor off")>=0){
      String phoneNumber = readStringFromEEPROM(2);
      mySerial.print("AT+CMGS=\"");
      mySerial.print(phoneNumber);
      mySerial.println("\"");
      updateSerial();
      mySerial.print("Motor Stop Complete");
      updateSerial();
      mySerial.write(26);
      motorOFF();
    }
    else{
      Serial.println(message);
      Serial.println("");
    }
  } 
}

//Motor Starter...................

void motorON()
{
  if (motorState == 0){
    motorState = 1;
    modeFinder();
    Serial.println(modeValue);
    if (modeValue == 0){
      mode1Starter();
    }
    else if (modeValue == 1){
      mode2Starter();
    }
    else if (modeValue == 2){
      mode3Starter();
    }
  }
  
}

//Motor stopper............

void motorOFF()
{
  motorState = 0;
  modeFinder();
  Serial.println(modeValue);
  if (modeValue == 0){
    mode1Stopper();
  }
  else if (modeValue == 1){
    mode2Stopper();
  }
  else if (modeValue == 2){
    mode3Stopper();
  }
}

//Mode and delay readers.............

void modeFinder()
{
  modeValue = EEPROM.read(modeAddress);
}

//Starters..................

void mode1Starter(){
  digitalWrite(13,HIGH);
}
void mode2Starter(){
  digitalWrite(12,HIGH);
  delay(200);
  digitalWrite(12,LOW);
}
void mode3Starter(){
  digitalWrite(12,HIGH);
  delay(timeDelay*1000);
  digitalWrite(12,LOW);
}


//Stoppers..................

void mode1Stopper(){
  digitalWrite(13,LOW);
}
void mode2Stopper(){
  digitalWrite(11,HIGH);
  delay(200);
  digitalWrite(11,LOW);
}
void mode3Stopper(){
  digitalWrite(11,HIGH);
  delay(200);
  digitalWrite(11,LOW);
}

void resetAll()
{
  digitalWrite(13,LOW);
  digitalWrite(11,HIGH);
  delay(200);
  digitalWrite(11,LOW);
}

void modeWriter(int mode){
  if (mode == 1){
    EEPROM.write(modeAddress, 0x00);
  }
  else if (mode == 2){
    EEPROM.write(modeAddress, 0x01);
  }
  else if (mode == 3){
    EEPROM.write(modeAddress, 0x02);
  }
}

void buttonPress()
{
  if(digitalRead(buttonLeft)==HIGH)
  {
    initialTime = millis();
    while(digitalRead(buttonLeft)==HIGH)
    {
      finalTime = millis();
      if ((finalTime-initialTime)>=1000)
      {
        resetAll();
        modeWriter(1);
        digitalWrite(mode1LED,HIGH);
        digitalWrite(mode2LED,LOW);
        digitalWrite(mode3LED,LOW);
      }
    }
  }
  else if(digitalRead(buttonCenter)==HIGH)
  {
    initialTime = millis();
    while(digitalRead(buttonCenter)==HIGH)
    {
      finalTime = millis();
      if ((finalTime-initialTime)>=1000)
      {
        resetAll();
        modeWriter(2);
        digitalWrite(mode1LED,LOW);
        digitalWrite(mode2LED,HIGH);
        digitalWrite(mode3LED,LOW);
      }
    }
  }
  else if(digitalRead(buttonRight)==HIGH)
  {
    initialTime = millis();
    while(digitalRead(buttonRight)==HIGH)
    {
      finalTime = millis();
      if ((finalTime-initialTime)>=1000)
      {
        resetAll();
        modeWriter(3);
        digitalWrite(mode1LED,LOW);
        digitalWrite(mode2LED,LOW);
        digitalWrite(mode3LED,HIGH);
      }
    }
  }
  else if(digitalRead(buttonRegister)==HIGH)
  {
    initialTime = millis();
    while(digitalRead(buttonRegister)==HIGH)
    {
      finalTime = millis();
      if ((finalTime-initialTime)>=3000)
      {
        Serial.println("Number register started");
        resetAll();
        NumberUpdater();
      }
    }
  }
}

void NumberUpdater()
{
  
  initialTime = millis();
  finalTime = millis();
  digitalWrite(mode1LED,HIGH);
  digitalWrite(mode2LED,HIGH);
  digitalWrite(mode3LED,HIGH);
  while ((finalTime-initialTime)<=30000)
  {
    finalTime = millis();
    if (mySerial.available()){
      message = mySerial.readString();
      if(message.indexOf("register")>=0){
        if (message.indexOf("+CMT:") != -1) {
          int senderStartIndex = message.indexOf(":") + 3;
          Serial.println(senderStartIndex);
          int senderEndIndex = message.indexOf(",", senderStartIndex)-1;
          String senderNumber = message.substring(senderStartIndex, senderEndIndex);
          Serial.print("Sender Number: ");
          Serial.println(senderNumber);
          writeStringToEEPROM(2, senderNumber);
          digitalWrite(mode1LED,LOW);
          digitalWrite(mode2LED,LOW);
          digitalWrite(mode3LED,LOW);
          delay(500);
          digitalWrite(mode1LED,HIGH);
          digitalWrite(mode2LED,HIGH);
          digitalWrite(mode3LED,HIGH);
          delay(500);
          break;
        }
      }
    }
  }
  digitalWrite(mode1LED,LOW);
  digitalWrite(mode2LED,LOW);
  digitalWrite(mode3LED,LOW);
  setup();
}

#define STRING_SIZE 15  // Maximum string length to store in EEPROM

void writeStringToEEPROM(int addr, const String& data) {
  int dataSize = data.length();
  for (int i = 0; i < dataSize; i++) {
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.write(addr + dataSize, '\0');  // Null-terminate the string
}

String readStringFromEEPROM(int addr) {
  char buffer[STRING_SIZE];
  for (int i = 0; i < STRING_SIZE; i++) {
    buffer[i] = EEPROM.read(addr + i);
    if (buffer[i] == '\0') {
      break;
    }
  }
  return String(buffer);
}
