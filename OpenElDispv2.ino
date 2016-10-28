// Simple I2C test for ebay 128x64 oled.
// Use smaller faster AvrI2c class in place of Wire.
//
#include <Arduino.h>
#include <aircraft.h>
#include <mavlink.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#include "InterFormatInterface.hpp"

#include "OneButton.h"
#include "Konwerter.h"

#include "Screen.hpp"

#include <extEEPROM.h>    //http://github.com/JChristensen/extEEPROM/tree/dev
#include <Streaming.h>    //http://arduiniana.org/libraries/streaming/
#include <Wire.h>         //http://arduino.cc/en/Reference/Wire

#include "FrskyMS.h"

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

bool InterFormatInterface::baseSavedFlag = false;

const int numOfScreens = 3;

#undef DEBUG

extern TelemetryMode telemetryMode;

FrskyMS frskyMS;

long telemetryModeTimer;
long resetDataTimer;
const int resetDataTimeout = 1000;
const int telemetryModeTimeout = 8000;

extern long headingGroupTimer;

Screen * screens[numOfScreens];

int screenNr = 0;
bool stuffed = false;
int updateTimer = 0;

SSD1306AsciiAvrI2c oled;
OneButton button(7, true);
OneButton button2(6, true);

struct eLeReS_data eLeReS;

#ifdef DEBUG
//char text[50];
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
const int frskyBufferLen = 200;
static byte buffer[frskyBufferLen];
static int idx = 0;

String getSubstringDeterminedBySpace(String &data) {

	int i = data.indexOf(' ');
	if (i == -1) {
		//return "";
		String tmp = data;
		data = "";
		return tmp;
	}
	String result = data.substring(0, i);
	data = data.substring(i + 1);
	return result;
}

String getValue(String data, char separator, int index) {
	String result = "";
	int found = 0;
	int strIndex[] = { 0, -1 };
	int maxIndex = data.length() - 1;
	for (int i = 0; i <= maxIndex && found <= index; i++) {
		if (data.charAt(i) == separator || i == maxIndex) {
			found++;
			strIndex[0] = strIndex[1] + 1;
			strIndex[1] = (i == maxIndex) ? i + 1 : i;
		}
	}
	if (found > index)
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
	if (fuel < 0)
		fuel = 0;
	return fuel;
}

