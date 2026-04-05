# AXI Stream Video VIP Test
MODULE = axis_video_test
TOP = ./$(MODULE).sv
TEST_CPP = ./$(MODULE).cpp \
    ../../src/axis_video/frame_mem.cpp

VERILATOR = verilator

SVLOG_FILES = \
    $(shell find . -name "*.sv" -o -name "*.v")

CPP_FILES = \
    $(shell find ../../src -name "*.hpp") \
    $(shell find ../../src -name "*.cpp")

SVLOG_DIRS = $(sort $(dir $(SVLOG_FILES)))
CPP_DIRS = $(sort $(dir $(CPP_FILES)))
CPP_INC_FLAGS = $(foreach dir, $(CPP_DIRS), -I$(abspath $(dir)))

CPP_SRC_FILES = $(filter %.cpp, $(CPP_FILES))

.PHONY: sim
sim: waveform.vcd

.PHONY: verilate
verilate: .stamp.verilate

.PHONY: build
build: .stamp.verilate ./obj_dir/V$(MODULE)

.PHONY: waves
waves: waveform.vcd
	@echo
	@echo "### WAVES ###"
	gtkwave waveform.vcd

waveform.vcd: ./obj_dir/V$(MODULE)
	@echo
	@echo "### SIMULATING ###"
	@./obj_dir/V$(MODULE)

./obj_dir/V$(MODULE): .stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	$(MAKE) -C obj_dir -f V$(MODULE).mk V$(MODULE)

FORCE:

.stamp.verilate: $(TOP) $(TEST_CPP) $(CPP_SRC_FILES) FORCE
	@echo
	@echo "### VERILATING ###"
	$(VERILATOR) -Wall --trace --no-trace-params --x-assign unique --x-initial unique -cc $(TOP) --exe $(TEST_CPP) -CFLAGS "$(CPP_INC_FLAGS)" $(foreach dir, $(SVLOG_DIRS), -y $(dir)) -public -Wno-PINCONNECTEMPTY -Wno-NULLPORT -Wno-PROCASSINIT -Wno-GENUNNAMED -Wno-IMPLICIT -Wno-PINNOTFOUND -Wno-PINMISSING -Wno-WIDTHEXPAND -Wno-WIDTHTRUNC -Wno-UNUSEDPARAM -Wno-SYNCASYNCNET -Wno-TIMESCALEMOD -Wno-CASEINCOMPLETE
	@touch .stamp.verilate

.PHONY: lint
lint: $(TOP)
	$(VERILATOR) --lint-only $(TOP) $(foreach dir, $(SVLOG_DIRS), -y $(dir))

.PHONY: clean
clean:
	rm -rf .stamp.verilate ./obj_dir waveform.vcd out.yuv