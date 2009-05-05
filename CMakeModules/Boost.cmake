# Find, include and link boost
# Find is already called in the top-level CMakeLists

macro (FIND_BOOST)
	# Rely on find_package only on non-VS builds
	if (NOT MSVC)
		# Static linking needed for UnitTests main
		set (Boost_USE_STATIC_LIBS ON)
		find_package (Boost COMPONENTS date_time filesystem system thread program_options unit_test_framework)
		if (NOT Boost_FOUND)
			message (FATAL_ERROR "Boost not found")
		endif (NOT Boost_FOUND)
	endif (NOT MSVC)
endmacro (FIND_BOOST)

macro (INCLUDE_BOOST)
	if (MSVC)
		include_directories (${REX_DEP_PATH}/Boost/include)
		link_directories (${REX_DEP_PATH}/Boost/lib)
	else (MSVC)
		include_directories (${Boost_INCLUDE_DIR})
	endif (MSVC)
endmacro (INCLUDE_BOOST)

macro (LINK_BOOST)
	if (NOT MSVC)
		target_link_libraries (${TARGET_NAME} ${Boost_LIBRARIES})
	endif (NOT MSVC)
endmacro (LINK_BOOST)
