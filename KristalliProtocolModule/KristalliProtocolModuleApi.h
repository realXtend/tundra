/** @file
	@author LudoCraft Oy

	Copyright 2009 LudoCraft Oy.
	All rights reserved.

	@brief
*/
#ifndef incl_KristalliProtocolModuleApi_h
#define incl_KristalliProtocolModuleApi_h

#if defined (_WINDOWS)
#if defined(KRISTALLIPROTOCOL_MODULE_EXPORTS) 
#define KRISTALLIPROTOCOL_MODULE_API __declspec(dllexport)
#else
#define KRISTALLIPROTOCOL_MODULE_API __declspec(dllimport) 
#endif
#else
#define KRISTALLIPROTOCOL
#endif

#endif
