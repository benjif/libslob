SRC_DIRS ?= ./src
SRC_FILES := $(shell find $(SRC_DIRS) -name *.cpp)

$(shell mkdir -p build)

test: tests/main.cpp $(SRC_FILES)
	g++ -o build/test tests/main.cpp $(SRC_FILES) \
		-Iinclude -llzma -lz -Wall -Wno-parentheses -Wno-missing-field-initializers -DU_CHARSET_IS_UTF8=1 `pkg-config --libs --cflags icu-uc icu-io` -O2

clean:
	rm build/test
