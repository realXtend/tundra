/^top_srcdir = /a\
resdir = @top_builddir@res\
vpath %.rc @top_builddir@res

/^\.cc\?\.o\:/i\
# Rule to make compiled resource (Windows)\
%-rc.o: %.rc\
	windres --include-dir $(resdir) -i $< -o $@\

s/^DEFS =/& -DINSTALLDIR=\\"$(prefix)\\" /

s/^\([^A-Z_]*\)_OBJECTS = /& \1-rc.o /
s/^\([^A-Z_]*\)_SOURCES = /& \1.rc /

/^VERSION =/a\
MAJOR=$(shell echo $(VERSION) | sed -e "s/\\..*$$//")\
MINOR=$(shell echo $(VERSION) | sed -e "s/^[^\\.]*\\.0*\\([^.]*\\).*$$/\\1/")
#s/^\([^_]*\)_*LDFLAGS = /& -Wl,--major-image-version=$(MAJOR) -Wl,--minor-image-version=$(MINOR) /
#s/^\([^_]*\)_*LDADD = /& -Wl,--major-image-version=$(MAJOR) -Wl,--minor-image-version=$(MINOR) /
s/^LDADD = /& -Wl,--major-image-version=$(MAJOR) -Wl,--minor-image-version=$(MINOR) /

s/@LN_S@/cp -fp/g
s/@LN@/cp -fp/g
