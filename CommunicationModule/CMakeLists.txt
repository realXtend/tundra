# Add header and source files from given subfolder 
# to H_FILES and CPP_FILES variables
#
# Alose create source groups (filters in Visual Studio)
# under "Header Files" and "Source Files" 
#
# NOTE: This macro must be called after defination
#       of source files of projecr root folder
#       and before setting SOURCE_FILES variable
# TODO: Add support for more deep folder structures. Current
#       implementation only support folders directly under project folder
macro (AddSourceFolder folder_name)
    file (GLOB H_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.h)
    file (GLOB CPP_FILES_IN_FOLDER_${folder_name} ${folder_name}/*.cpp)
    source_group ("Header Files\\${folder_name}" FILES ${H_FILES_IN_FOLDER_${folder_name}})
    source_group ("Source Files\\${folder_name}" FILES ${CPP_FILES_IN_FOLDER_${folder_name}})
    set (H_FILES ${H_FILES} ${H_FILES_IN_FOLDER_${folder_name}})
    set (CPP_FILES ${CPP_FILES} ${CPP_FILES_IN_FOLDER_${folder_name}})
endmacro (AddSourceFolder)

configure_telepathy_qt4 ()
configure_gstreamer ()
configure_dbus ()
configure_glib ()
configure_telepathy_glib ()
configure_telepathy_farsight ()

# Define target name and output directory
init_target (CommunicationModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h )
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES *.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

AddSourceFolder(OpensimIM)
AddSourceFolder(TelepathyIM)
AddSourceFolder(CommunicationUI)

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

add_definitions (-DCOMMS_MODULE_EXPORTS)

use_package (BOOST)
use_package (POCO)
use_package (QT4)
use_package (GLIB)
use_package (DBUS)
#use_package (DBUS_GLIB)
use_package (TELEPATHY_QT4)
use_package (TELEPATHY_GLIB)
use_package (TELEPATHY_FARSIGHT)
use_package (GSTREAMER) 
#use_package (TELEPATHY_FARSIGHT2) # Not implemented yet

use_modules (QtModule RexLogic OpenSimProtocol)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
use_framework_modules (Core Foundation Interfaces RexCommon) 

link_modules (QtModule RexLogicModule OpenSimProtocolModule)

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_package (TELEPATHY_FARSIGHT)
link_package (TELEPATHY_GLIB)
link_package (TELEPATHY_QT4)
#link_package (DBUS_GLIB)
link_package (DBUS)
link_package (GLIB)
link_package (GSTREAMER)
#link_package (TELEPATHY_FARSIGHT2) # Not implemented yet

# Copy XML files from source directory to target directory
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

final_target ()
