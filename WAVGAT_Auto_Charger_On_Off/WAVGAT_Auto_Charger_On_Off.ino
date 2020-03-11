
#include <SoftwareSerial.h>
#include <EEPROM.h>

const int addr = 10; //lets write at address 10
float minTempVal = 55;
float maxTempVal = 60;


SoftwareSerial mySerial(2, 3); // RX, TX
const int Switch1 = 9;
const int Switch2 = 13;

boolean switch_state = false;
boolean isbatteryfull = false;


//////////////////////////Temp variable//////////////////////////

int ThermistorPin = A0;//The Analogue pin where the thermistor is connected
int Vo;
float R1 = 1000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float current_temp = 0;

/////////for print without delay///////////////
unsigned long printable_milli = 0;

///////////////////////////////////////////////

void setup()
{
  // Open serial communications and wait for port to open:
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Hello, world?");
  pinMode(13, OUTPUT);
  readFloat(addr);//lets read and set from EEPROM data
}

void loop() // run over and over
{
  current_temp = measureTemp();//Lets measure the current tempt
  if (mySerial.available()) {
    String input = mySerial.readString();
    mySerial.println("Recieved: ");
    mySerial.println(input);

    if (input.indexOf(",") < 0) {
      if (input.indexOf("_full_") > -1)  {
        isbatteryfull = true;
      }
      if (input.indexOf("_low_") > -1) {
        isbatteryfull = false;
      }
    } else {
      int len = input.length();
      if ( len < 15) {
        float mintemp;
        float maxtemp;
        for (int i = 0; i < len; i++) {
          if (input.substring(i, i + 1) == ",") {
            mintemp = stringTofloat(input.substring(0, i));
            maxtemp = stringTofloat(input.substring(i + 1));
            break;
          }
        }
        mySerial.println(mintemp);
        mySerial.println(maxtemp);
        minTempVal = mintemp;
        maxTempVal = maxtemp ;
        /////save the string directly to EEPROM/////
        char inputchar[50];
        input.toCharArray(inputchar, 50);
        int _size = sizeof(inputchar);
        for (int i = 0; i < _size; i++)
        {
          EEPROM.update(addr + i, inputchar[i]);
        }
        EEPROM.update(addr + _size, '\0'); //Add termination null character for String Data
      } else {
        MyPrint("Argument too long");
      }
    }
  }//if serial is available
  //
  //  if (isbatteryfull) {
  //    switch_state = false;
  //  } else {
  //    if (current_temp < minTempVal) {
  //      switch_state = true;
  //    }
  //    if (current_temp > maxTempVal) {
  //      switch_state = false;
  //    }
  //  }
  //  if (switch_state) {
  //    SwitchOn();
  //  } else {
  //    SwitchOff();
  //  }

  unsigned long cur_milli = millis();
  if (cur_milli - printable_milli > 1000) {

    /////////////////////////////////////////////////////////////////
    if (isbatteryfull) {
      switch_state = false;
    } else {
      if (current_temp < minTempVal) {
        switch_state = true;
      }
      if (current_temp > maxTempVal) {
        switch_state = false;
      }
    }
    if (switch_state) {
      SwitchOn();
    } else {
      SwitchOff();
    }

    ///////////////////////////////////////////////////////////////////

    printable_milli = cur_milli;
    mySerial.println();
    mySerial.print("Min Temp:");
    mySerial.println(minTempVal);
    mySerial.print("Max Temp:");
    mySerial.println(maxTempVal);
    mySerial.print("Current Temp:");
    mySerial.println(current_temp);
    mySerial.print("Batter Status:");
    if (isbatteryfull) {
      mySerial.println("Full");
    } else {
      mySerial.println("Not Full");
    }
    if (switch_state) {
      MyPrint("Turn on");
    } else {
      MyPrint("Turn off");
    }
    mySerial.println();
  }
}
///////////////////////UTILITY///////////////////////
float stringTofloat(String value) {
  char buf[10];
  value.toCharArray(buf, value.length() + 1);
  return atof(buf);
}

void readFloat(char add)
{
  int CHARLEN = 25;
  char data[CHARLEN]; //Max 100 Bytes
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
  data[len] = '\0';
  char *mintem = strtok(data, ",");
  char *maxtem = strtok(NULL, ",");
  minTempVal = atof(mintem);
  maxTempVal = atof(maxtem);
}
/////////////////////////////////////////////////////////////////////////////////////////////

void SwitchOff() {
  digitalWrite(Switch1, HIGH);
  digitalWrite(Switch2, LOW);
}

void SwitchOn() {
  digitalWrite(Switch1, LOW);
  digitalWrite(Switch2, HIGH);
}

/////////////////////////////////

void MyPrint(char* tag) {
  mySerial.print("Log: ");
  mySerial.println(tag);
}


float measureTemp() {
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (3967.0 / (float)Vo - 1.0);//3968 is max
  //  mySerial.print("Analog: ");
  //  mySerial.println(Vo);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15;
  return Tc;
}
