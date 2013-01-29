# Define target name and output directory
init_target (PythonScriptModule OUTPUT plugins)

# Get EC defines that are added to this build.
GetEntityComponents()

# Define source files
# todo: check the moc list and in general what can be removed.
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
file (GLOB XML_FILES *.xml)
file (GLOB MOC_FILES PythonScriptModule.h PythonQtScriptingConsole.h TundraWrapper.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Qt4 Wrap
QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

set (FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${H_FILES} ${CPP_FILES} PARENT_SCOPE)

add_definitions (-DPYMODULE_MODULE_EXPORTS)

# Use python and python qt. 
# Boost/Ogre includes are automatic.
use_package (PYTHON)
use_package (PYTHON_QT)

# Use core and app modules we need. Notice separate macros due to source locations separation.
UseTundraCore()
use_core_modules(TundraCore Math OgreRenderingModule TundraProtocolModule)

# Build PythonScriptModule
build_library (${TARGET_NAME} SHARED ${SOURCE_FILES} ${MOC_SRCS})

# Link to all: this include core and app (as the linking is just done by name not absolute path)
link_modules(TundraCore OgreRenderingModule TundraProtocolModule)

# Link to ECs. This macro will smartly opt out ECs that are not included in the build, info for this provided by GetEntityComponents() on top.
link_entity_components (EC_Script EC_Highlight EC_HoveringText EC_Sound EC_TransformGizmo EC_PlanarMirror EC_ProximityTrigger)

# Use python, pythonqt and ogre
link_package (PYTHON)
link_package (PYTHON_QT)
link_ogre()

# Use PCH with StableHeaders.h
SetupCompileFlagsWithPCH()

# Finalize our target, this will do some nice automation for us.
final_target ()

# make install: Install files
setup_install_directory (${TUNDRA_BIN}/pyplugins)

# make install: Install windows deps
# todo: resurect the dep install steps when the windows deps are fixed. The macro signature has changed, look at how other working code is using it when porting!
#setup_install_windows_deps ("python26.dll;PythonQt.dll;PythonQt_QtAll.dll" Release|RelWithDebInfo "python26_d.dll;PythonQt_d.dll;PythonQt_QtAll_d.dll" Debug)
#setup_install_windows_deps_regexp ("[/]pyplugins[/]python[/]DLLs[/].*[^d][.]pyd$" Release|RelWithDebInfo "[/]pyplugins[/]python[/]DLLs[/].*[d][.]pyd$" Debug)
#setup_install_windows_deps_regexp ("[/]pyplugins[/]python[/].*[.]zip$" Release|RelWithDebInfo|Debug)
