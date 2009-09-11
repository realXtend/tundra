# Find, include and link PythonQt - http://pythonqt.sourceforge.net/
# Find is already called in the top-level CMakeLists

macro (FIND_PYTHONQT)
 
#  if (UNIX)
#    find_package(PythonLibs)
#    if (NOT PYTHON_LIBRARIES OR NOT_PYTHON_INCLUDE_PATH)
#      message (FATAL_ERROR "Python not found by pkg-config")
#    endif()
#  endif()

endmacro (FIND_PYTHONQT)

macro (INCLUDE_PYTHONQT)
	
  if (MSVC)
    include_directories (${NAALI_DEP_PATH}/PythonQt/include)
    link_directories (${NAALI_DEP_PATH}/PythonQt/lib)
  else()
    include_directories(${PYTHONQT_INCLUDE_PATH})
    link_directories(${PYTHONQT_LIBRARY_DIR})
  endif (MSVC)
  
endmacro (INCLUDE_PYTHONQT)

macro (LINK_PYTHONQT)
  if (MSVC)
    target_link_libraries (${TARGET_NAME}
            debug PythonQt_d
	    debug PythonQt_QtAll_d
            optimized PythonQt
	    optimized PythonQt_QtAll)
  else()
    target_link_libraries (${TARGET_NAME} ${PYTHONQT_LIBRARIES})
  endif (MSVC)
  

endmacro (LINK_PYTHONQT)
