# NOTE: Only MSVC environment setup done

include (glib)

macro (FIND_GSTREAMER)
  FIND_GLIB()
endmacro(FIND_GSTREAMER)


macro (INCLUDE_GSTREAMER)
	INCLUDE_GLIB()
    if (MSVC)
	  include_directories (${NAALI_DEP_PATH}/gstreamer/include)
	  link_directories (${NAALI_DEP_PATH}/gstreamer/lib)
    else(NOT MSVC AND POCO_FOUND)
	  include_directories(${GSTREAMER_INCLUDE_DIRS})
	  link_directories(${GSTREAMER_LIBRARY_DIRS})
	else()
	  include_directories(${GSTREAMER_INCLUDE_DIR})
	  link_directories(${GSTREAMER_LIBRARY_DIR})
	endif (MSVC)
	
endmacro (INCLUDE_GSTREAMER)

macro (LINK_GSTREAMER)
  LINK_GLIB()
  target_link_libraries (${TARGET_NAME} libavcodec.lib
        libavformat.lib
        libavutil.lib
        libgstapp-0.10.lib
        libgstaudio-0.10.lib
        libgstbase-0.10.lib
        libgstcontroller-0.10.lib
        libgstdataprotocol-0.10.lib
        libgstdshow-0.10.lib
        libgstinterfaces-0.10.lib
        libgstnet-0.10.lib
        libgstnetbuffer-0.10.lib
        libgstpbutils-0.10.lib
        libgstreamer-0.10.lib
        libgstriff-0.10.lib
        libgstrtp-0.10.lib
        libgstrtsp-0.10.lib
        libgstsdp-0.10.lib
        libgsttag-0.10.lib
        libgstvideo-0.10.lib
        libjpeg.lib
        libjpeg-static.lib)

endmacro (LINK_GSTREAMER)
