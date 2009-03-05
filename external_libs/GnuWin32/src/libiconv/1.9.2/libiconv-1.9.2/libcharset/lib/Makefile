# Makefile for libcharset/lib

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = ../../../libiconv-1.9.2-src/libcharset/lib
resdir = ../../res
vpath %.rc ../../res

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
INCLUDES = -I. -I$(srcdir) -I.. -I$(srcdir)/.. -I../include
DEFS = -DLIBDIR=\"$(libdir)\" \
-DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DNO_XMALLOC \
-Dset_relocation_prefix=libcharset_set_relocation_prefix \
-Drelocate=libcharset_relocate -DHAVE_CONFIG_H -DINSTALLDIR=\"$(prefix)/bin\" 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --mode=link
LIBTOOL_INSTALL = $(LIBTOOL) --mode=install
LIBTOOL_UNINSTALL = $(LIBTOOL) --mode=uninstall
RCTOOL_COMPILE = RCTOOL
RM = rm -f

# Programs used by "make install":
INSTALL = /bin/install.exe -cpD
INSTALL_DATA = /bin/install.exe -cp
mkinstalldirs = $(SHELL) ../../../libiconv-1.9.2-src/libcharset/autoconf/mkinstalldirs

# Rule to make compiled resource (Windows)
%-rc.o: %.rc
	windres --include-dir $(resdir) -i $< -o $@

#### End of system configuration section. ####

PACKAGE = 
VERSION = 1.9
MAJOR=$(shell echo $(VERSION) | sed -e "s/\..*$$//")
MINOR=$(shell echo $(VERSION) | sed -e "s/^[^\.]*\.0*\([^.]*\).*$$/\1/")

SHELL = /bin/sh

# Before making a release, change this according to the libtool documentation,
# section "Library interface versions".
LIBCHARSET_VERSION_INFO = 1:0:0

# Needed by $(LIBTOOL).
top_builddir = ..

SOURCES = $(srcdir)/localcharset.c $(srcdir)/relocatable.c

OBJECTS = localcharset.lo relocatable.lo 
libcharset_dll_OBJECTS = $(OBJECTS:.lo=.o) libcharset-dllversion.o libcharset-dll-rc.o

all : libcharset.la libcharset.dll charset.alias ref-add.sed ref-del.sed

libcharset.la : $(OBJECTS)
	$(LIBTOOL_LINK) $(CC) $(LDFLAGS) -o libcharset.la -rpath $(libdir) -version-info $(LIBCHARSET_VERSION_INFO) -no-undefined $(OBJECTS)

libcharset.dll: libcharset.la libcharset-dllversion.lo libcharset-dll-rc.lo
	cd .libs && o2dll -o libcharset.dll -l libcharset $(libcharset_dll_OBJECTS) $(LIBS)
	cp -fp libcharset.la libcharset.la.orig
	sed -e "s/libcharset.a/libcharset.dll.a/" libcharset.la.orig > libcharset.la
	cp -fp .libs/libcharset*.dll $@

localcharset.lo : $(srcdir)/localcharset.c
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -c $(srcdir)/localcharset.c

relocatable.lo : $(srcdir)/relocatable.c $(srcdir)/relocatable.h
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CFLAGS) $(CPPFLAGS) $(DEFS) -c $(srcdir)/relocatable.c

libcharset-dllversion.lo: $(resdir)/libcharset-dllversion.c
	$(LIBTOOL_COMPILE) $(CC) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $(DEFS) -c $(resdir)/libcharset-dllversion.c

libcharset-dll-rc.lo: $(resdir)/libcharset-dll.rc
	$(RCTOOL_COMPILE) $(resdir)/libcharset-dll.rc

charset.alias: $(srcdir)/config.charset
	$(SHELL) $(srcdir)/config.charset 'i386-pc-mingw32' > t-$@
	mv t-$@ $@

ref-add.sed : $(srcdir)/ref-add.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@//g' $(srcdir)/ref-add.sin > t-$@
	mv t-$@ $@