bool decode_frame(byte *buffer, int length) {
	int i = 1;

	bool result = false;
	int16_t alt_a, gps_alt_a, gps_course_a, gps_lat_a, gps_long_a, gps_speed_a,
			voltage_a;
	static int16_t alt_b, gps_alt_b, gps_course_b, gps_lat_b, gps_long_b,
			gps_speed_b, voltage_b;
	int vv = 0;

	while (i < length) {

		switch (buffer[i]) {

		case FRSKY_D_ACCX:
			//Serial << F("AccX:       ") << frskyMS.decodeInt (&buffer[i+1]) / 1000.0 << " [g]" << endl;
			result = true;
			break;

		case FRSKY_D_ACCY:
			//Serial << F("AccY:       ") << frskyMS.decodeInt (&buffer[i+1]) / 1000.0 << " [g]" << endl;
			result = true;
			break;

		case FRSKY_D_ACCZ:
			//Serial << F("AccZ:       ") << frskyMS.decodeInt (&buffer[i+1]) / 1000.0 << " [g]" << endl;
			result = true;
			break;

		case FRSKY_D_ALT_B:
			alt_b = frskyMS.decodeInt(&buffer[i + 1]);

			//                        Serial << F("--- skip GPS_ALT_B") << endl;
			result = true;
			break;

		case FRSKY_D_ALT_A:
			alt_a = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("Alt:        ") << frskyMS.calcFloat (alt_b, alt_a) << F(" [m]") << endl;
			result = true;
			break;

		case FRSKY_D_CELL_VOLT:
			//Serial << F("CellV[") << frskyMS.decodeCellVoltId(&buffer[i + 1])
			//<< "]:   " << frskyMS.decodeCellVolt(&buffer[i + 1])
			//<< " [V]" << endl;
			result = true;
			break;

		case FRSKY_D_FUEL:
			//Serial << F("Fuel:       ") << frskyMS.decodeInt(&buffer[i + 1])
			//<< F(" [%]") << endl;
			result = true;
			break;

		case FRSKY_D_GPS_ALT_B:
			gps_alt_b = frskyMS.decodeInt(&buffer[i + 1]);
			//Serial << F("--- skip GPS_ALT_B") << endl;
			result = true;
			break;
		case FRSKY_D_GPS_ALT_A:
			gps_alt_a = frskyMS.decodeInt(&buffer[i + 1]);
			//Serial 	<< F(
			//"GpsAlt:     ") << frskyMS.calcFloat (gps_alt_b, gps_alt_a) << F(" [m] << endl");
			result = true;
			break;

		case FRSKY_D_GPS_COURSE_B:
			gps_course_b = frskyMS.decodeInt(&buffer[i + 1]);
			//Serial << F("--- skip GPS_COURSE_B") << endl;
			eLeReS.KURS = gps_course_b;
			result = true;
			break;

		case FRSKY_D_GPS_COURSE_A:
			gps_course_a = frskyMS.decodeInt(&buffer[i + 1]);
			//Serial << F("GpsCourse:  ") << frskyMS.calcFloat (gps_course_b, gps_course_a) << " [" << char(176) << "]" << endl;
			//eLeReS.KURS = frskyMS.calcFloat (gps_course_b, gps_course_a);
			result = true;
			break;

		case FRSKY_D_GPS_DM:
			//Serial << F("Day, Month: ") << frskyMS.decode1Int (&buffer[i+1]) << " " << frskyMS.decode1Int (&buffer[i+2]) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_HM:
			//Serial << F("Hour, Min:  ") << frskyMS.decode1Int (&buffer[i+1]) << " " << frskyMS.decode1Int (&buffer[i+2]) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LAT_B:
			gps_lat_b = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("--- skip GPS_LAT_B") << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LAT_A:
			gps_lat_a = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("GpsLat:     ") << frskyMS.decodeGpsLat (gps_lat_b, gps_lat_a) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LAT_NS:
//   	  Serial << F("GpsLatNS:   ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LONG_B:
			gps_long_b = frskyMS.decodeInt(&buffer[i + 1]);
			//                                Serial << F("--- skip GPS_LONG_B") << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LONG_A:
			gps_long_a = frskyMS.decodeInt(&buffer[i + 1]);

			//Serial << "GpsLong:    "
			//	<< frskyMS.decodeGpsLong(gps_long_b, gps_long_a) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_LONG_EW:
			//Serial << F("GpsLongEW:  ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_SEC:
			//Serial << F("Sec:        ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			result = true;
			break;

		case FRSKY_D_GPS_SPEED_B:
			gps_speed_b = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("--- skip GPS_SPEED_B") << endl;
			result = true;
			break;

		case FRSKY_D_GPS_SPEED_A:
			gps_speed_b = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("GpsSpeed:   ") << frskyMS.calcFloat (gps_speed_b, gps_speed_a) << F(" [knots]") << endl;
			result = true;
			break;

		case FRSKY_D_GPS_YEAR:
			//Serial << F("Year:       ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			result = true;
			break;

		case FRSKY_D_RPM:
			//Serial << F("Rpm:        ") << frskyMS.decodeInt (&buffer[i+1]) << F(" [rpm]") << endl;
			result = true;
			break;

		case FRSKY_D_TEMP1:
			vv = frskyMS.decodeInt(&buffer[i + 1]);
			//Serial << F("Temp1:      ") << vv << " [" << char(176) << "C]" << endl;
			eLeReS.uTX = vv;
			result = true;
			break;

		case FRSKY_D_TEMP2:
			//  Serial << F("Temp2:      ") << frskyMS.decodeInt (&buffer[i+1]) << " [" << char(176) << "C]" << endl;
			result = true;
			break;

		case FRSKY_D_CURRENT:
			//Serial << F("Current:    ") << frskyMS.decodeInt (&buffer[i+1]) << " [A]" << endl;
			result = true;
			break;

		case FRSKY_D_VFAS:
			//Serial << F("VFAS:       ") << frskyMS.decodeInt (&buffer[i+1]) / 10 << " [V]" << endl;
			result = true;
			break;

		case FRSKY_D_VOLTAGE_B:
			voltage_b = frskyMS.decodeInt(&buffer[i + 1]);

			//                          Serial << F("--- skip VOLTAGE_B") << endl;
			result = true;
			break;

		case FRSKY_D_VOLTAGE_A:
			voltage_a = frskyMS.decodeInt(&buffer[i + 1]);
			;

			//                           Serial << F("Voltage:    ") << (float) (voltage_b * 10 + voltage_a) * 21 / 110 << " [V]" << endl;

			result = true;
			break;

		default:
			// Serial << F("unknown ID:    ") << _HEX(buffer[i]) << endl;
			//Serial << F("decodeInt:     ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			//Serial << F("decode1Int[0]: ") << frskyMS.decodeInt (&buffer[i+1]) << endl;
			//Serial << F("decode1Int[1]: ") << frskyMS.decodeInt (&buffer[i+2]) << endl;
			//Serial.print (F("HEX: "));
			while (buffer[i] != 0x5E) {
				//Serial << _HEX(buffer[i]) << " ";
				i++;
			}
			i--;
			//Serial << endl;
		}
		i++;
		while (buffer[i - 1] != 0x5E)
			i++;
	}
	return result;
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
	if (str.length() > 50) {
		str = "";
	}
	if (c == '\n') {
#ifdef DEBUG
		Serial.println("nowa linia! str len=" + str.length());
#endif
	} else
		return false;

//#ifdef DEBUG
	RSSI_OK = 0; //zerowanie licznika poprawności RSSI
#ifdef DEBUG
			Serial.print("rcv-eLeReS.Full_string: ");
			Serial.println(str);
#endif
	str.replace(", ", ",");
	str.replace("\r", "");
	//blink1();
	//for (uint8_t x = 0; x < 10; x++)
	String res = "";
	do {
		Serial.println("str0=" + str);
		res = getSubstringDeterminedBySpace(str);
		if (res == NULL) {
			Serial.println(F("res = NULL"));
		} else
			Serial.println("res = " + res);
#ifdef DEBUG
		Serial.println("str=" + str);
		//continue;
#endif

		String para = res;
		if (res == NULL)
			para = str;
		int p = para.indexOf('=');
		if (p != -1) {
			String nazwa = para.substring(0, p);
			String wartosc = para.substring(p + 1);
#ifdef DEBUG
			Serial.println("n/w=" + nazwa + " " + wartosc + " lenw=" + wartosc.length());
#endif
			if (nazwa == "c" || nazwa == "Pos" || nazwa == "s" || nazwa == "TTX"
					|| nazwa == "STX" || nazwa == "UTX" || nazwa == "F")
				parseEleresPair(nazwa, wartosc);
		}

	} while (res != "" && res != NULL);

	str = "";

#ifdef DEBUG
	Serial.println();
#endif
	return true;
}

