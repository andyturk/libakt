# override root locations in local_vars.mk
-include local_vars.mk

# The following variables ending in _ROOT are directories where external
# things are installed. The location of these may change depending on
# how the host machine is configured. Override the definitions here in
# the optional local_vars.mk
GTEST_ROOT              ?= ../../gtest-1.7.0
LIBAKT_ROOT             ?= ..

# Where build products go
BUILD                   := build
OBJ                      = $(BUILD)/obj

# Source files
C_SRC                   += 
CXX_SRC                 += src/gtest-all.cc src/gtest_main.cc
CXX_SRC                 += $(LIBAKT_ROOT)/akt/json/reader.cc $(LIBAKT_ROOT)/akt/json/writer.cc
CXX_SRC                 += $(shell find . -type f -name '*test.cc')

# Object files
OBJECTS                  = $(addprefix $(OBJ)/, $(C_SRC:.c=.o) $(CXX_SRC:.cc=.o))

CFLAGS                  += -I$(GTEST_ROOT)/include
CFLAGS                  += -I$(GTEST_ROOT)
CFLAGS                  += -I$(LIBAKT_ROOT)
CFLAGS                  += -g3
CFLAGS                  += -Wall

CXXFLAGS                += -std=c++11
CXXFLAGS                += $(CFLAGS)

DIRS                    += $(BUILD) $(BUILD)/deps
DIRS                    += $(sort $(dir $(OBJECTS)))

VPATH                   = $(GTEST_ROOT)

help :
	@echo "The following targets are available:"
	@echo "  make run               -- compile and run tests"
	@echo "  make a.out             -- compile and link executable"
	@echo "  make clean             -- nukes build products"
	@echo "  make info              -- stuff for debugging the Makefile"

clean :
	@rm -rf $(BUILD)

info :
	@echo USER: $(USER)
	@echo OBJECTS: $(OBJECTS)
	@echo C_SRC: $(C_SRC)
	@echo CXX_SRC: $(CXX_SRC)

a.out : $(BUILD)/a.out

run : $(BUILD)/a.out
	@$(BUILD)/a.out

$(DIRS) :
	@echo Creating $(@)
	@mkdir -p $(@)

$(BUILD)/a.out : $(OBJECTS) $(MAKEFILE_LIST) | $(BUILD)
	@echo Linking $(@)
	@$(CXX) \
		$(LDFLAGS) \
		-o $(@) $(OBJECTS)

$(OBJECTS) : | $(DIRS)

$(OBJ)/%.o : %.c
	@echo Compiling $(<F)
	@$(CC) $(CFLAGS) -c $< -o $(@) -MD -MF $(BUILD)/deps/$(notdir $*.d)

$(OBJ)/%.o : %.cc
	@echo Compiling $(<F)
	@$(CXX) $(CXXFLAGS) -c $< -o $(@) -MD -MF $(BUILD)/deps/$(notdir $*.d)

$(OBJ)/%.E : %.c
	@$(CC) $(CFLAGS) -E -c $< -o $(@)

$(OBJ)/%.E : %.cc
	@$(CXX) $(CXXFLAGS) -E -c $< -o $(@)

$(OBJ)/%.o : %.s
	@echo Assembling $(<F)
	@$(AS) $(ASFLAGS) $< -o $(@)

.PHONY : clean info default run
