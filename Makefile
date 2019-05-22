CXX=g++
CXXFLAGS=-Wall -Wextra -Werror -Weffc++ -std=c++17 -pedantic -g
LDFLAGS=-lavutil -lavformat -lavcodec -lz -lm
.PHONY: clean

all: gridmedian

gridmedian: gridmedian.cc video.cc video.h grid.h
	$(CXX) $(CXXFLAGS) -o gridmedian gridmedian.cc video.cc $(LDFLAGS)

unittest: unittest.cc unittest_grid.cc
	$(CXX) $(CXXFLAGS) -o gridmedian_unittest unittest.cc unittest_grid.cc
	./gridmedian_unittest

clean:
	rm -f gridmedian gridmedian_unittest frame*.ppm