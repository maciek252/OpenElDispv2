// Simple I2C test for ebay 128x64 oled.
// Use smaller faster AvrI2c class in place of Wire.
//
#include <Arduino.h>
#include <aircraft.h>
#include <mavlink.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include "OneButton.h"
#include "Konwerter.h"

#include "Screen.hpp"

#include <extEEPROM.h>    //http://github.com/JChristensen/extEEPROM/tree/dev
#include <Streaming.h>    //http://arduiniana.org/libraries/streaming/
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire


// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

const int numOfScreens = 3;

#undef DEBUG


Screen * screens[numOfScreens];

int screenNr = 0;

int updateTimer = 0;

SSD1306AsciiAvrI2c oled;
OneButton button(7, true);
OneButton button2(6, true);

struct eLeReS_data eLeReS;

#ifdef DEBUG
char text[50];
#endif

//zmienne do sprzwdzwnia czy dalej nadawane są te parametry
unsigned int RSSI_OK;
unsigned int RCQ_OK;
unsigned int uRX_OK;
unsigned int tRX_OK;
unsigned int STX_OK;
unsigned int aRX_OK;
unsigned int uTX_OK;
unsigned int tTX_OK;
unsigned int P_OK;
unsigned int TRYB_OK;
unsigned int HD_OK;
unsigned int FIX_OK;
unsigned int SAT_OK;
unsigned int KURS_OK;
unsigned int v_OK;
unsigned int h_OK;
unsigned int Pos_OK;

String str;
char text[50];


String getValue(String data, char separator, int index)
{
  String result = "";
  int found = 0;
  int strIndex[] = {0, -1  };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  if( found > index)
	  result = data.substring(strIndex[0], strIndex[1]);
  return result;
}



int ObliczFuel() //konwersja napięcia pakietu na wskaźnik fuel
{
  float fuel;
  float BAT_MIN = 85;
  float BAT_MAX = 126;

  fuel = eLeReS.uRX - BAT_MIN;
  fuel = fuel * (100 / (BAT_MAX - BAT_MIN));
  if (fuel < 0) fuel = 0;
  return fuel;
}

