#include <EEPROM.h>
#include <SoftwareSerial.h>

const float DEFAULT_TEMP = 50;
float MaxTemp = DEFAULT_TEMP;
float MinTemp = DEFAULT_TEMP;
const int Switch1 = 8;
const int Switch2 = 9;
const int Minaddr = 10; //lets write at address 10
const int Maxaddr = 30; //lets write at address 10
float currentTemp = 0;

SoftwareSerial mySerial(3, 4); // RX, TX
//////////////////////////Soft_Serial variables///////////////////
char aMessage[200];                               // to hold the message received
char cha;                                         // for reading char one at a time
byte messageSize;                                 // to hold the received message sixe

//////////////////////////Temp variable//////////////////////////

int ThermistorPin = A0;//The Analogue pin where the thermistor is connected
int Vo;
float R1 = 1000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
boolean isbatteryfull = false;


//////////////////////////////Printing without delay variables//////////////////////

unsigned long previousMillis = 0;
// constants won't change:
const long interval = 2000;

/////////////////////////////////

void setup() {
  pinMode(Switch1, OUTPUT);
  pinMode(Switch2, OUTPUT);

  SwitchOff();//TurnOff the switch
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("Started");
  mySerial.println("BT:Started");

  MinTemp = readFloat(Minaddr);
  MaxTemp = readFloat(Maxaddr);
  PrintMaxMinTemp();
  currentTemp = measureTemp();
  Serial.print("Current Tempt: ");
  Serial.println(currentTemp);
  mySerial.print("BT:Current Tempt: ");
  mySerial.println(currentTemp);

}

void loop() {

  //////////////READ SOFTWARE SERIAL////////////////
  if (mySerial.available()) {
    for (int c = 0; c <= 99; c++) aMessage[c] = 0;    // clear aMessage in prep for new message
    messageSize = 0;                                  // set message size to zero

    while (mySerial.available()) {                    // loop through while data is available
      cha = mySerial.read();                  // get character
      aMessage[messageSize] = cha;            // append to aMessage
      messageSize++;                          // bump message size
      delay(20);                              // just to slow the reads down a bit
    } // while

    aMessage[messageSize] = '\0';                     // set last character to a null
    // and then the message
    String input = String(aMessage);
    if (input.indexOf(",") < 2) {
      if (input.indexOf("_full_") > -1)
      {
        isbatteryfull = true;
        Serial.println("Battery FULL");
        mySerial.println("Battery FULL");
        SwitchOff();//turn off the switch as battery is low...
      } else if (input.indexOf("_low_") > -1)
      {
        isbatteryfull = false;
        SwitchOn();//turn on the switch as battery is low...
        Serial.println("Battery LOW");
        mySerial.println("Battery LOW");
      } else {
        Serial.print("Invalid str");
        Serial.println(input);
        mySerial.print("BT:Invalid str");
        mySerial.println(input);
      }
    } else {
      float mintemp;
      float maxtemp;
      for (int i = 0; i < input.length(); i++) {
        if (input.substring(i, i + 1) == ",") {
          mintemp = input.substring(0, i).toFloat();
          maxtemp = input.substring(i + 1).toFloat();
          break;
        }
      }


      if (mintemp > 1) {
        writeFloat(Minaddr, mintemp);
      }
      if (maxtemp > 1) {
        writeFloat(Maxaddr, maxtemp);
      }
      MinTemp = readFloat(Minaddr);
      MaxTemp = readFloat(Maxaddr);
      PrintMaxMinTemp();
    }

  } // if available



  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    currentTemp = measureTemp();
    Serial.print("Current Tempt: ");
    Serial.println(currentTemp);
    mySerial.print("BT:Current Tempt: ");
    mySerial.println(currentTemp);

    if (!isbatteryfull) {
      PrintMaxMinTemp();
      Serial.println("Battery not full");
      mySerial.println("Battery not full");
      if (currentTemp > MaxTemp) {
        SwitchOff();
      }
      if (currentTemp < MinTemp) {
        SwitchOn();
      }
    } else {
      Serial.println("Battery full");
      mySerial.println("Battery full");
      SwitchOff();
    }
  }


}

//////////////////////////EEPROM/////////////////////

void writeFloat(char add, float floatdata)
{
  String data = String(floatdata);
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.update(add + i, data[i]);
  }
  EEPROM.update(add + _size, '\0'); //Add termination null character for String Data
}


float readFloat(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 500) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
    if (len > 30) {
      break;
    }
  }
  Serial.print("Data:");
  Serial.println(data);
  data[len] = '\0';
  float mdata = atof(data);
  Serial.print("Float Data:");
  Serial.println(mdata);
  if (mdata < 1) {
    mdata = DEFAULT_TEMP;
  }
  return mdata;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void SwitchOff() {
  digitalWrite(Switch1, HIGH);
  digitalWrite(Switch2, LOW);
  Serial.println("Turn off");
  mySerial.println("BT:Turn off");
}


void SwitchOn() {
  digitalWrite(Switch1, LOW);
  digitalWrite(Switch2, HIGH);
  Serial.println("Turn on");
  mySerial.println("BT:Turn on");
}

///////////////////////////MEASURUING_TEMP/////////////////////////////////////////


float measureTemp() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15;
  return Tc;
}

///////////////////////////////////Printing Max Min Temp//////////////////////////////////
void PrintMaxMinTemp() {
  Serial.print("Readed Min tempt: ");
  Serial.println(MinTemp);
  mySerial.print("BT:Readed Min tempt: ");
  mySerial.println(MinTemp);
  Serial.print("Readed Max tempt: ");
  Serial.println(MaxTemp);
  mySerial.print("BT:Readed Max tempt: ");
  mySerial.println(MaxTemp);
}

