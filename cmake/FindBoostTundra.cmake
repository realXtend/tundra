
macro(configure_boost)
    if (NOT TUNDRA_NO_BOOST OR TUNDRA_BOOST_REGEX OR TUNDRA_BOOST_SYSTEM)
        if (NOT Boost_FOUND)
            if (MSVC)
                set(Boost_USE_MULTITHREADED TRUE)
                set(Boost_USE_STATIC_LIBS TRUE)
            else ()
                set(Boost_USE_MULTITHREADED FALSE)
                set(Boost_USE_STATIC_LIBS FALSE)
            endif ()

            # Boost lookup rules:
            # 1. If a CMake variable BOOST_ROOT was set before calling configure_boost(), that directory is used.
            # 2. Otherwise, if an environment variable BOOST_ROOT was set, use that.
            # 3. Otherwise, use Boost from the Tundra deps directory.

            if ("${BOOST_ROOT}" STREQUAL "")
                file (TO_CMAKE_PATH "$ENV{BOOST_ROOT}" BOOST_ROOT)
            endif()

            if ("${BOOST_ROOT}" STREQUAL "")
                SET(BOOST_ROOT ${ENV_TUNDRA_DEP_PATH}/boost)
            endif()

            message("** Configuring Boost")
            message(STATUS "Using BOOST_ROOT = " ${BOOST_ROOT})
            message(STATUS "-- Configuring as")
            message(STATUS "       TUNDRA_NO_BOOST      = ${TUNDRA_NO_BOOST}")
            message(STATUS "       TUNDRA_BOOST_REGEX   = ${TUNDRA_BOOST_REGEX}")
            message(STATUS "       TUNDRA_BOOST_SYSTEM  = ${TUNDRA_BOOST_SYSTEM}")

            # We build boost from custom deps directory, so don't look up boost from system.
            if (ANDROID)
                set(Boost_NO_SYSTEM_PATHS TRUE)
            endif()

            set(Boost_FIND_REQUIRED TRUE)
            set(Boost_FIND_QUIETLY TRUE)
            set(Boost_DEBUG FALSE)
            set(Boost_USE_MULTITHREADED TRUE)
            set(Boost_DETAILED_FAILURE_MSG TRUE)
            set(Boost_ADDITIONAL_VERSIONS "1.39.0" "1.40.0" "1.41.0" "1.42.0" "1.43.0" "1.44.0" "1.46.1")

            if ("${TUNDRA_BOOST_LIBRARIES}" STREQUAL "")
                # Full boost enabled
                if (NOT TUNDRA_NO_BOOST)
                    set(TUNDRA_BOOST_LIBRARIES system thread regex)
                else()
                    # Full boost disabled with regex or system from boost
                    if (TUNDRA_BOOST_SYSTEM)
                        set(TUNDRA_BOOST_LIBRARIES system)
                    endif()
                    if (TUNDRA_BOOST_REGEX)
                        set(TUNDRA_BOOST_LIBRARIES ${TUNDRA_BOOST_LIBRARIES} regex)
                    endif()
                endif()
            endif()

            find_package(Boost 1.39.0 COMPONENTS ${TUNDRA_BOOST_LIBRARIES})

            if (Boost_FOUND)
                include_directories(${Boost_INCLUDE_DIRS})

                # We are trying to move to absolute lib path linking.
                # This enables us to use Boost for certain functionality
                # without linking Boost to all built libraries and executables.
                # This works cleanly even if TUNDRA_NO_BOOST is defined.
                # Windows uses auto-linking to library names so it will need this dir.
                if (MSVC)
                    link_directories(${Boost_LIBRARY_DIRS})
                endif()

                message(STATUS "-- Include Directories")
                foreach(include_dir ${Boost_INCLUDE_DIRS})
                    message (STATUS "       " ${include_dir})
                endforeach()
                message(STATUS "-- Library Directories")
                foreach(library_dir ${Boost_LIBRARY_DIRS})
                    message (STATUS "       " ${library_dir})
                endforeach()
                message(STATUS "-- Libraries")
                foreach(lib ${Boost_LIBRARIES})
                    message (STATUS "       " ${lib})
                endforeach()
                message("")
            else()
                message(FATAL_ERROR "Boost not found!")
            endif()

            # On Android, pthread library does not exist. Remove it if mistakenly added to boost libraries
            if (ANDROID)
                list(REMOVE_ITEM Boost_LIBRARIES "pthread")
            endif()
        endif()
    endif()
endmacro (configure_boost)

macro(link_boost)
    # Visual Studio uses library auto-linking via include pragmas.
    if (UNIX OR APPLE)
        if (NOT TUNDRA_NO_BOOST OR TUNDRA_BOOST_REGEX)
            if (NOT Boost_FOUND)
                message(FATAL_ERROR "Boost has not been found with configure_boost!")
            endif()
            if (NOT TUNDRA_NO_BOOST)
                # All found boost libraries
                set(TUNDRA_BOOST_LIBRARIES_TO_LINK ${Boost_LIBRARIES})
            elseif (TUNDRA_BOOST_REGEX)
                # Only regexp
                # note: TUNDRA_BOOST_SYSTEM is linked separately by modules that need it.
                set(TUNDRA_BOOST_LIBRARIES_TO_LINK optimized ${Boost_REGEX_LIBRARY_RELEASE} debug ${Boost_REGEX_LIBRARY_DEBUG})
            endif()
            target_link_libraries(${TARGET_NAME} ${TUNDRA_BOOST_LIBRARIES_TO_LINK})
        endif()
    endif()
endmacro (link_boost)
