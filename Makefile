###############################################################################
#                                                                             #
#             General Makefile for C++ Projects with Unit Tests               #
#           Copyright (C) 2014 Grant Ayers <ayers@cs.stanford.edu>            #
#           Hosted at GitHub: https://github.com/grantea/makefiles            #
#                                                                             #
# This file is free software distributed under the BSD license. See LICENSE   #
# for more information.                                                       #
#                                                                             #
# This is a single-target, general-purpose Makefile for C++ projects which    #
# supports unit testing using the googletest (gtest) framework. It is         #
# for use with GNU Make and GCC, but may work with other software with little #
# or no modification.                                                         #
#                                                                             #
# Set the program name, source root, and any desired options and compiler     #
# flags. For unit testing, specify the unit test identifier which is a file   #
# name suffix that identifies unit tests. For example, if this is 'Test' then #
# source files 'car.cc' and 'animal.cc' would have unit tests called          #
# 'carTest.cc' and 'animalTest.cc', respectively. To exclude any files from   #
# unit tests, set 'TEST_EXCLUDE' (e.g. 'main' to exclude main.cc, etc.).      #
#                                                                             #
# Type 'make' to build, and 'make test' to test. All dependencies (including  #
# header file changes) will be handled automatically.                         #
#                                                                             #
###############################################################################

#---------- Basic settings  ----------#
PROGRAM_NAME := blob
SOURCE_BASE  := src
BUILD_BASE   := build
BINARY_BASE  := $(BUILD_BASE)
PROGRAM_MAIN := main
LIB_VERSION  := 1.0

#---------- Unit tests ----------#
TEST_SUFFIX  := _test
TEST_EXCLUDE := $(PROGRAM_MAIN)
GTEST_BASE   := gtest
GTEST_URL    := https://googletest.googlecode.com/files/gtest-1.7.0.zip

#---------- Compilation and linking ----------#
CXX        ?= g++
SRC_EXTS   := .cc .cpp .cxx .c++ .c
CXX_LANG   := -std=c++11 -Wl,-rpath=/usr/local/lib/gcc5 -D_GLIBCXX_USE_C99
CXX_WARN   := -pedantic -Wall -Wextra -Wcast-align -Wcast-qual \
              -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op \
              -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept \
              -Woverloaded-virtual -Wredundant-decls -Wsign-conversion \
              -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 \
              -Wswitch-default -Wshadow \
              #-Wundef -Wold-style-cast -Wctor-dtor-privacy
CXX_OPT    := -O3 -march=native -g -fPIC
CXX_COMP   := #-fdiagnostics-color=auto -pipe -Wfatal-errors
INC_DIRS   := -I$(SOURCE_BASE)
LINK_FLAGS := -lgmp -lgmpxx -lpthread


#---------- No need to modify below ----------#

