# Makefile to build class 'helloworld' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# how do I add a "shared" folder to put all the .h files in?

# library name
lib.name = visca

# input source file (class name == source file basename)
class.sources = visca.c

# build for 64 bit only
# arch := x86_64
define forDarwin
  cflags = -mmacosx-version-min=10.14 -arch x86_64
endef

# Add a ../include folder for organizing header files
# CPPFLAGS += -Iinclude

# link to dynlibs
ldlibs = -lvisca -lserialport

# all extra files to be included in binary distribution of the library
# datafiles = mp3cast~-help.pd README.txt LICENSE.txt
# datafiles = 

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
# include ‎⁨~/Documents/Pd/externals/Gem/include/Gem/Base/

# must do (why this doesn”t work with stock pd and pd-lib-builder?)
# make PDINCLUDEDIR=Applications/Pd.app/Contents/Resources/include/