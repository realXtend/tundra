# Define target name and output directory
init_target(Console)

# Use this if building Console as a SHARED lib.
#init_target(Console OUTPUT ./)

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB MOC_FILES ConsoleAPI.h UiConsoleManager.h)
set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_modules(Framework Ui Input)

build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

#add_definitions(-DCONSOLE_API_EXPORTS)

link_package(BOOST)
link_package(QT4)

link_modules(Framework Ui Input)

SetupCompileFlags()

final_target()
