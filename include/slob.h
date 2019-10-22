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
#include "compression.h"

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
};*/

struct SLOBRef {
    std::string key;
    U_INT bin_index;
    U_SHORT item_index;
    std::string fragment;
};

struct SLOBStoreItem {
    std::string content_type_ids;
    std::string compressed_content;
};

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
        std::cout << "Encoding: " << encoding << '\n' <<
                     "Compression: " << compression << '\n' <<
                     "Tags: " << '\n';

        for (auto const& tag : tags) {
            std::cout << "  " << tag.first <<
                         " : " << tag.second << '\n';
        }

        std::cout << "Content Types: " << '\n';

        for (auto const& content_type : content_types) {
            std::cout << "  " << content_type << '\n';
        }

        std::cout << "Blob Count: " << blob_count << '\n' <<
                     "Store Offset: " << store_offset << '\n' <<
                     "Refs Offset: " << refs_offset << '\n' <<
                     "Size: " << size << '\n';
        
    }
};

class SLOBReader {
public:
    SLOBReader();
    ~SLOBReader();
    void open_file(const char *file);

    template<typename C>
    void for_each_tag(C call) const
    {
        for (const auto &tag_pair : m_header.tags) {
            call(tag_pair);
        }
    }

    template<typename C>
    void for_each_content_type(C call) const
    {
        for (const auto &type : m_header.content_types) {
            call(type);
        }
    }

    template<typename C>
    void for_each_reference(C);
    template<typename C>
    void for_each_store_item(C);

    std::string content_type(U_CHAR id);
private:
    void parse_header();
    void read_references();
    void read_store_items();

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
    std::vector<SLOBRef> m_references;
    std::vector<SLOBStoreItem> m_store_items;

    std::ifstream m_fp;
    unsigned int filesize;
};

template<typename C>
void SLOBReader::for_each_reference(C call)
{
    unsigned int file_pos = m_fp.tellg();
    for (; file_pos+9 <= m_header.store_offset; file_pos = m_fp.tellg()) {
        SLOBRef cur_ref {
            read_text(),
            read_int(),
            read_short(),
            read_tiny_text()
        };
        call(cur_ref);
    }
}

template<typename C>
void SLOBReader::for_each_store_item(C call)
{
    m_fp.seekg(m_header.store_offset, std::ios::beg);
    U_LONG_LONG file_pos = m_fp.tellg();
    std::cout << "STARTING FILE_POS: " << file_pos << '\n';
    for (; file_pos < filesize; file_pos = m_fp.tellg()) {
        SLOBStoreItem item;

        U_INT bin_item_count = read_int();
        std::cout << bin_item_count << " bin items" << '\n';
        char packed_content_type_ids[bin_item_count];
        m_fp.read(packed_content_type_ids, bin_item_count);

        for (unsigned int i = 0; i < bin_item_count; i++) {
            item.content_type_ids.push_back(packed_content_type_ids[i]);
        }

        U_INT content_length = read_int();
        item.compressed_content.resize(content_length);
        m_fp.read(&item.compressed_content[0], content_length);

        std::cout << zlib_inflate(item.compressed_content) << '\n';

        call(item);
    }
}

#endif
