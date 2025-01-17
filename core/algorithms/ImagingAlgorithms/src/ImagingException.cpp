//<LICENSE>

#include <sstream>
#include "../include/ImagingException.h"

ImagingException::~ImagingException()
{
}

ImagingException::ImagingException(std::string msg) :
    sExceptionName("ImagingException"),
    sMessage(msg),
    nLineNumber(0)
{
}

ImagingException::ImagingException(std::string msg, std::string filename, size_t line) :
    sExceptionName("ImagingException"),
    sMessage(msg),
    sFileName(filename),
    nLineNumber(line)
{
}

const char* ImagingException::what() const
{

    if (nLineNumber==0) {
        return sMessage.c_str();
    }

    if (sFileName.empty())
        return sMessage.c_str();

    std::ostringstream str;

    str<<"An "<<sExceptionName<<" was thrown in file "<<sFileName<<" on line "<<nLineNumber<<": "<<std::endl<<sMessage;

    return str.str().c_str();
}

#ifdef HAVEPYBIND11
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void bindException(py::module &m)
{
    py::register_exception<ImagingException>(m, "ImagingException");
}
#endif
