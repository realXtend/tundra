# Define target name and output directory
init_target (viewer OUTPUT ./)

# Define source files
file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Preprocessor defines on MSVC
if (MSVC)
   if (WINDOWS_APP)
       add_definitions (-DWINDOWS_APP)
   endif (WINDOWS_APP)

   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELEASE _DMEMDUMP)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_RELWITHDEBINFO _DMEMDUMP)
   set_property (DIRECTORY PROPERTY COMPILE_DEFINITIONS_DEBUG _DMEMORYLEAKS)

    # Add the DEBUG_CPP_NAME to all other .cpp files but StableHeaders.cpp (would conflict with the #defines in PCH)
    # This define is used to remember the name of the compilation unit for memory leak checking.
    # Also label all files to use a PCH.
    foreach(src_file ${CPP_FILES})
            if (${src_file} MATCHES "StableHeaders.cpp$")
            else()
                get_filename_component(basename ${src_file} NAME)
                set_source_files_properties(${src_file}
                    PROPERTIES COMPILE_FLAGS "-DDEBUG_CPP_NAME=\"\\\"${basename}\"\\\"")
            endif()
    endforeach(src_file ${CPP_FILES})
endif (MSVC)

use_package (BOOST)
use_package (POCO)
use_package (CURL)
use_modules (Core Foundation Interfaces HttpUtilities SupportModules TestModule SceneManager)

build_executable (${TARGET_NAME} ${SOURCE_FILES})

link_package (BOOST)
link_package (POCO)
link_package (CURL)
link_modules (Core Foundation Interfaces HttpUtilities SceneManager)

final_target ()

