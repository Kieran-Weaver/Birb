CC=gcc
CXX=g++
TARGET=Birb
INC_FLAGS := -I include
CFLAGS := -O3 -march=native
CXXFLAGS := -O3 -march=native -std=c++17
CPPFLAGS = $(INC_FLAGS) -MT $@ -MMD -MP -MF build/$*.d
SRCS := $(shell find src -path "*.cpp")
OBJS := $(patsubst %.c, ./build/%.o, $(patsubst %.cpp, ./build/%.o, $(SRCS)))
DEPS := $(OBJS:.o=.d)

LDFLAGS=-lssl -lcrypto -lpthread -O3 -march=native -Wl,-O1

.PHONY: all clean

all: build $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -o $(TARGET) $(LDFLAGS)

./build/%.o : ./%.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

./build/%.o : ./%.c
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -fr $(TARGET) build

build:
	mkdir build
	cd build && mkdir -p src/core src/util src/goose

-include $(DEPS)

