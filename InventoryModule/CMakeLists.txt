# Define target name and output directory
init_target (InventoryModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES AbstractInventoryDataModel.h AbstractInventoryItem.h InventoryAsset.h InventoryFolder.h InventoryItemModel.h
    InventoryWindow.h OpenSimInventoryDataModel.h WebdavInventoryDataModel.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DUSE_QT)

use_package (BOOST)
use_package (POCO)
use_package (XMLRPC)
use_package (CURL)
use_package (QT4)
use_package (PYTHON)
use_modules (RexCommon ReXLogic OpenSimProtocol RpcUtilities HttpUtilities QtModule PythonModule)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} )
use_framework_modules (Core Foundation Interfaces RexCommon)

link_modules (RexCommon RexLogicModule OpenSimProtocolModule HttpUtilities RpcUtilities QtModule PythonScriptModule)
link_package (BOOST)
link_package (POCO)
link_package (XMLRPC)
link_package (CURL)
link_package (QT4)

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
    add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

final_target ()
