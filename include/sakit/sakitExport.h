/// @file
/// @version 1.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines macros for DLL exports/imports.

#ifndef SAKIT_EXPORT_H
#define SAKIT_EXPORT_H

	/// @def sakitExport
	/// @brief Macro for DLL exports/imports.
	/// @def sakitFnExport
	/// @brief Macro for function DLL exports/imports.
	#ifdef _LIB
		#define sakitExport
		#define sakitFnExport
	#else
		#ifdef _WIN32
			#ifdef SAKIT_EXPORTS
				#define sakitExport __declspec(dllexport)
				#define sakitFnExport __declspec(dllexport)
			#else
				#define sakitExport __declspec(dllimport)
				#define sakitFnExport __declspec(dllimport)
			#endif
		#else
			#define sakitExport __attribute__ ((visibility("default")))
			#define sakitFnExport __attribute__ ((visibility("default")))
		#endif
	#endif
	#ifndef DEPRECATED_ATTRIBUTE
		#ifdef _MSC_VER
			#define DEPRECATED_ATTRIBUTE __declspec(deprecated("function is deprecated"))
		#else
			#define DEPRECATED_ATTRIBUTE __attribute__((deprecated))
		#endif
	#endif

#endif

