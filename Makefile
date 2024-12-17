# Compiler
CXX = g++
# Compiler flags
CXXFLAGS := -g $(shell root-config --cflags)
CXXLIBS := $(shell root-config --libs --glibs)
# Suppress warnings
# CXXFLAGS += -w

WCSIM_INSTALL_DIR = $(wcsim_install_directory)

EXT_INC = $(WCSIM_INSTALL_DIR)/include/WCSim
EXT_LIB = -L$(WCSIM_INSTALL_DIR)/lib -lWCSimRoot

# Directories
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
BIN_DIR = bin

# Source files
#  Gets all .cpp files from the utilities directory
SRCS := $(wildcard $(SRC_DIR)/utilities/*.cpp)
#  and all of its subdirectories
SRCS += $(wildcard $(SRC_DIR)/utilities/**/*.cpp)

# Object files
#  Generates .o object file names from the list of files in SRCS
OBJS = $(patsubst $(SRC_DIR)/utilities/%.cpp,$(BUILD_DIR)/utilities/%.o,$(SRCS))

# Program names
#  Collects all .cpp files in the programs directory
#  basename removes the .cpp from the filename
#  notdir removes the directory path from each filename
PROGRAMS = $(notdir $(basename $(wildcard $(SRC_DIR)/programs/*.cpp)))

# Executables
#  Adds the directory path defined in BIN_DIR to each program name to
#  give the final binary path for each program
EXES = $(addprefix $(BIN_DIR)/,$(PROGRAMS))

# Build rule for each program
#  Builds the final executable by linking the program object files to the
#  utility object files
#  If the binary directory, BIN_DIR, does not exist then it gets made here
#  $$^ represents the above prerequisites, the program object files and the
#  utility object files
#  $$@ represents the target, the final executable
define build_program
$(BIN_DIR)/$(1): $(BUILD_DIR)/programs/$(1).o $(OBJS)
	@echo "\033[1;33m==========Building $(1)==========\033[0m"
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I$(EXT_INC) $(CXXLIBS) $(EXT_LIB) $$^ -o $$@
	@echo "\033[0;32m==========Done==========\033[0m\n"
endef

# Loop over each program name in the PROGRAMS list and evaluates build_program
# for each item in the list
$(foreach prog,$(PROGRAMS),$(eval $(call build_program,$(prog))))

# Build rule for utility object files
#  Creates the build directory for utilities if it does not exist
#  Compiles the utility source files into object files
#  -MD generates dependency files for each object file so that make can detect
#    changes in header files
#  $< represents the first prerequisite, which is the source file to be compiled
#  $@ represents the target, the object file to be created
$(BUILD_DIR)/utilities/%.o: $(SRC_DIR)/utilities/%.cpp
	@echo "\033[1;33m==========Building $@==========\033[0m"
	@mkdir -p $(BUILD_DIR)/utilities
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(EXT_INC) -MD -c $< -o $@
	@echo "\033[0;32m==========Done==========\033[0m\n"

# Build rule for program object files
#  Creates the build directory for programs if it does not exist
#  Compiles the program source files into object files
${BUILD_DIR}/programs/%.o: ${SRC_DIR}/programs/%.cpp
	@echo "\033[1;33m==========Building $@==========\033[0m"
	@mkdir -p ${BUILD_DIR}/programs
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -I$(EXT_INC) -c $< -o $@
	@echo "\033[0;32m==========Done==========\033[0m\n"

# Include dependency files
-include $(OBJS:.o=.d)

# Clean rule
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Phony target to avoid conflicts with file names
.PHONY: all clean

# Default target
all: $(EXES)

# Default target when running `make` without arguments
.DEFAULT_GOAL := all