OPTS     := $(CXX_LANG) $(CXX_WARN) $(CXX_OPT) $(CXX_COMP)
SRC_DIRS := $(shell find $(SOURCE_BASE) -type d -print)
BLD_DIRS := $(addprefix $(BUILD_BASE)/,$(SRC_DIRS))
ALL_SRCS := $(foreach DIR,$(SRC_DIRS),$(foreach EXT,$(SRC_EXTS),$(wildcard $(DIR)/*$(EXT))))
ALL_TSTS := $(foreach EXT,$(SRC_EXTS),$(filter %$(TEST_SUFFIX)$(EXT),$(ALL_SRCS)))
MAIN_SRC := $(foreach EXT,$(SRC_EXTS),$(filter %$(PROGRAM_MAIN)$(EXT),$(ALL_SRCS)))
MAIN_OBJ := $(addsuffix .o,$(addprefix $(BUILD_BASE)/,$(MAIN_SRC)))

# Application
APP      := $(BINARY_BASE)/$(PROGRAM_NAME)
APP_SRCS := $(filter-out $(ALL_TSTS),$(ALL_SRCS))
APP_OBJS := $(addsuffix .o,$(addprefix $(BUILD_BASE)/,$(APP_SRCS)))
APP_DEPS := $(APP_OBJS:.o=.d)

# Library (subset of Application; excludes main)
SLIB     := $(dir $(APP))lib$(notdir $(APP)).a
DLIB     := $(dir $(APP))lib$(notdir $(APP)).so
LIB_SRCS := $(filter-out $(MAIN_SRC),$(APP_SRCS))
LIB_OBJS := $(filter-out $(MAIN_OBJ),$(APP_OBJS))

# Test infrastructure
TST      := $(BINARY_BASE)/$(PROGRAM_NAME)$(TEST_SUFFIX)
TST_UOBJ := $(addsuffix .o,$(addprefix $(BUILD_BASE)/,$(ALL_TSTS)))
TST_UDEP := $(TST_UOBJ:.o=.d)
TST_AOBJ := $(filter-out $(MAIN_OBJ),$(APP_OBJS))

# Gtest framework
GTEST_PKG      := $(GTEST_BASE)/README
GTEST_INC      := $(GTEST_BASE)/include
GTEST_MAIN_OBJ := $(BUILD_BASE)/$(GTEST_BASE)/gtest_main.o
GTEST_ALL_OBJ  := $(BUILD_BASE)/$(GTEST_BASE)/gtest-all.o
GTEST_LIB      := $(BUILD_BASE)/$(GTEST_BASE)/gtest_main.a

.PHONY: all
all: $(APP) static_lib dynamic_lib $(TST)

.PHONY: clean
clean:
ifeq ($(SOURCE_BASE),$(BUILD_BASE))
	@rm -f $(APP_OBJS) $(APP_DEPS) $(APP) $(SLIB) $(DLIB) $(TST_UOBJ) $(TST_UDEP) $(TST)
else
	@rm -rf $(APP) $(SLIB) $(DLIB) $(TST) $(BUILD_BASE)
endif

# Application

$(APP): $(APP_OBJS)
	@echo [LD] $@
	@$(CXX) $(OPTS) $(APP_OBJS) $(LINK_FLAGS) -o $(APP)

$(APP_OBJS): $(BUILD_BASE)/%.o: % | $(BLD_DIRS)
	@echo [CC] $<
	@$(CXX) $(OPTS) $(INC_DIRS) -MD -MP -c -o $@ $<

$(BLD_DIRS):
	@mkdir -p $@

# Libraries

.PHONY: static_lib
static_lib: $(SLIB)

$(SLIB): $(LIB_OBJS)
	@echo [Static Library] $@
	@ar -cr -o $@ $(LIB_OBJS)
	@ranlib $@

.PHONY: dynamic_lib
dynamic_lib: $(DLIB)

$(DLIB): $(LIB_OBJS)
	@echo [Shared Library] $@
	$(CXX) -shared -Wl,-soname,lib$(PROGRAM_NAME).so.$(LIB_VERSION) $(LIB_OBJS) -o $@

# Test infrastructure

.PHONY: test
test: $(TST)
	@./$(TST)

#$(TST): $(GTEST_LIB) $(TST_UOBJ) $(TST_AOBJ) | $(BLD_DIRS)
$(TST): $(TST_UOBJ) $(TST_AOBJ) | $(BLD_DIRS)
	@echo [LD] $@
	@$(CXX) $(OPTS) -I$(GTEST_INC) $(TST_UOBJ) $(TST_AOBJ) $(GTEST_LIB) $(LINK_FLAGS) -o $(TST)


# Gtest Infrastructure

# Gtest extracted directory
$(GTEST_PKG):
	@echo '[GTest]    $(GTEST_URL)'
	@curl -O $(GTEST_URL)
	@unzip -q $(notdir $(GTEST_URL))
	@mv $(basename $(notdir $(GTEST_URL))) $(GTEST_BASE)
	@rm $(notdir $(GTEST_URL))

# Gtest primary object file (everything but main())
$(GTEST_ALL_OBJ): $(GTEST_PKG)
	@echo '[GTest]    $@'
	@mkdir -p $(dir $@)
	@$(CXX) -isystem $(GTEST_INC) -I$(GTEST_BASE) -pthread \
     -c $(GTEST_BASE)/src/gtest-all.cc -o $@

# Gtest main() function which runs all tests
$(GTEST_MAIN_OBJ): $(GTEST_PKG)
	@echo '[GTest]    $@'
	@mkdir -p $(dir $@)
	@$(CXX) -isystem $(GTEST_INC) -I$(GTEST_BASE) -pthread \
     -c $(GTEST_BASE)/src/gtest_main.cc -o $(BUILD_BASE)/$(GTEST_BASE)/gtest_main.o

# Rule for building unit test source files
$(TST_UOBJ): $(BUILD_BASE)/%.o: % $(GTEST_LIB) | $(BLD_DIRS)
	@echo [CC] $<
	@$(CXX) $(OPTS) $(INC_DIRS) -I$(GTEST_INC) -MD -MP -c -o $@ $<

.PHONY: gtest
gtest: $(GTEST_LIB)

# The gtest library which is linked with test source objects
$(GTEST_LIB): $(GTEST_ALL_OBJ) $(GTEST_MAIN_OBJ)
	@echo '[GTest]    $@'
	@ar -c -rv $@ $^


-include $(APP_DEPS) $(TST_UDEP)
