/*
 * Screen.hpp
 *
 *  Created on: May 1, 2016
 *      Author: maciek
 */

#ifndef SCREEN_HPP_
#define SCREEN_HPP_

extern SSD1306AsciiAvrI2c oled;

extern int screenNr;

class Screen {

public:
	virtual void display(){
		oled.clear();
	}

	virtual void displayM() {

		//oled.clear();
		oled.setRow(0);
		oled.setCol(0);

		oled.println(screenNr);
		display();
	}

};

class Screen1: public Screen {

public:
	virtual void display() {

		oled.setRow(1);
		oled.setCol(2);
		oled.println("jestem 1");

	}
};

class Screen2: public Screen {
public:
	virtual void display() {


		oled.setRow(1);
		oled.setCol(2);
		oled.println("jestem2");



	}

};


class Screen3: public Screen {
public:
	virtual void display() {


		oled.setRow(1);
		oled.setCol(2);
		oled.println("jestem3");



	}

};


#endif /* SCREEN_HPP_ */
