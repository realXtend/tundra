# Define target name and output directory
init_target (AvatarModule OUTPUT plugins)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB UI_FILES ui/*.ui)
file (GLOB MOC_FILES AvatarDescAsset.h AvatarEditor.h AvatarModule.h EC_Avatar.h)

# Qt4 Moc files to "CMake Moc" subgroup
# and ui_*.h generated .h files to "Generated UI" subgroup
MocFolder ()
UiFolder ()

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} ${UI_FILES} PARENT_SCOPE)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP (MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI (UI_SRCS ${UI_FILES})

add_definitions (-DAVATAR_MODULE_EXPORTS)

# Includes
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule JavascriptModule)
if (NOT ANDROID)
    use_app_modules(ECEditorModule)
endif ()

build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})

# Linking
link_ogre ()
link_modules(TundraCore Math OgreRenderingModule JavascriptModule)
if (NOT ANDROID)
    link_modules(ECEditorModule)
endif ()

SetupCompileFlagsWithPCH()

final_target()
