// Simple I2C test for ebay 128x64 oled.
// Use smaller faster AvrI2c class in place of Wire.
//
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include "OneButton.h"
#include "Konwerter.h"
#include <aircraft.h>
#include <mavlink.h>

#include "Screen.hpp"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

const int numOfScreens = 3;

Screen * screens[numOfScreens];

int screenNr = 0;

int updateTimer = 0;

SSD1306AsciiAvrI2c oled;
OneButton button(7, true);
OneButton button2(6, true);


//------------------------------------------------------------------------------
void setup() {

  Screen * s1 = new Screen1();
  screens[0] = s1;

  Screen * s2 = new Screen2();
  screens[1] = s2;

  Screen * s3 = new Screen3();
  screens[2] = s3;



  button.attachDoubleClick(doubleclick);
  button2.attachDoubleClick(doubleclick2);


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
  Serial.begin(57600);
}
//------------------------------------------------------------------------------
void loop() {

	button.tick();
	button2.tick();

	if(Serial.available() > 0){
		char c = Serial.read();
		//oled.print(c);
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

	oled.println("Zelwa Wigrance");

	if(screenNr < numOfScreens-1)
		screenNr++;
	else
		screenNr = 0;

} // doublec


void doubleclick2() {
	oled.clear();

	oled.println("Dubowo");


	if(screenNr > 0)
		screenNr--;
	else
		screenNr = numOfScreens-1;


} // doublec

