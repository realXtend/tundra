# Find, include and link xmlrpc-epi
# Find is already called in the top-level CMakeLists

macro (FIND_XMLRPCEPI)
endmacro (FIND_XMLRPCEPI)

macro (INCLUDE_XMLRPCEPI)
    set (XMLRPCEPI_INCLUDE_DIRS ${REX_DEP_PATH}/xmlrpc-epi/src)
    set (XMLRPCEPI_LINK_DIRS ${REX_DEP_PATH}/xmlrpc-epi)

	if (NOT MSVC)
		set (XMLRPCEPI_INCLUDE_DIRS ${XMLRPCEPI_INCLUDE_DIRS} /usr/include/xmlrpc-epi)
	endif (NOT MSVC)

    include_directories (${XMLRPCEPI_INCLUDE_DIRS})
    link_directories (${XMLRPCEPI_LINK_DIRS})
endmacro (INCLUDE_XMLRPCEPI)

macro (LINK_XMLRPCEPI)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}
			debug xmlrpcepid
			optimized xmlrpcepi)
	else (MSVC)
		target_link_libraries (${TARGET_NAME} xmlrpc-epi)
	endif (MSVC)
endmacro (LINK_XMLRPCEPI)
