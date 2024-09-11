EXEC := raw-sockets

CPP := g++
CPPFLAGS := -O2 -Wall -Wextra -Werror -pedantic -std=c++23 -flto

LD := g++
LDFLAGS :=

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach dir,$(wildcard $1*),$(call rwildcard,$(dir)/,$2))

SRC := $(call rwildcard,src/,*.cpp)
OBJ := $(addprefix build/,$(SRC:.cpp=.o))

.PHONY: all
all: build/$(EXEC)

-include $(OBJ:.o=.d)

build/$(EXEC): $(OBJ)
	@mkdir -p $(@D)
	$(LD) $+ $(LDFLAGS) -o $@

build/%.o: %.cpp
	@mkdir -p $(@D)
	$(CPP) $(CPPFLAGS) -MMD -c -o $@ $<

.PHONY: clean
clean:
	rm -rf build
