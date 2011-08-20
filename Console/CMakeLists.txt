# Define target name and output directory
init_target(Console)

# Use this if building Asset as a SHARED lib.
#init_target(Console OUTPUT ./)

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
file(GLOB UI_FILES ui/*.ui)
file(GLOB MOC_FILES ConsoleAPI.h ConsoleProxyWidget.h UiConsoleManager.h CommandManager.h)
set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)

QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})

use_modules(Core Foundation Ui Input Interfaces)

build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS})

#add_definitions(-DCONSOLE_API_EXPORTS)

link_package(BOOST)
link_package(POCO)
link_package(QT4)

link_modules(Core Foundation Ui Input Interfaces)

SetupCompileFlags()

final_target()