bool parseEleresPair(String nazwa, String wartosc) {

	String tmp;
	String wynik;

	float lat;
	float lon;

	if (nazwa == "STX" && wartosc.length() == 3) {

		//eLeReS.tTX = wartosc.substring(0,2).toInt();

		//eLeReS.STX = 8;
		eLeReS.STX = wartosc.toInt();
		//KURS_OK = 0;

	} else if (nazwa == "TTX"
	/*&& wartosc.length() == 2*/) {

		eLeReS.tTX = wartosc.substring(0, 2).toInt();

		//eLeReS.tTX = 9;
		//KURS_OK = 0;

	} else if (nazwa == "F" //&& wartosc.length() == 2
			) {
		//headingGroupTimer = millis();
		eLeReS.TRYBString = wartosc;
		//eLeReS.TRYB = wartosc.toInt();
		//KURS_OK = 0;

	} else if (nazwa == "P" && wartosc.length() == 5) {

		eLeReS.P = wartosc.toInt();
		//KURS_OK = 0;
		eLeReS.KURSset = true;
	} else if (nazwa == "UTX" && wartosc.length() == 5) {

		wartosc = wartosc.substring(0, 3);
		eLeReS.uTX = wartosc.toInt();
		//KURS_OK = 0;
		eLeReS.KURSset = true;
	} else if (nazwa == "c" && wartosc.length() == 3) {
		headingGroupTimer = millis();

		eLeReS.KURS = wartosc.toInt();
		//KURS_OK = 0;
		eLeReS.KURSset = true;
	} else if (nazwa == "s" and wartosc.length() == 2) {
		eLeReS.SAT = wartosc.toInt();
		SAT_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.SAT:%d ", eLeReS.SAT);
		Serial.print(text);
#endif
	} else if ((nazwa == "Pos" or nazwa == "os") and wartosc.length() == 22) {
		//pobranie lattitude
		//sprintf(text, "rcv-eLeReS.LonA:%d ", eLeReS.LonA);

		//54.023834N,023. 45317 0E

		tmp = //getValue(wartosc, ',', 0);
				wartosc.substring(0, 9);
		lat = atof(tmp.c_str());
#ifdef DEBUG
		Serial.print("lat=");
		Serial.println(lat,5);
#endif
		eLeReS.LatB = (uint16_t) lat;
		lat = (lat - (float) eLeReS.LatB) * 60.0;
		//eLeReS.LatB = eLeReS.LatB * 100 + (uint16_t) lat;
		eLeReS.LatB = (long) (tmp.toFloat() * 100.0);
		eLeReS.LatA = (uint16_t) round((lat - (uint16_t ) lat) * 10000.0);
		//pobranie longtitude
		//tmp = getValue(wartosc, ',', 1);
		tmp = wartosc.substring(11, 17);
		lon = atof(tmp.c_str());
		eLeReS.LonB = (uint16_t) lon;
		lon = (lon - (float) eLeReS.LonB) * 60.0;
		eLeReS.LonB = eLeReS.LonB * 100 + (uint16_t) lon;
		eLeReS.LonA = (uint16_t) round((lon - (uint16_t ) lon) * 10000.0);
		Pos_OK = 0;
#ifdef DEBUG
		//screens[1] = s2;
		sprintf(text, "rcv-eLeReS.LatB:%d ", eLeReS.LatB);
		Serial.print(text);
		sprintf(text, "rcv-eLeReS.LatA:%d ", eLeReS.LatA);
		Serial.print(text);
		sprintf(text, "rcv-eLeReS.LonB:%d ", eLeReS.LonB);
		Serial.print(text);
		Serial.print(text);
#endif
	}

	return false;

//	        nazwa = getValue(xval, '=', 0);
//	        wartosc = getValue(xval, '=', 1);

//	        Serial.println("xval:" + xval);
	Serial.println("nazwa=" + nazwa);
	Serial.println("wartosc=" + wartosc);

	if ((nazwa == "RSSI" or nazwa == "SSI") and wartosc.length() == 3) {
		eLeReS.RSSI = wartosc.toInt();
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.RSSI:%d ", eLeReS.RSSI);
		Serial.print(text);
#endif
	} else if (nazwa == "RCQ" and wartosc.length() == 3) {
		eLeReS.RCQ = wartosc.toInt() * 2;
		RCQ_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.RCQ:%d ", eLeReS.RCQ);
		Serial.print(text);
#endif
	} else if (nazwa == "RCQ" and wartosc.length() == 3) {
		eLeReS.RCQ = wartosc.toInt() * 2;
		RCQ_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.RCQ:%d ", eLeReS.RCQ);
		Serial.print(text);
#endif
	} else if (nazwa == "U" and wartosc.length() == 5) {
		eLeReS.uRX = atof(wartosc.c_str()) * 10;
		uRX_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.uRX:%d ", eLeReS.uRX);
		Serial.print(text);
#endif
	} else if (nazwa == "T"
			and (wartosc.length() == 4 or wartosc.length() == 5)) {
		eLeReS.tRX = wartosc.toInt();
		tRX_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.tRX:%d ", eLeReS.tRX);
		Serial.print(text);
#endif
	} else if (nazwa == "I" and wartosc.length() == 5) {
		eLeReS.aRX = atof(wartosc.c_str()) * 10;
		aRX_OK = 0;
#ifdef DEBUG
		sprintf(text, "rcv-eLeReS.aRX:%d ", eLeReS.aRX);
		Serial.print(text);
#endif
	} else if (nazwa == "UTX" and wartosc.length() == 5) {
		eLeReS.uTX = atof(wartosc.c_str()) * 10;
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
	} else if (nazwa == "TTX"
			and (wartosc.length() == 4 or wartosc.length() == 5)) {

		wartosc = wartosc.substring(0, wartosc.length() - 3);
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
		eLeReS.P = (atof(wartosc.c_str()) + 50000) / 100;
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
		eLeReS.HD = atof(wartosc.c_str()) * 10; //?
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
	} else if (nazwa == "c" and wartosc.length() == 3) {
		eLeReS.KURS = wartosc.toInt();
		KURS_OK = 0;
		//#ifdef DEBUG
		//sprintf(text, "rcv-eLeReS.c:%d ", eLeReS.KURS);
		//Serial.print(text);
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
	}
	/*
	 eLeReS.FUEL = ObliczFuel();
	 float sealevelPressure = 101325;
	 if (eLeReS.P == 0) {
	 eLeReS.b_h_B = 0;
	 eLeReS.b_h_A = 0;
	 } else {
	 float alt = 44330
	 * (1.0 - pow((eLeReS.P * 100) / sealevelPressure, 0.1903));
	 eLeReS.b_h_B = (uint16_t) alt;
	 eLeReS.b_h_A = abs((int16_t) round((alt - eLeReS.b_h_B) * 100.0));
	 }

	 }
	 */
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
	//oled.

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

	//oled.clearToEOL();
	oled.clear();
	screens[0]->setText("oko");

	//DefineSerialPort(Serial,0);

	Serial.begin(57600);
}
//------------------------------------------------------------------------------
// LOOP
//------------------------------------------------------------------------------

