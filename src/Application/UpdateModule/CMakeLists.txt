# Define target name and output directory
init_target (UpdateModule OUTPUT plugins)

# Define source files
if (WIN32)
    file (GLOB CPP_FILES UpdateModule.cpp)
    file (GLOB H_FILES UpdateModule.h)
    file (GLOB XML_FILES *.xml)
elseif (APPLE)
    file (GLOB CPP_FILES AutoUpdater.cpp CocoaInitializer.mm SparkleAutoUpdater.mm UpdateModuleMac.cpp)
    file (GLOB H_FILES AutoUpdater.h CocoaInitializer.h SparkleAutoUpdater.h UpdateModuleMac.h)
endif ()

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()

set (SOURCE_FILES ${CPP_FILES} ${H_FILES})
QT4_WRAP_CPP(MOC_SRCS ${H_FILES})

# Includes
if (APPLE)
    configure_sparkle()
endif ()
use_core_modules (Framework Ui)

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

# Linking
if (APPLE)
    link_package(SPARKLE)
endif ()
link_modules(Framework Ui)

SetupCompileFlags()

final_target ()
