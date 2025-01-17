//<LICENCE>

#ifndef MATLABIO_H_
#define MATLABIO_H_

#include "../../kipl_global.h"

#include <cstdlib>
#include <iostream>

namespace kipl { namespace io { namespace core { 
/// Enum used to select data type to save in mat file

typedef enum {
	mxUNKNOWN_CLASS = 0,
	mxCELL_CLASS,
	mxSTRUCT_CLASS,
	mxLOGICAL_CLASS,
	mxCHAR_CLASS,
	mxSPARSE_CLASS,		// OBSOLETE! DO NOT USE 
	mxDOUBLE_CLASS,
	mxSINGLE_CLASS,
	mxINT8_CLASS,
	mxUINT8_CLASS,
	mxINT16_CLASS,
	mxUINT16_CLASS,
	mxINT32_CLASS,
	mxUINT32_CLASS,
	mxINT64_CLASS,		// place holder - future enhancements 
	mxUINT64_CLASS,		// place holder - future enhancements 
	mxFUNCTION_CLASS,
    mxOPAQUE_CLASS,
	mxOBJECT_CLASS
} mxClassID;

std::ostream KIPLSHARED_EXPORT  & operator<<(std::ostream & s, mxClassID id);

size_t KIPLSHARED_EXPORT sizeofMAT(mxClassID id);

/// \brief Reads the first matrix of a mat file (raw read)
///	Both compressed and raw data can be read
///	\param data pointer to the data array
///	\param dims Array containing the lengts of the supported dimensions
///	\param NDim Number of dimensions
///	\param type Data type of the matrix
///	\param name The Matlab name of the variable
///	\param fname Filename of the file to be read
///
///  \note The function allocates memory for both data and name. This memory has to be deallocated
///  manually by delete [].
int KIPLSHARED_EXPORT ReadMATmatrix(char **data, size_t *dims, int &NDim, mxClassID  & type, char  **varname, const char *fname);


/// \brief Writes mat file with the data (raw version)
///	\param data pointer to the data array
///	\param dims Array containing the lengts of the supported dimensions
///	\param NDim Number of dimensions
///	\param type Data type of the matrix
///	\param varname The Matlab name of the variable
///	\param fname Filename of the file to be written
int KIPLSHARED_EXPORT WriteMATmatrix(void *data,const size_t * dims, size_t NDim, mxClassID type, const char *varname, const char *fname);

/// Reads the file head of a mat file
int  KIPLSHARED_EXPORT ReadMAThead(FILE **inf, size_t & rows, size_t & cols, mxClassID  & type, char  **varname, const char *fname);

/// Writes the file head of a mat file
int KIPLSHARED_EXPORT WriteMAThead(FILE **of,
				 const size_t *dims, 
				 size_t NDim, 
				 mxClassID type, 
				 const char *varname, 
				 const char *fname);

/// Finishes of the writing of a mat file
int KIPLSHARED_EXPORT FinishWriteMAT(FILE **of, int headsize=0x88);

/// \brief Get the text name of of MAT class id
///	\param id Class ID
///	\param str String containing the class description
std::string KIPLSHARED_EXPORT ClassIDstr(mxClassID id);

mxClassID KIPLSHARED_EXPORT GetMatlabClassID( float a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( double a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( char a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( unsigned char a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( short a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( unsigned short a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( int a );
mxClassID KIPLSHARED_EXPORT GetMatlabClassID( unsigned int a );
//mxClassID GetMatlabClassID( long long a );
//mxClassID GetMatlabClassID( unsigned long long a );

}}} // end namespace fileio
#endif // MATLABIO_H_