bool readLRS(char c) //czytanie eLeReSa obliczenia i pakowanie do tablicy
{
  String tmp;
  String wynik;
  String nazwa;
  String wartosc;
  float lat;
  float lon;

  str += c;
  if(str.length() > 40){
 	  str = "";
   }
  if(c == '\n'){
	  Serial.write("nowa linia! str len=" + str.length());
  } else
	  return false;


//#ifdef DEBUG
          RSSI_OK = 0; //zerowanie licznika poprawności RSSI
    Serial.print("rcv-eLeReS.Full_string: ");
    Serial.println(str);
//#endif
    str.replace(", ", ",");
    str.replace("\r", "");
    //blink1();
    for (uint8_t x = 0; x < 10; x++)
    {
      String xval = getValue(str, ' ', x); //wydzielenie pary parametr=wartosc
      if (xval == NULL){
    	  Serial.println("xval NULL for x=" + x);
      }
      else
      {
        nazwa = getValue(xval, '=', 0);
        wartosc = getValue(xval, '=', 1);

        Serial.println("nazwa=" + nazwa);
        Serial.println("wartosc=" + wartosc);

        if ((nazwa == "RSSI" or nazwa == "SSI") and wartosc.length() == 3) {
          eLeReS.RSSI = wartosc.toInt();
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.RSSI:%d ", eLeReS.RSSI);
          Serial.print(text);
#endif
        } else if (nazwa == "RCQ" and wartosc.length() == 3) {
          eLeReS.RCQ = wartosc.toInt() *2;
          RCQ_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.RCQ:%d ", eLeReS.RCQ);
          Serial.print(text);
#endif
        }
       else if (nazwa == "RCQ" and wartosc.length() == 3) {
        eLeReS.RCQ = wartosc.toInt() *2;
        RCQ_OK = 0;
#ifdef DEBUG
        sprintf(text, "rcv-eLeReS.RCQ:%d ", eLeReS.RCQ);
        Serial.print(text);
#endif
      }
        else if (nazwa == "U" and wartosc.length() == 5) {
          eLeReS.uRX = atof (wartosc.c_str()) * 10;
          uRX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.uRX:%d ", eLeReS.uRX);
          Serial.print(text);
#endif
        } else if (nazwa == "T" and (wartosc.length() == 4 or wartosc.length() == 5)) {
          eLeReS.tRX = wartosc.toInt();
          tRX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.tRX:%d ", eLeReS.tRX);
          Serial.print(text);
#endif
        } else if (nazwa == "I" and wartosc.length() == 5) {
          eLeReS.aRX = atof (wartosc.c_str()) * 10;
          aRX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.aRX:%d ", eLeReS.aRX);
          Serial.print(text);
#endif
        } else if (nazwa == "UTX" and wartosc.length() == 5) {
          eLeReS.uTX = atof (wartosc.c_str()) * 10;
          uTX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.uTX:%d ", eLeReS.uTX);
          Serial.print(text);
#endif
        } else if (nazwa == "STX" and wartosc.length() == 3) {
          eLeReS.STX = wartosc.toInt();
          STX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.STX:%d ", eLeReS.STX);
          Serial.print(text);
#endif
        } else if (nazwa == "TTX" and (wartosc.length() == 4 or wartosc.length() == 5)) {

          wartosc = wartosc.substring(0,wartosc.length()-3);
//          Serial.println("wartosc skr:" + wartosc);

          eLeReS.tTX = wartosc.toInt();
          tTX_OK = 0;
          //Serial.println("MSttx=" + eLeReS.tTX);
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.tTX:%d ", eLeReS.tTX);
          Serial.print(text);
#endif
          screens[0]->setText("ttx=" + eLeReS.tTX);
          screens[1]->setText("ttx=" + eLeReS.tTX);
          oled.print(eLeReS.tTX);
        } else if (nazwa == "P" and wartosc.length() == 5) {
          eLeReS.P = (atof (wartosc.c_str()) + 50000) / 100;
          P_OK = 0;
#ifdef DEBUG
          Serial.print("rcv-eLeReS.P:");
          Serial.print(eLeReS.P);
          Serial.print(" ");
#endif
        } else if (nazwa == "F" and wartosc.length() == 2) {
          eLeReS.TRYB = wartosc.toInt();
          TRYB_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.TRYB:%d ", eLeReS.TRYB);
          Serial.print(text);
#endif
        } else if (nazwa == "HD" and wartosc.length() == 4) {
          eLeReS.HD = atof (wartosc.c_str()) * 10; //?
          HD_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.HD:%d ", eLeReS.HD);
          Serial.print(text);
#endif
        } else if (nazwa == "f" and wartosc.length() == 1) {
          eLeReS.FIX = wartosc.toInt();
          FIX_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.FIX:%d ", eLeReS.FIX);
          Serial.print(text);
#endif
        } else if (nazwa == "s" and wartosc.length() == 2) {
          eLeReS.SAT = wartosc.toInt();
          SAT_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.SAT:%d ", eLeReS.SAT);
          Serial.print(text);
#endif
        } else if (nazwa == "c" and wartosc.length() == 3) {
          eLeReS.KURS = wartosc.toInt();
          KURS_OK = 0;
//#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.c:%d ", eLeReS.KURS);
          Serial.print(text);
//#endif
        } else if (nazwa == "v" and wartosc.length() == 3) {
          eLeReS.v = wartosc.toInt();
          v_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.v:%d ", eLeReS.v);
          Serial.print(text);
#endif
        } else if (nazwa == "h" and wartosc.length() == 4) {
          eLeReS.h = wartosc.toInt();
          h_OK = 0;
#ifdef DEBUG
          sprintf(text, "rcv-eLeReS.h:%d ", eLeReS.h);
          Serial.print(text);
#endif
        } else if ((nazwa == "Pos" or nazwa == "os") and wartosc.length() == 22) {
          //pobranie lattitude
          sprintf(text, "rcv-eLeReS.LonA:%d ", eLeReS.LonA);
          tmp = getValue(wartosc, ',', 0);
          lat = atof (tmp.c_str());
          eLeReS.LatB = (uint16_t)lat;
          lat = (lat - (float)eLeReS.LatB) * 60.0;
          eLeReS.LatB = eLeReS.LatB * 100 + (uint16_t)lat;
          eLeReS.LatA = (uint16_t)round((lat - (uint16_t)lat) * 10000.0);
          //pobranie longtitude
          tmp = getValue(wartosc, ',', 1);
          lon = atof (tmp.c_str());
          eLeReS.LonB = (uint16_t)lon;
          lon = (lon - (float)eLeReS.LonB) * 60.0;
          eLeReS.LonB = eLeReS.LonB * 100 + (uint16_t)lon;
          eLeReS.LonA = (uint16_t)round((lon - (uint16_t)lon) * 10000.0);
          Pos_OK = 0;
#ifdef DEBUG
  screens[1] = s2;
          sprintf(text, "rcv-eLeReS.LatB:%d ", eLeReS.LatB);
          Serial.print(text);
          sprintf(text, "rcv-eLeReS.LatA:%d ", eLeReS.LatA);
          Serial.print(text);
          sprintf(text, "rcv-eLeReS.LonB:%d ", eLeReS.LonB);
          Serial.print(text);
          Serial.print(text);
#endif
        }
      }
    }

    eLeReS.FUEL = ObliczFuel();
    float sealevelPressure = 101325;
    if (eLeReS.P == 0)
    {
      eLeReS.b_h_B=0;
      eLeReS.b_h_A=0;
    }
    else
    {
      float alt = 44330 * (1.0 - pow((eLeReS.P * 100) / sealevelPressure, 0.1903));
      eLeReS.b_h_B = (uint16_t)alt;
      eLeReS.b_h_A = abs((int16_t)round((alt - eLeReS.b_h_B) * 100.0));
    }

    str = "";

#ifdef DEBUG
    Serial.println();
#endif
    return true;
  }



