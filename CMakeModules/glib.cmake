# NOTE: Only MSVC environment setup done

macro (FIND_GLIB)
endmacro (FIND_GLIB)

macro (INCLUDE_GLIB)
	
    if (MSVC)
	  include_directories (${ENV_NAALI_DEP_PATH}/glib/include)
	  link_directories (${ENV_NAALI_DEP_PATH}/glib/lib)
    else(NOT MSVC AND GLIB_FOUND)
	  include_directories(${GLIB_INCLUDE_DIRS})
	  link_directories(${GLIB_LIBRARY_DIRS})
	else()
	  include_directories(${GLIB_INCLUDE_DIR})
	  link_directories(${GLIB_LIBRARY_DIR})
	endif (MSVC)
	
endmacro (INCLUDE_GLIB)

macro (LINK_GLIB)
  
  if (MSVC)
    target_link_libraries (${TARGET_NAME} glib-2.0.lib gobject-2.0.lib)
  endif()

endmacro (LINK_GLIB)
