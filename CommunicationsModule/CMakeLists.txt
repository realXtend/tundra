# Define target name and output directory
init_target (CommunicationsModule OUTPUT modules/core)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h )
file (GLOB XML_FILES *.xml)
file (GLOB RESOURCE_FILES resource/*.qrc)

# Qt4 Moc files to subgroup "CMake Moc"
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (SOURCE_FILES ${SOURCE_FILES} ${CPP_FILES} ${H_FILES})

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

use_package (BOOST)
use_package (POCO)
use_package (QT4)

use_modules ( Core Foundation Interfaces RexCommon )

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

link_package (BOOST)
link_package (POCO)
link_package (QT4)
link_modules (Core Foundation Interfaces RexCommon )

# Copy XML files from source directory to target directory
# TODO: Move this code to a 'copy_files_to_target(files)' macro in CMakeModules
foreach (XML_FILE ${XML_FILES})
	add_custom_command (TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${XML_FILE} ${TARGET_DIR})
endforeach (XML_FILE)

SetupCompileFlagsWithPCH()

final_target ()
