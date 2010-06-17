// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InventoryModule_InventoryModuleApi_h
#define incl_InventoryModule_InventoryModuleApi_h

#if defined (_WINDOWS)
#if defined(INVENTORY_MODULE_EXPORTS) 
#define INVENTORY_MODULE_API __declspec(dllexport)
#else
#define INVENTORY_MODULE_API __declspec(dllimport) 
#endif
#else
#define INVENTORY_MODULE_API
#endif

#endif
