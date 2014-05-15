//
// This file is part of the ModuleConfig library by Anders Kaestner
// (c) 2010 Anders Kaestner
// Distribution is only allowed with the permission of the author.
//
// Revision information
// $Author$
// $Date$
// $Rev$
// $Id$
//
#include "stdafx.h"
#include "../include/ModuleException.h"
#include <string>
#include <sstream>

ModuleException::ModuleException(void)
{
}

ModuleException::~ModuleException(void)
{
}

ModuleException::ModuleException(std::string msg) : kipl::base::KiplException(msg)
{}

ModuleException::ModuleException(std::string msg, std::string filename, size_t line) :
		kipl::base::KiplException(msg,filename,line)
{}

std::string ModuleException::what()
{
	if (sFileName.empty())
		return sMessage;
	else {
		std::stringstream str;
		str<<"Module exception in "<<sFileName<<" on line "<<nLineNumber<<": \n"<<sMessage;
		return str.str();
	}
}
