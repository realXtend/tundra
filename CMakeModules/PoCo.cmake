# Find, include and link Poco
# Find is already called in the top-level CMakeLists

macro (FIND_POCO)
endmacro (FIND_POCO)

macro (INCLUDE_POCO)
	if (MSVC)
		include_directories (${PROJECT_SOURCE_DIR}/external_libs/PoCo/include)

		link_directories (${PROJECT_SOURCE_DIR}/external_libs/PoCo/lib)
	endif (MSVC)
endmacro (INCLUDE_POCO)

macro (LINK_POCO)
	if (NOT MSVC)
		target_link_libraries (${TARGET_NAME}
			optimized PocoFoundation
			debug PocoFoundationd
			optimized PocoUtil
			debug PocoUtild
			optimized PocoXML
			debug PocoXMLd
			optimized PocoNet
			debug PocoNetd
		)
	endif (NOT MSVC)
endmacro (LINK_POCO)
