# Requires an IEEE Std 1666-2023 implementation (SystemC 3.0.x).
# Point SYSTEMC_HOME at it, e.g. export SYSTEMC_HOME=$$HOME/opt/systemc-3.0.2
CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O1 -g
LDLIBS   ?= -lsystemc

ifdef SYSTEMC_HOME
CXXFLAGS += -I$(SYSTEMC_HOME)/include
LDFLAGS  += -L$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib
endif

lt_model: lt_model.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

run: lt_model
	./lt_model

clean:
	rm -f lt_model features_2023

all: lt_model features_2023

.PHONY: all run clean check

features_2023: features_2023.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

check: features_2023
	./features_2023
