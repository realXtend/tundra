# NOTE: Only MSVC environment setup done

macro (FIND_TELEPATHY_QT4)
endmacro (FIND_TELEPATHY_QT4)


macro (INCLUDE_TELEPATHY_QT4)
	
    if (MSVC)
	  include_directories (${NAALI_DEP_PATH}/telepathy-qt4/include)
	  link_directories (${NAALI_DEP_PATH}/telepathy-qt4/lib)
    else(NOT MSVC AND TELEPATHY_QT4_FOUND)
	  include_directories(${TELEPATHY_QT4_INCLUDE_DIRS})
	  link_directories(${TELEPATHY_QT4_LIBRARY_DIRS})
	else()
	  include_directories(${TELEPATHY_QT4_INCLUDE_DIR})
	  link_directories(${TELEPATHY_QT4_LIBRARY_DIR})
	endif (MSVC)
	
endmacro (INCLUDE_TELEPATHY_QT4)

macro (LINK_TELEPATHY_QT4)
  
  if (MSVC)
    target_link_libraries (${TARGET_NAME} telepathy-qt4.lib)
  endif()

endmacro (LINK_TELEPATHY_QT4)
