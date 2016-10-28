/*
  @author   Scott

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef INTERFORMATINTERFACE_HPP
#define INTERFORMATINTERFACE_HPP

// --- konfiguracja
#define DEBUG
#define Baud_eLeReS 57600 //58823 realna predkosc eLeReSa?
#define Pin_eLeReS 3
#define Pin_Led 13
// --- koniec konfiguracji

void blink1();
String getValue(String data, char separator, int index);
int ObliczFuel();
void Czysc_eLeReS();

struct eLeReS_data
{
  int RSSI;
  int RCQ;
  int uRX;
  int aRX;
  int tRX;
  float P;
  int TRYB;
  String TRYBString;
  int uTX;
  int STX;
  int tTX;
  int HD;
  int FIX;
  int SAT;
  int KURS;
  bool KURSset;
  int v;
  int h;
  int b_h_B;
  int b_h_A;
  int16_t LatB;
  uint16_t LatA;
  int16_t LonB;
  uint16_t LonA;
  int FUEL;
};

// --- eLeReS
//CH to wartosci 8 kanalow RC z nadajnika w HEX 00-FF, 80 srodek.
//P to ciśnienie z baro. W 0.01mbar + 50000. Czyli 48933 = 989.33mbar
//Deb to wartosci zmiennych debug z MuliWii.
//Pos to pozycja GPS
//f to FIX GPS 0=brak, 1=2D fix, 2=3D fix
//s to ilosc satelit
//c to kurs w stopniach
//v to predkosc w km/h
//h to wysokosc w metrach
//UTX to napiecie baterii nadajnika RC

//F to tryb lotu MuliWii.
//MANUAL = 0x0A
//STAB = 0x01
//6- RTH
//LEVEL = 0x02
//ALTHOLD = 0x03
//RTH = 0x06
//POSHOLD = 0x05
//RTH_DSC = 0x08
//FAILSAFE = 0x07
//
//Do tego dodanie 0x10 oznacza ARMED, a 0x20 zapisanie HOME.
//
//A tak wogóle to tryby jakie potrafi wyświetlić eLDisp:
//1 - ACRO
//2- LEVEL
//3- BARO
//4- HesdFree
//5- PosHild
//7- FailSafe
//8- RTH_DSC (TH)
//9- wolne
//A- Manual
//B- GPS
//C- ATTI
//D- Course Lock
//E- POI Lock
//F- Home Lock

#endif