//------------------------------------------------------------------------------
void setup() {

  Screen * s1 = new Screen1();
  screens[0] = s1;


  Screen * s2 = new Screen2();
  screens[1] = s2;


  Screen * s3 = new Screen3();
  screens[2] = s3;



#if 1
  button.attachDoubleClick(doubleclick);
  button2.attachDoubleClick(doubleclick2);
#endif

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
/*
  uint32_t m = micros();
  oled.clear();
  oled.println("MRUKVA Hello world!");
  oled.println("MRUKVA A long line may be truncated");
  oled.println();
  oled.set2X();
  oled.println("2X demo");
  oled.set1X();
  oled.print("\nmicros: ");
  oled.print(micros() - m);
  */

  screens[0]->setText("oko");

  //DefineSerialPort(Serial,0);

  Serial.begin(57600);
}
//------------------------------------------------------------------------------
void loop() {

#if 1
	button.tick();
	button2.tick();
#endif

	while(Serial.available() > 0){
		uint8_t c = Serial.read();
		//screens[0]->setText("serial");

		//oled.print(c);
		//Serial.write(c);
		//oled.write(c);

#if 1
		if(read_mavlink(c)){


			String s= "mavl";
			if(the_aircraft.attitude.roll> 30.0)
				s = "MAVL";
			screens[0]->setText(s);

		} else if(readLRS(c)){

		}
#endif

	}

	//if(millis() - updateTimer> 1000)
	if(true)
	{
		updateTimer = millis();
		screens[screenNr]->displayM();
	}

}

void doubleclick() {
	oled.clear();

	oled.println("Zelwa Wigrancce");

	if(screenNr < numOfScreens-1)
		screenNr++;
	else
		screenNr = 0;

} // doublec


void doubleclick2() {
	oled.clear();

	oled.println("Dubowo");


	screens[0]->setText("Sumowo");

	if(screenNr > 0)
		screenNr--;
	else
		screenNr = numOfScreens-1;


} // doublec

