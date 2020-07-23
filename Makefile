# Copyright © 2014-2018 Tyler Veness
#
# This makefile recursively finds, compiles, and links .c and .cpp files.
#
# The following targets are supported by this makefile. Indentations represent
# dependencies.
#
# all
# clean
#
# Defining VERBOSE before running make (i.e., VERBOSE=1 make) will print all
# commands ran.

EXEC := raw-sockets

CC := gcc
CFLAGS := -O3 -Wall -s -std=c11 -flto

CXX := g++
CXXFLAGS := -O3 -Wall -s -std=c++17 -flto

# Specify defines with -D directives here
DEFINES :=

LD := g++

# Specify include paths with -I directives here
IFLAGS :=

# Specify libs with -l directives here
LDFLAGS := -lpthread

SRCDIR := src
OBJDIR := build

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach dir,$(wildcard $1*),$(call rwildcard,$(dir)/,$2))

# Recursively find all C source files
SRC_C := $(call rwildcard,$(SRCDIR)/,*.c)

# Recursively find all C++ source files
SRC_CXX := $(call rwildcard,$(SRCDIR)/,*.cpp)

# Create raw list of object files
C_OBJ := $(SRC_C:.c=.o)
CXX_OBJ := $(SRC_CXX:.cpp=.o)

# Create list of object files for build
C_OBJ := $(addprefix $(OBJDIR)/,$(C_OBJ))
CXX_OBJ := $(addprefix $(OBJDIR)/,$(CXX_OBJ))

.PHONY: all
all: $(OBJDIR)/$(EXEC)

-include $(C_OBJ:.o=.d) $(CXX_OBJ:.o=.d)

$(OBJDIR)/$(EXEC): $(C_OBJ) $(CXX_OBJ)
	@mkdir -p $(@D)
	@echo Linking $@
ifdef VERBOSE
	$(LD) -o $@ $(C_OBJ) $(CXX_OBJ) $(LDFLAGS)
else
	@$(LD) -o $@ $(C_OBJ) $(CXX_OBJ) $(LDFLAGS)
endif

# Pattern rule for building object file from C source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(C_OBJ): $(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo Building C object $@
ifdef VERBOSE
	$(CC) $(CFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CC) $(CFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
endif

# Pattern rule for building object file from C++ source
# The -MMD flag generates .d files to track changes in header files included in
# the source.
$(CXX_OBJ): $(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@echo Building CXX object $@
ifdef VERBOSE
	$(CXX) $(CXXFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
else
	@$(CXX) $(CXXFLAGS) $(DEFINES) $(IFLAGS) -MMD -c -o $@ $<
endif

# Cleans the build directory
.PHONY: clean
clean:
	@echo Removing object files
ifdef VERBOSE
	-$(RM) -r $(OBJDIR)/$(SRCDIR)
	-$(RM) $(OBJDIR)/$(EXEC)
else
	-@$(RM) -r $(OBJDIR)/$(SRCDIR)
	-@$(RM) $(OBJDIR)/$(EXEC)
endif
