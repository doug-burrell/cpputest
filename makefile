#Set this to @ to keep the makefile quiet
SILENCE = @

#---- Outputs ----#
COMPONENT_NAME = metal2

#--- Inputs ----#
PROJECT_HOME_DIR = .
ifeq "$(CPPUTEST_HOME)" ""
    CPPUTEST_HOME =/cygdrive/c/Users/db185198/Documents/cpputest
endif

# --- SRC_FILES ---
# Use SRC_FILES to specifiy individual production
# code files.
# These files are compiled and put into the
# ProductionCode library and links with the test runner
SRC_FILES += app/metal2.c

# --- SRC_DIRS ---
# Use SRC_DIRS to specifiy production directories
# code files.
# These files are compiled and put into a the
# ProductionCode library and links with the test runner
SRC_DIRS = \

# --- TEST_SRC_FILES ---
# TEST_SRC_FILES specifies individual test files to build.  Test
# files are always included in the build and they
# pull in production code from the library
TEST_SRC_FILES = \

# --- TEST_SRC_DIRS ---
# Like TEST_SRC_FILES, but biulds everyting in the directory
TEST_SRC_DIRS = \
	tests \

# --- MOCKS_SRC_DIRS ---
# MOCKS_SRC_DIRS specifies a directories where you can put your
# mocks, stubs and fakes.  You can also just put them
# in TEST_SRC_DIRS
MOCKS_SRC_DIRS = \

# Turn on CppUMock
CPPUTEST_USE_EXTENSIONS = Y

INCLUDE_DIRS =\
  .\
  $(CPPUTEST_HOME)/include/ \
  $(CPPUTEST_HOME)/include/Platforms/Gcc \
  app \

# --- CPPUTEST_OBJS_DIR ---
# if you have to use "../" to get to your source path
# the makefile will put the .o and .d files in surprising
# places.
# To make up for each level of "../", add place holder
# sub directories in CPPUTEST_OBJS_DIR
# each "../".  It is kind of a kludge, but it causes the
# .o and .d files to be put under objs.
# e.g. if you have "../../src", set to "test-objs/1/2"
# This is set no "../" in the source path.
CPPUTEST_OBJS_DIR = tests/test-obj

CPPUTEST_LIB_DIR = tests/test-lib
CPPUTEST_WARNINGFLAGS = -Wall -Werror -Wswitch-default -Wfatal-errors -Wno-unknown-pragmas
CPPUTEST_CXXFLAGS =  -std=c++0x
CPPUTEST_CFLAGS = -std=c99
CPPUTEST_CPPFLAGS += -D__stdint_h

# Look at $(CPPUTEST_HOME)/build/MakefileWorker.mk for more controls

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
