# Copyright (c) 2014 Baidu.com, Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file. See the AUTHORS file for names of contributors.

#-----------------------------------------------
# Modify prefix to specify the directory to install logbook.
#
PREFIX=./output
#-----------------------------------------------

#-----------------------------------------------
# Uncomment exactly one of the lines labelled (A), (B), and (C) below
# to switch between compilation modes.
#
OPT ?= -O2        # (A) Production use (optimized mode)
# OPT ?= -g2      # (B) Debug mode, generate full line-level debugging symbols
# OPT ?= -O2 -g2  # (C) Profiling mode: opt, but generate debugging symbols
#-----------------------------------------------

#-----------------------------------------------
# !!! Do not change the following lines !!!
#-----------------------------------------------

include depends.mk

LIB=liblogbook.a
LIB_SRC=$(wildcard src/common/*.cc) \
		$(wildcard src/core/*.cc)   \
		$(wildcard src/log/*.cc)    \
		$(wildcard src/rpc/*.cc)    \
		$(wildcard src/proto/*.cc)
LIB_OBJ=$(patsubst %.cc,%.o,$(LIB_SRC))
PROTO=$(wildcard src/proto/*.proto)
PROTO_SRC=$(patsubst %.proto,%.pb.cc,$(PROTO))
PROTO_HEADER=$(patsubst %.proto,%.pb.h,$(PROTO))
PROTO_OBJ=$(patsubst %.cc,%.o,$(PROTO_SRC))

BIN=logbook
BIN_SRC=$(wildcard test/*.cc)
BIN_OBJ=$(patsubst %.cc,%.o,$(BIN_SRC))

#-----------------------------------------------
ifeq ($(OS),Windows_NT)
    LDFLAGS += -lrt
    TARGET_DIRECTORY := --target-directory
else
    UNAME_S := $(shell uname -s)
    
    ifeq ($(UNAME_S),Linux)
        LDFLAGS += -lrt
        TARGET_DIRECTORY := --target-directory
    endif
    
    ifeq ($(UNAME_S),Darwin)
        TARGET_DIRECTORY := 
    endif
endif

#-----------------------------------------------

CXX=g++
INCPATH=-Isrc -I$(BOOST_HEADER_DIR) -I$(PROTOBUF_DIR)/include
CXXFLAGS += $(OPT) -pipe -W -Wall -fPIC -D_GNU_SOURCE -D__STDC_LIMIT_MACROS $(INCPATH)

LDFLAGS += -L$(PROTOBUF_DIR)/lib -lprotobuf -lpthread
.PHONY: check_depends build rebuild clean

all: build

check_depends:
	@if [ ! -f "$(BOOST_HEADER_DIR)/boost/smart_ptr.hpp" ]; then echo "ERROR: need boost header"; exit 1; fi

clean:
	rm -f $(LIB) $(BIN) $(LIB_OBJ) $(PROTO_OBJ) $(BIN_OBJ) $(PROTO_HEADER) $(PROTO_SRC)

rebuild: clean all

$(PROTO_OBJ): $(PROTO_HEADER)

$(LIB_OBJ): $(PROTO_HEADER)

$(LIB): $(LIB_OBJ) $(PROTO_OBJ)
	ar crs $@ $(LIB_OBJ) $(PROTO_OBJ)

$(BIN): $(LIB) $(BIN_OBJ)
	$(CXX) $(BIN_OBJ) -o $@ $(LIB) $(LDFLAGS)

%.pb.cc %.pb.h: %.proto
	${PROTOBUF_DIR}/bin/protoc --proto_path=./src/proto --proto_path=${PROTOBUF_DIR}/include --cpp_out=./src/proto $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

build: $(LIB) $(BIN)
	@echo
	@echo 'Build succeed.'
