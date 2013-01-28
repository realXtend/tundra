# Define target name and output directory
init_target(Math)

MocFolder()

# Define source files
file(GLOB CPP_FILES *.cpp)
file(GLOB H_FILES *.h)
# file(GLOB MOC_FILES )

AddSourceFolder(Algorithm/Random)
AddSourceFolder(Geometry)
AddSourceFolder(Math)
AddSourceFolder(Time)

#AddSourceFolder(MathGeoLib/src/Algorithm/Random)
#AddSourceFolder(MathGeoLib/src/Geometry)
#AddSourceFolder(MathGeoLib/src/Math)
#AddSourceFolder(MathGeoLib/src/Time)

set(SOURCE_FILES ${CPP_FILES} ${H_FILES})

#QT4_WRAP_CPP(MOC_SRCS ${MOC_FILES})

UseTundraCore() ## Needed only for CoreTypes.h

build_library(${TARGET_NAME} STATIC ${SOURCE_FILES} ${MOC_SRCS})

if (NOT TUNDRA_NO_BOOST)
    # Link Boost to Framework. All plugins link to framework, so Boost dependency will automatically propagate from here to each library.
    if (UNIX OR APPLE) # Visual Studio uses library auto-linking, so this is only necessary for the other platforms.
       target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES})
    endif()
endif()

link_package(QT4)

SetupCompileFlags()

final_target()
