# Find, include and link Ogre Find is already called in the top-level 
# CMakeLists

macro (FIND_OGRE)
	if (NOT MSVC)
		find_package (PkgConfig)
		pkg_search_module (OGRE OGRE)
		if (NOT OGRE_INCLUDE_DIRS AND NOT OGRE_LIBRARIES)
			message(FATAL_ERROR "Ogre not found by pkg-config")
		endif (NOT OGRE_INCLUDE_DIRS AND NOT OGRE_LIBRARIES)
		separate_arguments (OGRE_INCLUDE_DIRS)
		separate_arguments (OGRE_LIBRARIES)
	endif (NOT MSVC)
endmacro (FIND_OGRE)

macro (INCLUDE_OGRE)
	if (MSVC)
		if (DEFINED ENV{OGRE_HOME})
			include_directories ($ENV{OGRE_HOME}/include)
			link_directories ($ENV{OGRE_HOME}/lib)
		else ()
			include_directories (${PROJECT_SOURCE_DIR}/external_libs/Ogre/include)
			link_directories (${PROJECT_SOURCE_DIR}/external_libs/Ogre/lib)
		endif ()
		
	else (MSVC)
		include_directories (${OGRE_INCLUDE_DIRS})
		link_directories (${OGRE_LIBDIR})
	endif (MSVC)
endmacro (INCLUDE_OGRE)

macro (LINK_OGRE)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug OgreMain_d
			optimized OgreMain
		)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} ${OGRE_LIBRARIES})
	endif (MSVC)
endmacro (LINK_OGRE)
