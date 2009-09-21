# Define target name and output directory
init_target (RexCBLogin)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB UI_FILES *.ui)
file (GLOB RESOURCE_FILES resource/*.qrc)

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap 
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

build_library (${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
use_package (QT4)
