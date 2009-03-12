# Find, include and link Gtkmm.
# Find is already called in the top-level CMakeLists

macro (FIND_GTKMM)
	if (NOT DEFINED ENV{GTKMM_HOME})
		message (FATAL_ERROR "Environment variable GTKMM_HOME not set! Please set it to point to where you installed gtkmm, http://www.gtkmm.org/download.shtml e.g. \"C:\\Program Files\\gtkmm\"")
	endif ()
endmacro ()

macro (INCLUDE_GTKMM)
	include_directories (
		$ENV{GTKMM_HOME}/include/gtk-2.0/gdk
		$ENV{GTKMM_HOME}/include/atk-1.0
		$ENV{GTKMM_HOME}/lib/gtk-2.0/include
		$ENV{GTKMM_HOME}/include/gdkmm-2.4
		$ENV{GTKMM_HOME}/include/atkmm-1.6
		$ENV{GTKMM_HOME}/lib/gdkmm-2.4/include
		$ENV{GTKMM_HOME}/lib/gtkmm-2.4/include
		$ENV{GTKMM_HOME}/include/cairomm-1.0
		$ENV{GTKMM_HOME}/include/cairo
		$ENV{GTKMM_HOME}/include/pango-1.0
		$ENV{GTKMM_HOME}/include/gtk-2.0
		$ENV{GTKMM_HOME}/include/pangomm-1.4
		$ENV{GTKMM_HOME}/include/sigc++-2.0
		$ENV{GTKMM_HOME}/lib/glib-2.0/include
		$ENV{GTKMM_HOME}/include/glib-2.0
		$ENV{GTKMM_HOME}/lib/glibmm-2.4/include
		$ENV{GTKMM_HOME}/include/glibmm-2.4
		$ENV{GTKMM_HOME}/include/gtkmm-2.4
		$ENV{GTKMM_HOME}/lib/sigc++-2.0/include
		$ENV{GTKMM_HOME}/include/libglade-2.0
		$ENV{GTKMM_HOME}/include/libglademm-2.4
	)
	
	link_directories (
		$ENV{GTKMM_HOME}/lib
	)
	
endmacro ()

macro (LINK_GTKMM)
	if (MSVC)
		target_link_libraries (${TARGET_NAME}

			glib-2.0.lib
			gtk-win32-2.0.lib

			debug atkmm-vc90-d-1_6.lib
			debug cairomm-vc90-d-1_0.lib
			debug gdkmm-vc90-d-2_4.lib
			debug glibmm-vc90-d-2_4.lib
			debug gtkmm-vc90-d-2_4.lib
			debug pangomm-vc90-d-1_4.lib
			debug sigc-vc90-d-2_0.lib
			debug glademm-vc90-d-2_4.lib

			optimized atkmm-vc90-1_6.lib
			optimized cairomm-vc90-1_0.lib
			optimized gdkmm-vc90-2_4.lib
			optimized glibmm-vc90-2_4.lib
			optimized gtkmm-vc90-2_4.lib
			optimized pangomm-vc90-1_4.lib
			optimized sigc-vc90-2_0.lib
			optimized glademm-vc90-2_4.lib
			)
	endif ()
endmacro ()
