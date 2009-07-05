PPE_SRCS = $(wildcard *ppe*)
SRCS = $(filter-out $(PPE_SRCS),$(wildcard *.c) $(wildcard *.cpp))
ASMS = $(addsuffix .s,$(basename $(SRCS)))
OBJS = $(addsuffix .o,$(basename $(SRCS)))
ELFS = $(addsuffix .elf,$(basename $(SRCS)))
TIMINGS = $(addsuffix .timing,$(ASMS))
EXES = $(basename $(PPE_SRCS))

.PRECIOUS: %.s %.o

all : $(TIMINGS) $(ELFS) $(EXES)

CXX = spu-g++
CXXFLAGS = -fverbose-asm -fdump-tree-final_cleanup -O3

%.s : %.cpp
	$(CXX) $(CXXFLAGS) -S $< -o $@

%.s : %.c
	$(CXX) $(CXXFLAGS) -S $< -o $@

%.s.timing : %.s
	/opt/cell/sdk/usr/bin/spu_timing -running-count $<

%.o : %.s
	$(CXX) $(LDFLAGS) $(LDLIBS) -c $< -o $@

%.elf : %.o
	$(CXX) $(LDFLAGS) $(LDLIBS) $< -o $@

% : %.ppe.cpp
	ppu-g++ -lspe2 $< -o $@