void loop() {

#if 1
	button.tick();
	button2.tick();
#endif

	if (telemetryDetectionMode == TELEM_SEARCHING
			&& (millis() - detectionTimer > detectionTimerConst)) {
		detectionTimer = millis();
		if (telemetryMode == TELEM_FRSKY) {
			telemetryMode = TELEM_ELERES;
			Serial.end();
			Serial.begin(57600);
			telemetryModeTimer = millis();
		} else if (telemetryMode == TELEM_ELERES) {
			telemetryMode = TELEM_MAVLINK;
			Serial.end();
			Serial.begin(57600);
			telemetryModeTimer = millis();
		} else if (telemetryMode == TELEM_MAVLINK) {
			telemetryMode = TELEM_FRSKY;
			idx = 0;
			Serial.end();
			Serial.begin(9600);
			telemetryModeTimer = millis();
		}

	}

	while (Serial.available() > 0) {
		uint8_t c = Serial.read();
		//screens[0]->setText("serial");

		//oled.print(c);
		//Serial.write(c);
		//oled.write(c);

#if 1
		if (telemetryMode == TELEM_MAVLINK) {
			if (read_mavlink(c)) {

				telemetryDetectionMode = TELEM_WORKING;

				String s = "mavl";
				if (the_aircraft.attitude.roll > 30.0)
					s = "MAVL";
				screens[0]->setText(s);

				telemetryMode = TELEM_MAVLINK;
				telemetryModeTimer = millis();

				headingGroupTimer = millis();
				/*
				 } else if (readLRS(c)) {


				 telemetryMode = TELEM_ELERES;
				 telemetryModeTimer = millis();
				 */
			}
		} else if (telemetryMode == TELEM_FRSKY) {

			if (c == 0x7D){
			        stuffed = true;
			        continue;
			    }
			    if (stuffed){
			        c = c ^ 0x20;
			        stuffed = false;
			    }
			if(c == 0x7E)
				c = 0x5E;

			buffer[idx] = c;		//frskyMS.read ();
			//Serial.write(buffer[idx]);
			if (idx != 0 && buffer[idx - 1] == 0x5E && buffer[idx] == 0x5E) {
				//Serial << "\n" << "--- [FRAME] --- ";
				telemetryDetectionMode = TELEM_WORKING;
				telemetryModeTimer = millis();
				//for (int i = 0; i < idx; i++)
				//Serial << _HEX(buffer[i]) << " ";
				//Serial << " (" << idx << ")" << endl;
				if (decode_frame(buffer, idx) == true) {

				}
				buffer[0] = 0x5E;
				idx = 1;
			} else {
				idx++;
				if (idx > frskyBufferLen) {  // error
					buffer[0] = 0xE;
					idx = 1;
				}
			}

		}
#endif
	}

	if (millis() - telemetryModeTimer > telemetryModeTimeout) {
		telemetryDetectionMode = TELEM_SEARCHING;
		detectionTimer = millis();
	}

	//if(millis() - updateTimer> 1000)
	if (true) {
		updateTimer = millis();
		InterFormatInterface::tick();
		screens[screenNr]->displayM();
	}

}

void doubleclick() {
	oled.clear();

	oled.println("Zelwa Wigrancce");

	if (screenNr < numOfScreens - 1)
		screenNr++;
	else
		screenNr = 0;

} // doublec

void doubleclick2() {
	oled.clear();

	oled.println("Dubowo");

	screens[0]->setText("Sumowo");

	if (screenNr > 0)
		screenNr--;
	else
		screenNr = numOfScreens - 1;

} // doublec

