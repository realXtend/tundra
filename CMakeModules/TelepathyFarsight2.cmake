
macro (FIND_TELEPATHY_FARSIGHT2)
endmacro (FIND_TELEPATHY_FARSIGHT2)


macro (INCLUDE_TELEPATHY_FARSIGHT2)
	
    if (MSVC)
	  include_directories (${ENV_NAALI_DEP_PATH}/telepathy-farsight2/include)
	  link_directories (${ENV_NAALI_DEP_PATH}/telepathy-farsight2/lib)
    else(NOT MSVC AND TELEPATHY_FARSIGHT2_FOUND)
	  include_directories(${TELEPATHY_FARSIGHT2_INCLUDE_DIRS})
	  link_directories(${TELEPATHY_FARSIGHT2_LIBRARY_DIRS})
	else()
	  include_directories(${TELEPATHY_FARSIGHT2_INCLUDE_DIR})
	  link_directories(${TELEPATHY_FARSIGHT2_LIBRARY_DIR})
	endif (MSVC)
	
endmacro (INCLUDE_TELEPATHY_FARSIGHT2)

macro (LINK_TELEPATHY_FARSIGHT2)
  
  if (MSVC)
    target_link_libraries (${TARGET_NAME} gstfarsight.lib)
  endif()

endmacro (LINK_TELEPATHY_FARSIGHT2)
