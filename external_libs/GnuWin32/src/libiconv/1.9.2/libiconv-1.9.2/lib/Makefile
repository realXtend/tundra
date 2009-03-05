# Makefile for libiconv/lib

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = ../../libiconv-1.9.2-src/lib
resdir = ../res
vpath %.rc ../res

# Directories used by "make install":
prefix = c:/progra~1/LibIconv
local_prefix = /usr/local
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib

# Programs used by "make":
CC = gcc
CFLAGS =  -Wall -O3 -fms-extensions -mms-bitfields  -fno-exceptions -fomit-frame-pointer -march=i386 -mcpu=i686  
CPPFLAGS =   -D__GW32__ -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64     -Ie:/progra~1/gnuwin32/include -idirafter h:/glibc/include   
LDFLAGS =  -Wl,-s -Wl,--force-exe-suffix -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc  	 -Wl,--major-image-version=$(MAJOR) -Wl,--minor-image-version=$(MINOR) 
INCLUDES = -I. -I$(srcdir) -I../include -I$(srcdir)/../include
DEFS =  -DINSTALLDIR=\"$(prefix)\"  -DLIBDIR=\"$(libdir)\" \
-DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DNO_XMALLOC \
-Dset_relocation_prefix=libiconv_set_relocation_prefix \
-Drelocate=libiconv_relocate -DHAVE_CONFIG_H	-DINSTALLDIR=\"$(prefix)/bin\" 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --mode=link
LIBTOOL_INSTALL = $(LIBTOOL) --mode=install
LIBTOOL_UNINSTALL = $(LIBTOOL) --mode=uninstall
RCTOOL_COMPILE = RCTOOL
CP = cp
MV = mv
LN = cp -fp
LN_S = cp -fp
RM = rm -f

# Programs used by "make install":
INSTALL = /bin/install.exe -cpD
INSTALL_PROGRAM = $(top_builddir)/../libiconv-1.9.2-src/autoconf/install-reloc
INSTALL_DATA = /bin/install.exe -cp
mkinstalldirs = $(SHELL) ../../libiconv-1.9.2-src/autoconf/mkinstalldirs

# Rule to make compiled resource (Windows)
%-rc.o: %.rc
	windres --include-dir $(resdir) -i $< -o $@

#### End of system configuration section. ####

PACKAGE = libiconv
VERSION = 1.9
MAJOR=$(shell echo $(VERSION) | sed -e "s/\..*$$//")
MINOR=$(shell echo $(VERSION) | sed -e "s/^[^\.]*\.0*\([^.]*\).*$$/\1/")

SHELL = /bin/sh

# Before making a release, change this according to the libtool documentation,
# section "Library interface versions".
LIBICONV_VERSION_INFO = 4:0:2

# Needed by $(LIBTOOL).
top_builddir = ..

SOURCES = $(srcdir)/iconv.c $(srcdir)/../libcharset/lib/localcharset.c $(srcdir)/relocatable.c

OBJECTS = iconv.lo localcharset.lo relocatable.lo 
libiconv_dll_OBJECTS = $(OBJECTS:.lo=.o) libiconv-dllversion.o libiconv-dll-rc.o

all :  libiconv.la	libiconv.dll

libiconv.la : $(OBJECTS) 
	$(LIBTOOL_LINK) $(CC) $(LDFLAGS) -o libiconv.la -rpath $(libdir) -version-info $(LIBICONV_VERSION_INFO) -no-undefined $(OBJECTS)

libiconv.dll: libiconv.la libiconv-dllversion.lo libiconv-dll-rc.lo
	cd .libs && o2dll -o libiconv.dll -l libiconv $(libiconv_dll_OBJECTS) $(LIBS)
	cp -fp libiconv.la libiconv.la.orig
	sed -e "s/libiconv.a/libiconv.dll.a/" libiconv.la.orig > libiconv.la
	cp -fp .libs/libiconv*.dll $@

libiconv_plug.so : libiconv_plug_.so
	$(RM) libiconv_plug.so
	$(LN_S) libiconv_plug_.so libiconv_plug.so

libiconv_plug_linux.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o libiconv_plug_linux.so; \
	else \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -KPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o libiconv_plug_linux.so; \
	fi

libiconv_plug_solaris.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o libiconv_plug_solaris.so; \
	else \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -KPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -G -o libiconv_plug_solaris.so; \
	fi

libiconv_plug_osf.so : $(SOURCES)
	if test -n "yes"; then \
	  $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -fPIC -DPIC -DLIBICONV_PLUG $(SOURCES) -shared -o libiconv_plug_osf.so; \
	else \
	  mkdir objects; \
	  for f in $(SOURCES); do \
	    $(CC) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -DPIC -DLIBICONV_PLUG -c $$f -o objects/`basename $$f | sed -e 's,\.c$$,.o,'`; \
	  done; \
	  /bin/ld -shared -expect_unresolved \* -o libiconv_plug_osf.so objects/*.o; \
	  rm -rf objects; \
	fi

iconv.lo : $(srcdir)/iconv.c $(srcdir)/converters.h $(srcdir)/encodings.def $(srcdir)/encodings_aix.def $(srcdir)/encodings_osf1.def $(srcdir)/encodings_dos.def $(srcdir)/encodings_local.def $(srcdir)/aliases.h $(srcdir)/aliases_aix.h $(srcdir)/aliases_osf1.h $(srcdir)/aliases_dos.h $(srcdir)/flags.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/iconv.c

localcharset.lo : $(srcdir)/../libcharset/lib/localcharset.c $(srcdir)/relocatable.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/../libcharset/lib/localcharset.c

relocatable.lo : $(srcdir)/relocatable.c $(srcdir)/relocatable.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(srcdir)/relocatable.c

libiconv-dllversion.lo: $(resdir)/libiconv-dllversion.c
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(resdir)/libiconv-dllversion.c

libiconv-dll-rc.lo: $(resdir)/libiconv-dll.rc
	$(RCTOOL_COMPILE) $(resdir)/libiconv-dll.rc

# Installs the library and include files only. Typically called with only
# $(libdir) and $(includedir) - don't use $(prefix) and $(exec_prefix) here.
install-lib : all force
	if [ ! -d $(libdir) ] ; then $(mkinstalldirs) $(libdir) ; fi
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libiconv.la $(libdir)/libiconv.la

install : all force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libiconv.la $(DESTDIR)$(libdir)/libiconv.la
	$(INSTALL_DATA) .libs/libiconv.lib $(DESTDIR)$(libdir)/libiconv.lib
	$(INSTALL_DATA) .libs/libiconv-bcc.lib $(DESTDIR)$(libdir)/libiconv-bcc.lib
	$(INSTALL_DATA) .libs/libiconv2.def $(DESTDIR)$(libdir)/libiconv2.def
	if test -n ""; then $(INSTALL_DATA)  $(DESTDIR)$(libdir)/.new && mv $(DESTDIR)$(libdir)/.new $(DESTDIR)$(libdir)/ ; fi

installdirs : force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(libdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(libdir) ; fi

uninstall : force
	$(LIBTOOL_UNINSTALL) $(RM) $(DESTDIR)$(libdir)/libiconv.la
	if test -n ""; then $(RM) $(DESTDIR)$(libdir)/ ; fi

check : all

mostlyclean : clean

clean : force
	$(RM) *.o *.lo *.a *.la libiconv_plug* core so_locations
	$(RM) -r .libs _libs

distclean : clean
	$(RM) Makefile config.h libtool

maintainer-clean : distclean

force :

