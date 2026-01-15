# Module name configuration
MODULE = axi_test
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
verilate:
	@echo "### VERILATING ###"
	$(VERILATOR) -cc --exe -j 4 \
		--top-module $(MODULE) \
		$(SVLOG_FILES) $(TEST_CPP) \
		-CFLAGS "$(CPP_INC_FLAGS)" \
		--trace

# Generate Verilator project files and build simulation executable
.PHONY: build
build: ./obj_dir/V$(MODULE)

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

# Build executable
./obj_dir/V$(MODULE): $(SVLOG_FILES) $(TEST_CPP) $(CPP_FILES)
	@echo
	@echo "### VERILATING ###"
	$(VERILATOR) -cc --exe --build -j 4 \
		--top-module $(MODULE) \
		$(SVLOG_FILES) $(TEST_CPP) \
		-CFLAGS "$(CPP_INC_FLAGS)" \
		--trace

# Clean up generated files
.PHONY: clean
clean:
	rm -rf obj_dir waveform.vcd .stamp.verilate
