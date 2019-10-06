// SLOB (sorted list of blobs) format reader
#ifndef _SLOB_H
#define _SLOB_H

#include <map>
#include <cmath>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_map>

#define UTF8 "utf-8"
#define MAGIC "!-1SLOB\x1F"

#define DEFAULT_COMPRESSION "zlib"

#define U_CHAR_SIZE 1
#define U_SHORT_SIZE 2
#define U_INT_SIZE 4
#define U_LONG_LONG_SIZE 8

#define calcmax(type) (pow(2, sizeof(type) * 8) - 1)

#define MAX_TEXT_LEN calcmax(unsigned short)
#define MAX_TINY_TEXT_LEN calcmax(unsigned char)
#define MAX_LARGE_BYTE_STRING_LEN calcmax(uint32_t)
#define MAX_BIN_ITEM_COUNT calcmax(unsigned short)

typedef unsigned char U_CHAR;
typedef unsigned short U_SHORT;
typedef unsigned long long U_LONG_LONG;
typedef uint32_t U_INT;

#define MIME_TEXT "text/plain"
#define MIME_HTML "text/html"

static const std::unordered_map<std::string, std::string> MIME_TYPES {
    { "html", MIME_HTML },
    { "txt", MIME_TEXT }
};

/*struct SLOBCompression {
    compress;
    decompress;
};

struct SLOBRef {
    key;
    bin_index;
    item_index;
    fragment;
};*/

struct SLOBHeader {
    std::string uuid;
    std::string encoding;
    std::string compression;
    std::map<std::string, std::string> tags;
    std::vector<std::string> content_types;
    U_INT blob_count;
    U_LONG_LONG store_offset;
    U_LONG_LONG refs_offset;
    size_t size;

    // TODO: remove, this is for testing
    void print()
    {
        std::cout << "Encoding: " << encoding << std::endl <<
                     "Compression: " << compression << std::endl <<
                     "Tags: " << std::endl;

        for (auto const& tag : tags) {
            std::cout << "  " << tag.first <<
                         " : " << tag.second << std::endl;
        }

        std::cout << "Content Types: " << std::endl;

        for (auto const& content_type : content_types) {
            std::cout << "  " << content_type << std::endl;
        }

        std::cout << "Blob Count: " << blob_count << std::endl <<
                     "Store Offset: " << store_offset << std::endl <<
                     "Refs Offset: " << refs_offset << std::endl <<
                     "Size: " << size << std::endl;
        
    }
};

class SLOBReader {
public:
    SLOBReader();
    ~SLOBReader();
    void open_file(const char *file);
private:
    void parse_header();

    std::ifstream m_fp;
    unsigned int filesize;

    template<typename LenSpec>
    std::string read_byte_string();
    template<typename LenSpec>
    std::string _read_text();

    std::string read_tiny_text();
    std::string read_text();
    U_INT read_int();
    U_LONG_LONG read_long();
    U_CHAR read_byte();
    U_SHORT read_short();

    SLOBReader(const SLOBReader&);
    SLOBReader& operator=(const SLOBReader&);

    SLOBHeader m_header;
};

#endif
