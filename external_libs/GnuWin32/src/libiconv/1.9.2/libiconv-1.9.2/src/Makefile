# Makefile for libiconv/src

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = ../../libiconv-1.9.2-src/src
top_srcdir = ../../libiconv-1.9.2-src
resdir = ../res
vpath %.rc ../res

# Directories used by "make install":
prefix = c:/progra~1/LibIconv
local_prefix = /usr/local
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib
datadir = ${prefix}/share
localedir = $(datadir)/locale

# Programs used by "make":
CC = gcc
CFLAGS =  -Wall -O3 -fms-extensions -mms-bitfields  -fno-exceptions -fomit-frame-pointer -march=i386 -mcpu=i686  
CPPFLAGS =   -D__GW32__ -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64     -Ie:/progra~1/gnuwin32/include -idirafter h:/glibc/include   
LDFLAGS =  -Wl,-s -Wl,--force-exe-suffix -Wl,--enable-auto-import -Wl,--enable-runtime-pseudo-reloc   -lgw32c
INCLUDES = -I. -I$(srcdir) -I.. -I../include -I$(srcdir)/../include -I../srclib -I$(srcdir)/../srclib
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBTOOL_COMPILE = $(LIBTOOL) --mode=compile
LIBTOOL_LINK = $(LIBTOOL) --mode=link
LIBTOOL_INSTALL = $(LIBTOOL) --mode=install
LIBTOOL_UNINSTALL = $(LIBTOOL) --mode=uninstall
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

SHELL = /bin/sh

# Needed by $(LIBTOOL).
top_builddir = ..

# We cannot link with libintl until libiconv is installed. (When we call
# libtool with arguments "../lib/libiconv.la -lintl", libtool will call ld
# with "../lib/.libs/libiconv.so $libdir/libintl.so $libdir/libiconv.so",
# (remember that $libdir/libintl.la lists $libdir/libiconv.so as a dependency),
# and this gives a fatal linker error on Solaris because the two libiconv.so
# files are different but have the same soname.
# So we can link the iconv executable only after we have installed libiconv,
# i.e. during "make install". The intermediate 'iconv' executable is built
# without internationalization and not linked with libintl.

all : iconv_no_i18n iconv.o
	test `ls -ld . | sed -e 's/^d\(.........\).*/\1/'` = rwxrwxrwx || chmod 777 .

# This is the temporary iconv executable, without internationalization.
iconv_no_i18n : iconv_no_i18n.o ../lib/libiconv.la iconv-rc.o
	$(LIBTOOL_LINK) $(CC) $(LDFLAGS) iconv_no_i18n.o iconv-rc.o ../srclib/libicrt.a ../lib/libiconv.la -o $@

iconv_no_i18n.o : $(srcdir)/iconv_no_i18n.c $(srcdir)/iconv.c
	$(CC) -c $(INCLUDES) -I../lib $(CFLAGS) $(CPPFLAGS) -DINSTALLDIR=\"$(bindir)\" -DLOCALEDIR=\"$(localedir)\" $(srcdir)/iconv_no_i18n.c

iconv.o : $(srcdir)/iconv.c
	$(CC) -c $(INCLUDES) -I../lib $(CFLAGS) $(CPPFLAGS) -DINSTALLDIR=\"$(bindir)\" -DLOCALEDIR=\"$(localedir)\" $(srcdir)/iconv.c

# The following rule is necessary to avoid a toplevel "make -n check" failure.
../lib/libiconv.la :
	cd ../lib && $(MAKE) libiconv.la

# Support for relocatability.
RELOCATABLE_LIBRARY_PATH = $(libdir)
RELOCATABLE_SRC_DIR = $(top_srcdir)/srclib
RELOCATABLE_BUILD_DIR = ../srclib
RELOCATABLE_CONFIG_H_DIR = ..
INSTALL_PROGRAM_ENV = RELOC_LIBRARY_PATH_VAR="PATH" RELOC_LIBRARY_PATH_VALUE="$(RELOCATABLE_LIBRARY_PATH)" RELOC_PREFIX="$(prefix)" RELOC_COMPILE_COMMAND="$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS)" RELOC_SRCDIR="$(RELOCATABLE_SRC_DIR)" RELOC_BUILDDIR="$(RELOCATABLE_BUILD_DIR)" RELOC_CONFIG_H_DIR="$(RELOCATABLE_CONFIG_H_DIR)" RELOC_INSTALL_PROG="/bin/install.exe -cpD"
iconv_LDFLAGS = `if test -n '$(RELOCATABLE_LDFLAGS)'; then $(RELOCATABLE_LDFLAGS) $(bindir); fi`

# During "make install", we can build the final iconv executable.
# On HP-UX, in order to ensure that the new libiconv.sl will override the old
# one that is hardcoded in libintl.sl, we need to mention libiconv.sl before
# libintl.sl on the link command line. We have to bypass libtool in order to
# achieve this.
# On Solaris, the linker gives an error if we are using libintl.so and it
# refers to a libiconv.so in $prefix/lib since then it sees two libiconv.so's,
# one in $prefix/lib and one in ../lib/.libs. So we have to avoid using
# ../lib/libiconv.la entirely.
install : all force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(bindir) ] ; then $(mkinstalldirs) $(DESTDIR)$(bindir) ; fi
	case "mingw32" in \
	  hpux*) $(CC) $(LDFLAGS) $(iconv_LDFLAGS) iconv.o ../srclib/libicrt.a -L$(libdir) -liconv -lintl -o iconv;; \
	  *) $(LIBTOOL_LINK) $(CC) $(LDFLAGS) $(iconv_LDFLAGS) iconv.o iconv-rc.o  ../srclib/libicrt.a $(DESTDIR)$(libdir)/libiconv.la -lintl -lgw32c -o iconv;; \
	esac
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL_INSTALL) $(INSTALL_PROGRAM) iconv $(DESTDIR)$(bindir)/iconv

installdirs : force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(exec_prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(exec_prefix) ; fi
	if [ ! -d $(DESTDIR)$(bindir) ] ; then $(mkinstalldirs) $(DESTDIR)$(bindir) ; fi

uninstall : force
	$(LIBTOOL_UNINSTALL) $(RM) $(DESTDIR)$(bindir)/iconv

check : all

mostlyclean : clean

clean : force
	$(RM) *.o *.lo iconv_no_i18n iconv core
	$(RM) -r .libs _libs

distclean : clean
	$(RM) Makefile

maintainer-clean : distclean

force :

