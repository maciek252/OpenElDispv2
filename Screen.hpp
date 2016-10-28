/*
 * Screen.hpp
 *
 *  Created on: May 1, 2016
 *      Author: maciek
 */

#ifndef SCREEN_HPP_
#define SCREEN_HPP_

#include "InterFormatInterface.hpp"
#include "Utils.hpp"

extern SSD1306AsciiAvrI2c oled;

extern int screenNr;

extern struct eLeReS_data eLeReS;

long headingGroupTimer = 0;

enum TelemetryMode {TELEM_ELERES, TELEM_MAVLINK, TELEM_FRSKY};

enum TelemetryDetectionMode {TELEM_WORKING, TELEM_SEARCHING};

TelemetryMode telemetryMode = TELEM_MAVLINK;
TelemetryDetectionMode telemetryDetectionMode = TELEM_SEARCHING;

long detectionTimer = 0;
const long detectionTimerConst = 1500;


bool flag = false;
long flagTimer = 0;

class Screen {

protected:
	String str;
	bool updateHeadingGroupFlag;


public:

	void setFlags(){
		if(millis() - headingGroupTimer < 2000)
				updateHeadingGroupFlag = true;
			else
				updateHeadingGroupFlag = false;

	}

	virtual void display(){
		setFlags();
		oled.clear();
	}

	virtual void displayM() {
		setFlags();
		//oled.clear();
		oled.setRow(0);
		oled.setCol(0);

		oled.print(screenNr);
		oled.print(" ");


		if(telemetryMode == TELEM_MAVLINK)
			oled.print("MAVLINK");
		else if(telemetryMode == TELEM_ELERES)
					oled.print("ELERES ");
		else if(telemetryMode == TELEM_FRSKY)
							oled.print("FRSKY ");

		if( telemetryDetectionMode == TELEM_SEARCHING){
			oled.print("S");
		} else
			oled.print("W");

		oled.print(" ");
		if(millis() - flagTimer > 1000){
			flagTimer = millis();
			if(flag){
				flag = false;
				oled.print("X");
			} else{
				flag = true;
				oled.print("+");
			}

		}

		display();
	}


	void setText(String str){
		this->str = str;
	}
};

class Screen1: public Screen {

public:
	virtual void display() {

		oled.setRow(1);
		oled.setCol(2);
		oled.println(F("1"));
		oled.println(str);
		oled.print(F("kurs="));
		oled.print("" + InterFormatInterface::getHeading());
		oled.print(F("tryb="));
		oled.println("" + InterFormatInterface::getFlightMode());


		//oled.println(eLeReS.KURS);
		//oled.print(the_aircraft.heading);

		oled.print(F("uTX:"));
		oled.println("" + InterFormatInterface::getUTX());


		oled.print(F("STX:"));
		oled.println("" + InterFormatInterface::getSTX());


		oled.print(F("TTX:"));
		oled.print("" + InterFormatInterface::getTTX());


		oled.print(F(" roll:"));
		oled.println("" + InterFormatInterface::getRoll());
		//if(updateHeadingGroupFlag)
			//oled.println(eLeReS.KURS);
		//else
			//oled.println(F("XXX"));

		oled.print(F("lonB="));
		oled.print(InterFormatInterface::getLatitude());
		//oled.print(eLeReS.LonB);
		oled.print(F("lonA="));
		oled.println(eLeReS.LonA);
		oled.print(F("latB="));
		oled.print(InterFormatInterface::getLongitude());
		oled.print(F("latA="));
		oled.println(eLeReS.LatA);
	}
};

class Screen2: public Screen {
public:
	virtual void display() {


		oled.setRow(1);
		oled.setCol(2);
		oled.println(F("jsem2"));
		oled.print(F("sat="));
		oled.print(InterFormatInterface::numOfSats());
		oled.print(F("fix="));
		oled.print(InterFormatInterface::fixType());

		oled.setCursor(0,5);
		if(InterFormatInterface::baseSaved())
			oled.print("B");
		else
			oled.print("NB");


	}

};


class Screen3: public Screen {
public:
	virtual void display() {

		oled.setRow(1);
		oled.setCol(2);
		oled.println(F("somb3"));

	}

};


#endif /* SCREEN_HPP_ */
