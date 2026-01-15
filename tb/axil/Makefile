# Module name configuration
MODULE = axil_test
TOP = ./$(MODULE).sv
TEST_CPP = ./$(MODULE).cpp

# Verilator executable (set path here if not in system PATH)
VERILATOR = verilator

# Find all SystemVerilog/Verilog source files recursively
SVLOG_FILES = \
    $(shell find . -name "*.sv" -o -name "*.v")

# Find all C++ source files recursively
CPP_FILES = \
    $(shell find ../../src -name "*.hpp") \
    $(shell find ../../src -name "*.cpp")

# Get unique directories containing .sv files
SVLOG_DIRS = $(sort $(dir $(SVLOG_FILES)))

# Get unique directories containing .cpp/.hpp files
CPP_DIRS = $(sort $(dir $(CPP_FILES)))
CPP_INC_FLAGS = $(foreach dir, $(CPP_DIRS), -I$(abspath $(dir)))

# Run simulation (default target)
.PHONY: sim
sim: waveform.vcd

# Only generate Verilator project files (no build)
.PHONY: verilate
verilate: .stamp.verilate

# Generate Verilator project files and build simulation executable
.PHONY: build
build: .stamp.verilate ./obj_dir/V$(MODULE)

# Run simulation and open waveform in GTKWave
.PHONY: waves
waves: waveform.vcd
	@echo
	@echo "### WAVES ###"
	gtkwave waveform.vcd

# Generate VCD waveform by running simulation
waveform.vcd: ./obj_dir/V$(MODULE)
	@echo
	@echo "### SIMULATING ###"
	@./obj_dir/V$(MODULE)

# Build simulation executable using generated makefile
./obj_dir/V$(MODULE): .stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	$(MAKE) -C obj_dir -f V$(MODULE).mk V$(MODULE)

# Dummy target to force regeneration
FORCE:

# Run Verilator to generate project files
.stamp.verilate: $(TOP) $(TEST_CPP) FORCE
	@echo
	@echo "### VERILATING ###"
	$(VERILATOR) -Wall --trace --no-trace-params --x-assign unique --x-initial unique -cc $(TOP) --exe $(TEST_CPP) -CFLAGS "$(CPP_INC_FLAGS)" $(foreach dir, $(SVLOG_DIRS), -y $(dir)) -public  -Wno-PINCONNECTEMPTY -Wno-NULLPORT -Wno-PROCASSINIT -Wno-GENUNNAMED -Wno-IMPLICIT -Wno-PINNOTFOUND -Wno-PINMISSING -Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-UNUSEDPARAM -Wno-SYNCASYNCNET
	@touch .stamp.verilate

# Lint SystemVerilog code using Verilator
.PHONY: lint
lint: $(TOP)
	$(VERILATOR) --lint-only $(TOP) $(foreach dir, $(SVLOG_DIRS), -y $(dir))

# Clean all generated files and directories
.PHONY: clean
clean:
	rm -rf .stamp.verilate ./obj_dir waveform.vcd
