# Find, include and link xmlrpc-epi
# Find is already called in the top-level CMakeLists

macro (FIND_XMLRPCEPI)
endmacro (FIND_XMLRPCEPI)

macro (INCLUDE_XMLRPCEPI)
	if (MSVC)
		include_directories (${PROJECT_SOURCE_DIR}/external_libs/xmlrpc-epi/src)

		link_directories (
			${PROJECT_SOURCE_DIR}/external_libs/xmlrpc-epi/
		)
	else (MSVC)
		include_directories (/usr/include/xmlrpc-epi)
	endif (MSVC)
endmacro (INCLUDE_XMLRPCEPI)

macro (LINK_XMLRPCEPI)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug xmlrpcepid
			optimized xmlrpcepi
		)
	else (MSVC)
		target_link_libraries (${TARGET_NAME}
			xmlrpc-epi
		)
	endif (MSVC)
endmacro (LINK_XMLRPCEPI)
