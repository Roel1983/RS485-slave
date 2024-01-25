all:

# Public settings
PROJECT_NAME              ?= RGBW-bar
UPLOAD_PORT               ?= /dev/ttyUSB0

# Directories
SOURCE_DIR                := src
BUILD_DIR                 := build
BIN_DIR                   := bin
CODE_COVERAGE_DIR         := codecoverage

# File name patterns (operation system dependent)
OBJECT_FILE_PATTERN       := %.o
EXECUTABLE_FILE_PATTERN   := %

FileName                   = $(foreach file,$1,$(dir $(file))$(patsubst %,$2,$(notdir $(basename $(file)))))
ObjectFileName             = $(call FileName,$1,$(OBJECT_FILE_PATTERN))
ExecutableFileName         = $(call FileName,$1,$(EXECUTABLE_FILE_PATTERN))

# Utility functions
wildcard_recursive_1       = $(wildcard $1$2) $(foreach dir,$(wildcard $1*/),$(call wildcard_recursive_1,$(dir),$2))
wildcard_recursive         = $(foreach base_dir,$1,$(call wildcard_recursive_1,$(base_dir),$2))

# Determine Firmware files
FIRMWARE_SOURCE_FILES_CPP := $(wildcard src/*.cpp) # Make recursive
FIRMWARE_SOURCE_FILES_C   := $(wildcard src/*.c)   # Make recursive
FIRMWARE_SOURCE_FILES     := $(FIRMWARE_SOURCE_FILES_CPP) $(FIRMWARE_SOURCE_FILES_C)

FIRMWARE_BUILD_DIR         = $(BUILD_DIR)/firmware
FIRMWARE_OBJECT_FILES      = $(call ObjectFileName, $(addprefix $(FIRMWARE_BUILD_DIR)/, $(FIRMWARE_SOURCE_FILES)))

FIRMWARE_ELF_FILE          = $(BUILD_DIR)/$(PROJECT_NAME).elf
FIRMWARE_HEX_FILE          = $(BIN_DIR)/$(PROJECT_NAME).hex

# Determine Unittest files
UNITTEST_SOURCE_DIRS       = $(call wildcard_recursive,$(SOURCE_DIR)/,unittest/)

UNITTEST_SOURCE_FILES_CPP  = $(FIRMWARE_SOURCE_FILES_CPP)
UNITTEST_SOURCE_FILES_CPP += $(call wildcard_recursive,$(UNITTEST_SOURCE_DIRS),*.cpp)
UNITTEST_SOURCE_FILES_C   += $(call wildcard_recursive,$(UNITTEST_SOURCE_DIRS),*.c)
UNITTEST_SOURCE_FILES      = $(UNITTEST_SOURCE_FILES_CPP) $(UNITTEST_SOURCE_FILES_C)

UNITTEST_BUILD_DIR         = $(BUILD_DIR)/unittest
UNITTEST_OBJECT_FILES      = $(call ObjectFileName, $(addprefix $(UNITTEST_BUILD_DIR)/, $(UNITTEST_SOURCE_FILES)))

UNITTEST_EXECUTABLE_FILE   = $(call ExecutableFileName, $(BIN_DIR)/$(PROJECT_NAME)_unittest)
bla:
	echo $(UNITTEST_SOURCE_DIRS)
# Dependency generation
DEPFLAGS = -MT $@ -MMD -MP -MF $(basename $@).d

# Determine avrdude args
AVRDUDE_ARGS               = -carduino -P$(UPLOAD_PORT) -b57600

# Utility functions
echo_execute = (echo "$1"; $1)
mkdir        = test -d "$1" && exit 0 || mkdir -p $1

# Check needed programs
is_installed     = $(if $(shell which $1),yes,no)

AVR_GCC_INSTALLED := $(call is_installed, avr-gcc)
.PHONY: avr-gcc
avr-gcc:
ifneq ($(AVR_GCC_INSTALLED), yes)
	@echo "Error: 'avr-gcc' is not installed"
	@exit 1
endif

AVRDUDE_INSTALLED := $(call is_installed, avrdude)
.PHONY: avrdude
avrdude:
ifneq ($(AVRDUDE_INSTALLED), yes)
	@echo "Error: 'avrdude' is not installed"
	@exit 1
endif

GCOVR_INSTALLED := $(call is_installed, gcovr)
.PHONY: gcovr
gcovr:
ifneq ($(GCOVR_INSTALLED), yes)
	@echo "Error: 'gcovr' is not installed"
	@exit 1
endif

# Code coverage
ifeq ($(GCOVR_INSTALLED), yes)
CODE_COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage
endif
UNITTEST_GCDA_FILES = $(UNITTEST_OBJECT_FILES:%.o=%.gcda)
UNITTEST_GCNO_FILES = $(UNITTEST_OBJECT_FILES:%.o=%.gcno)

exclude_from_code_coverage = $(filter %_test,$1)$(filter src/unittest/fakeavr/%,$1)

# Public targets
.PHONY: all
.PHONY: firmware
.PHONY: unittest
.PHONY: test
.PHONY: codecoverage
.PHONY: clean
.PHONY: cleanall
.PHONY: upload

all: firmware
ifeq ($(GCOVR_INSTALLED), yes)
all: codecoverage
else
all: test
endif

firmware: $(FIRMWARE_HEX_FILE)

unittest: $(UNITTEST_EXECUTABLE_FILE)

test: unittest
	@rm -f $(UNITTEST_GCDA_FILES)
	./$(UNITTEST_EXECUTABLE_FILE) --gtest_brief=0

codecoverage: $(CODE_COVERAGE_DIR)/codecoverage.html

$(CODE_COVERAGE_DIR)/codecoverage.html: $(UNITTEST_EXECUTABLE_FILE)
	@rm -f $(UNITTEST_GCDA_FILES)
	./$(UNITTEST_EXECUTABLE_FILE) --gtest_brief=0
	
	@$(if $(wildcard %.gcov),-rm *.gcov)
	@$(call mkdir, $(CODE_COVERAGE_DIR))
	$(foreach dir, $(filter-out build/unittest/src/unittest/fakeavr%,$(sort $(dir $(UNITTEST_OBJECT_FILES)))),\
		gcov \
		$(if $(filter-out %/unittest/,$(dir)),-b) \
		-l -p -o $(dir) $(dir)*.gcno > /dev/null;)
	
	@gcovr --use-gcov-files --json > $(CODE_COVERAGE_DIR)/codecoverage.json
	@gcovr --add-tracefile $(CODE_COVERAGE_DIR)/codecoverage.json --html-nested $(CODE_COVERAGE_DIR)/codecoverage.html
	@gcovr --add-tracefile $(CODE_COVERAGE_DIR)/codecoverage.json --print-summary

clean:
	@test -d "$(BUILD_DIR)" && rm -r $(BUILD_DIR) || :
	@test -d "$(CODE_COVERAGE_DIR)" && rm -r $(CODE_COVERAGE_DIR) || :

cleanall: clean
	@test -d "$(BUILD_DIR)" && rm -r $(BUILD_DIR) || :
	@test -d "$(BIN_DIR)" && rm -r $(BIN_DIR) || :

upload: $(FIRMWARE_HEX_FILE) avrdude
	avrdude -p m328p $(AVRDUDE_ARGS) -D -U flash:w:$(FIRMWARE_HEX_FILE)

# Firmware 
$(FIRMWARE_HEX_FILE): $(FIRMWARE_ELF_FILE) | avr-gcc
	@$(call mkdir, $(dir $@))
	avr-objcopy -O ihex -j .text -j .data $< $@

$(FIRMWARE_ELF_FILE): $(FIRMWARE_OBJECT_FILES) | avr-gcc
	@$(call mkdir, $(dir $@))
	avr-gcc -mmcu=atmega328 $(FIRMWARE_OBJECT_FILES) -o $@

$(FIRMWARE_BUILD_DIR)/%.o: %.cpp
$(FIRMWARE_BUILD_DIR)/%.o: %.cpp $(FIRMWARE_BUILD_DIR)/%.d | avr-gcc Makefile
	@$(call mkdir, $(dir $@))
	avr-gcc -c -Wall -Os $(DEPFLAGS) -mmcu=atmega328 $< -o $@

# Unittest
$(UNITTEST_EXECUTABLE_FILE): $(UNITTEST_OBJECT_FILES)
	@$(call mkdir, $(dir $@))
	g++ $(CODE_COVERAGE_FLAGS) -o $@ $^ /usr/src/gtest/lib/libgtest.a

$(UNITTEST_BUILD_DIR)/%.o: %.cpp
$(UNITTEST_BUILD_DIR)/%.o: %.cpp $(UNITTEST_BUILD_DIR)/%.d Makefile
	@$(call mkdir, $(dir $@))
	g++ -g3 -c $(DEPFLAGS) $(CODE_COVERAGE_FLAGS) -DUNITTEST -Isrc/unittest/fakeavr -o $@ $< 

#dependency generation


DEPFILES := $(UNITTEST_OBJECT_FILES:%.o=%.d)
DEPFILES += $(FIRMWARE_OBJECT_FILES:%.o=%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))
