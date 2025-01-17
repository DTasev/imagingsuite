//<LICENCE>

#ifndef ANALYZEFILEEXT_H
#define ANALYZEFILEEXT_H

#include "../kipl_global.h"

#include <iostream>
#include <string>

namespace kipl {
/// The io name space contains functions and classes that support file io using different image file formats.
namespace io {

/// \brief enum to describe supported file types
enum eExtensionTypes {
    ExtensionTXT,   ///< The image is provided as lines of space separated numbers
    ExtensionDMP,   ///< A raw binary image format. Usually a binary dump file from a camera
    ExtensionDAT,   ///< A raw binary image format.
    ExtensionXML,   ///< Image data stored in xml format (rather unusual)
    ExtensionRAW,   ///< A raw binary image format
    ExtensionFITS,  ///< Images are stored in the fits format
    ExtensionTIFF,  ///< Images are stored in the tiff format
    ExtensionPNG,   ///< Images are stored in the png format
    ExtensionJPG,   ///< Images are stored in the jpg format
    ExtensionMAT,   ///< Images are stored in the matlab binary format (deprecated since a several years)
    ExtensionHDF,    ///< Images are stored using a HDF format
    ExtensionHDF4,    ///< Images are stored using a HDF4 format
    ExtensionHDF5,    ///< Images are stored using a HDF5 format
    ExtensionSEQ    ///< Images are stored in the Varian ViVa SEQ format
};

eExtensionTypes KIPLSHARED_EXPORT GetFileExtensionType(std::string fname);

}}

/// \brief Sends the name of the enum value to a stream
/// \param s The target stream
/// \param eType The enum value
/// \returns The target stream
std::ostream KIPLSHARED_EXPORT & operator<<(std::ostream &s, kipl::io::eExtensionTypes etype);

/// \brief Translates a string to an extension enum
/// \param ext a string containing a file extension
/// \param et target enum variable
void KIPLSHARED_EXPORT string2enum(std::string ext, kipl::io::eExtensionTypes &et);

/// \brief Translates the value of an extension enum to a string
/// \param et The enum value to translate
/// \returns A string with the value
std::string KIPLSHARED_EXPORT enum2string(kipl::io::eExtensionTypes et);
#endif // ANALYZEFILEEXT_H

