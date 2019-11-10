SRC_DIRS ?= ./src
SRC_FILES := $(shell find $(SRC_DIRS) -name *.cpp)

$(shell mkdir -p build)
lexis: $(SRC_FILES)
	g++ -o build/lexis $(SRC_FILES) \
		-Iinclude -ltermbox -llzma -lz -Wall -Wno-missing-field-initializers -g -DU_CHARSET_IS_UTF8=1
