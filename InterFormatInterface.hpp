/*
 * InterFormatInterface.hxx
 *
 *  Created on: Jul 19, 2016
 *      Author: maciek
 */

#ifndef INTERFORMATINTERFACE_HPP_
#define INTERFORMATINTERFACE_HPP_

#include "Konwerter.h"


#include <aircraft.h>
#include <mavlink.h>
#include "Utils.hpp"

extern struct eLeReS_data eLeReS;



class InterFormatInterface {

public:
	double heading;

	static const boolean modeMavlink = true;

	static bool baseSavedFlag;

	static String getHeading(){

		double val = eLeReS.KURS;
		//if(the_aircraft.)
		//if(modeMavlink)
			//val = the_aircraft.heading;
		return doubleToString(val, 3, 1);


	}

	static String getUTX(){

		//if(the_aircraft.)
		return doubleToString(eLeReS.uTX, 3, 1);


	}

	static bool baseSaved(){
		return baseSavedFlag;
		//return false;
	}

	static String getTTX(){

		//if(the_aircraft.)
		return doubleToString(eLeReS.tTX, 3, 1);


	}


	static String getSTX(){

		//if(the_aircraft.)
		return doubleToString(eLeReS.STX, 3, 1);


	}


	static String getFlightMode(){

		char text[10];
		//sprintf(text, ":%h ", eLeReS.TRYB);


		//if(the_aircraft.)
		//return String(text);
		//return doubleToString(eLeReS.TRYB, 3, 1);
		return eLeReS.TRYBString;

	}



	static String getPressure(){

		//if(the_aircraft.)
		return doubleToString(eLeReS.KURS, 3, 1);


	}



	static String getRoll(){
	//	String text;
		//char * text;
		//sprintf(text, "roll:%f ", the_aircraft.attitude.roll);
		//return "3";// + the_aircraft.attitude.roll;
		return doubleToString(the_aircraft.attitude.roll, 4, 1);
	}

	static void tick(){

		if(the_aircraft.gps.num_sats > 3)
					baseSavedFlag = true;
	}

	static String getLatitude(){

		return doubleToString(the_aircraft.location.gps_lat/10000000.0, 4, 6);
		//return doubleToString(the_aircraft.gps.gps_t);

	}

	static int fixType(){
		return the_aircraft.gps.fix_type;
	}

	static int numOfSats(){

		return the_aircraft.gps.num_sats;
	}

	static String getLongitude(){


			return doubleToString(the_aircraft.location.gps_lon/10000000.0, 4, 6);
			//return doubleToString(the_aircraft.gps.gps_t);

	}


};




#endif /* INTERFORMATINTERFACE_HPP_ */
