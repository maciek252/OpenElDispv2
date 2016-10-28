/*
 * Utils.hpp
 *
 *  Created on: Jul 19, 2016
 *      Author: maciek
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

String doubleToString(double d, int len, int prec){
	char buffer[20];
	String tem = dtostrf(d, len, prec, buffer);
	String temper = (tem);
	return temper;
}



#endif /* UTILS_HPP_ */
