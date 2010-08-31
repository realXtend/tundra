/** @file Settings.h
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief Specifies global build settings and configurations.
*/
#ifndef clbLib_Settings_h
#define clbLib_Settings_h

namespace clb
{
	enum MatrixPackingOrder
	{
		PackRowMajor,
		PackColMajor
	};

	/// The default matrix packing order used by the clbLib system.
	const MatrixPackingOrder cDefaultPackingOrder = PackColMajor;

	/// The default floating point precision to use in 3D calculations.
	typedef float real;

	/// The type to use for representing angles.
	typedef float angle_t;

//#define CLBLIBVECTORALIGN __declspec(align(16))
#define CLBLIBVECTORALIGN /* no restriction */
//#define CLBLIBVECISSSEALIGNED
}

#endif
