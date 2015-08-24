/*
 * ImagingException.cpp
 *
 *  Created on: Jan 25, 2013
 *      Author: kaestner
 */
//#include "stdafx.h"

#include <sstream>
#include "../include/ImagingException.h"

ImagingException::~ImagingException()
{
}

ImagingException::ImagingException(std::string msg) :
        kipl::base::KiplException(msg,"ImagingException")
{
}

ImagingException::ImagingException(std::string msg, std::string filename, size_t line) :
        kipl::base::KiplException(msg, filename, line,"ImagingException")
{
}

std::string ImagingException::what()
{
	if (sFileName.empty())
		return sMessage;
	else {
		std::stringstream str;
		str<<"Imaging exception in "<<sFileName<<" on line "<<nLineNumber<<": \n"<<sMessage;
		return str.str();
	}
}
