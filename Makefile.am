SUBDIRS = . include tests examples

#License files, documentation
EXTRA_DIST = COPYING COPYING.LESSER ChangeLog doc DoxygenFooter.html

#Common flags definitions
AM_CXXFLAGS = @CXX0X_CFLAGS@ -W -Wall -Iinclude @DBUS_CFLAGS@
#Common libraries
LDADD = @DBUS_LIBS@

lib_LTLIBRARIES = libdbustl-1.la \
		  libdbustl-noex-1.la
libdbustl_1_la_SOURCES = src/ObjectProxy.cpp \
                   src/DBusObject.cpp \
                   src/Connection.cpp \
                   src/DBusException.cpp \
                   src/Message.cpp \
                   src/EventLoopIntegration.cpp 
libdbustl_noex_1_la_SOURCES = src/ObjectProxy.cpp \
                   src/DBusObject.cpp \
                   src/Connection.cpp \
                   src/DBusException.cpp \
                   src/Message.cpp \
                   src/EventLoopIntegration.cpp
libdbustl_noex_1_la_CPPFLAGS = -DDBUSTL_NO_EXCEPTIONS -fno-exceptions

#dbustl pkg-config support
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = dbustl-1.pc dbustl-noex-1.pc
DISTCLEANFILES = dbustl-1.pc dbustl-noex-1.pc
EXTRA_DIST     += dbustl-1.pc.in dbustl-noex-1.pc.in           

if HAVE_GLIB
lib_LTLIBRARIES += libdbustl-glib-1.la libdbustl-noex-glib-1.la
endif

libdbustl_glib_1_la_CPPFLAGS = @GLIB_CFLAGS@
libdbustl_glib_1_la_LIBADD = @GLIB_LIBS@
libdbustl_glib_1_la_SOURCES = src/GlibEventLoopIntegration.cpp
libdbustl_noex_glib_1_la_CPPFLAGS = @GLIB_CFLAGS@ -DDBUSTL_NO_EXCEPTIONS -fno-exceptions
libdbustl_noex_glib_1_la_LIBADD = @GLIB_LIBS@
libdbustl_noex_glib_1_la_SOURCES = src/GlibEventLoopIntegration.cpp

#dbustl-glib pkg-config support
if HAVE_GLIB
pkgconfig_DATA += dbustl-glib-1.pc dbustl-noex-glib-1.pc
endif
DISTCLEANFILES += dbustl-glib-1.pc dbustl-noex-glib-1.pc
EXTRA_DIST     += dbustl-glib-1.pc.in dbustl-noex-glib-1.pc.in

#Auto generated documentation
.PHONY: doc
doc:
	doxygen

#default to bzip2 tarball generation
dist: doc dist-bzip2
	@echo ""    