ref-del.sed : $(srcdir)/ref-del.sin
	sed -e '/^#/d' -e 's/@''PACKAGE''@//g' $(srcdir)/ref-del.sin > t-$@
	mv t-$@ $@

# Installs the library and include files only. Typically called with only
# $(libdir) and $(includedir) - don't use $(prefix) and $(exec_prefix) here.
install-lib : all force
	$(mkinstalldirs) $(libdir)
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libcharset.la $(libdir)/libcharset.la
	$(INSTALL_DATA) .libs/libcharset.lib $(DESTDIR)$(libdir)/libcharset.lib
	$(INSTALL_DATA) .libs/libcharset-bcc.lib $(DESTDIR)$(libdir)/libcharset-bcc.lib
	$(INSTALL_DATA) .libs/libcharset1.def $(DESTDIR)$(libdir)/libcharset1.def
	test -f $(libdir)/charset.alias && orig=$(libdir)/charset.alias \
	                                || orig=charset.alias; \
	sed -f ref-add.sed $$orig > $(libdir)/t-charset.alias; \
	$(INSTALL_DATA) $(libdir)/t-charset.alias $(libdir)/charset.alias; \
	rm -f $(libdir)/t-charset.alias

# The following is needed in order to install a simple file in $(libdir)
# which is shared with other installed packages. We use a list of referencing
# packages so that "make uninstall" will remove the file if and only if it
# is not used by another installed package.
# On systems with glibc-2.1 or newer, the file is redundant, therefore we
# avoid installing it.

install : all force
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	$(LIBTOOL_INSTALL) $(INSTALL_DATA) libcharset.la $(DESTDIR)$(libdir)/libcharset.la
	$(INSTALL_DATA) .libs/libcharset.lib $(DESTDIR)$(libdir)/libcharset.lib
	$(INSTALL_DATA) .libs/libcharset-bcc.lib $(DESTDIR)$(libdir)/libcharset-bcc.lib
	$(INSTALL_DATA) .libs/libcharset1.def $(DESTDIR)$(libdir)/libcharset1.def
	if test -f $(DESTDIR)$(libdir)/charset.alias; then \
	  sed -f ref-add.sed $(DESTDIR)$(libdir)/charset.alias > $(DESTDIR)$(libdir)/t-charset.alias; \
	  $(INSTALL_DATA) $(DESTDIR)$(libdir)/t-charset.alias $(DESTDIR)$(libdir)/charset.alias; \
	  rm -f $(DESTDIR)$(libdir)/t-charset.alias; \
	else \
	  if test no = no; then \
	    sed -f ref-add.sed charset.alias > $(DESTDIR)$(libdir)/t-charset.alias; \
	    $(INSTALL_DATA) $(DESTDIR)$(libdir)/t-charset.alias $(DESTDIR)$(libdir)/charset.alias; \
	    rm -f $(DESTDIR)$(libdir)/t-charset.alias; \
	  fi ; \
	fi

installdirs : force
	$(mkinstalldirs) $(DESTDIR)$(libdir)

uninstall : force
	$(LIBTOOL_UNINSTALL) $(RM) $(DESTDIR)$(libdir)/libcharset.la
	if test -f $(DESTDIR)$(libdir)/charset.alias; then \
	  sed -f ref-del.sed $(DESTDIR)$(libdir)/charset.alias > $(DESTDIR)$(libdir)/t-charset.alias; \
	  if grep '^# Packages using this file: $$' $(DESTDIR)$(libdir)/t-charset.alias > /dev/null; then \
	    rm -f $(DESTDIR)$(libdir)/charset.alias; \
	  else \
	    $(INSTALL_DATA) $(DESTDIR)$(libdir)/t-charset.alias $(DESTDIR)$(libdir)/charset.alias; \
	  fi; \
	  rm -f $(DESTDIR)$(libdir)/t-charset.alias; \
	fi

check : all

mostlyclean : clean

clean : force
	$(RM) *.o *.lo *.a *.la *.o.lock core charset.alias ref-add.sed ref-del.sed
	$(RM) -r .libs _libs

distclean : clean
	$(RM) config.status config.log config.cache Makefile libtool

maintainer-clean : distclean

force :

