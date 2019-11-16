// SLOB (sorted list of blobs) format reader
#ifndef _SLOB_H
#define _SLOB_H

#include <map>
#include <cmath>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <iostream>
#include "iteration.h"
#include "compression.h"

#define UTF8 "utf-8"
#define MAGIC "!-1SLOB\x1F"

#define DEFAULT_COMPRESSION "lzma2"

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

static const std::map<std::string, std::string> MIME_TYPES {
    { "html", MIME_HTML },
    { "txt", MIME_TEXT }
};

struct SLOBReference {
    // TODO: SWAP CONTENT STORAGE TO UCHAR/UNICODESTRING
    std::string key;
    U_INT bin_index;
    U_SHORT item_index;
    std::string fragment;
};

struct SLOBStoreItem {
    std::vector<U_CHAR> content_type_ids;
    // TODO: SWAP CONTENT STORAGE TO UCHAR/UNICODESTRING
    std::string content;
};

struct SLOBItem {
    std::string content_type;
    // TODO: SWAP CONTENT STORAGE TO UCHAR/UNICODESTRING
    std::string content;
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
};

class SLOBStorageBin {
public:
    SLOBStorageBin(const SLOBStoreItem &, U_INT);
    std::string item(U_SHORT);
    std::string next();

private:
    template<typename LenSpec>
    std::string read_byte_string();
    template<typename LenSpec>
    std::string _read_text();

    std::string read_tiny_text();
    std::string read_text();
    U_INT read_int();

    const SLOBStoreItem &m_store_item;
    std::istringstream m_stream;
    std::vector<U_INT> m_item_positions;
    size_t m_items_data_offset;
    U_INT m_item_count;
};

class SLOBReader {
public:
    SLOBReader();
    ~SLOBReader();

    void open_file(const char *);

    void print_header_info() const;

    std::string uuid() const { return m_header.uuid; }
    std::string encoding() const { return m_header.encoding; }
    std::string compression() const { return m_header.compression; }
    U_INT blob_count() const { return m_header.blob_count; }
    U_INT ref_count() const { return m_references.size(); }
    size_t size() const { return m_header.size; }

    template<typename C>
    void for_each_tag(C) const;

    template<typename C>
    void for_each_content_type(C) const;
    std::string content_type(U_CHAR) const;

    // TODO: CACHE REFERENCES
    template<typename C>
    void for_each_reference(C);
    SLOBReference reference(U_INT);

    template<typename C>
    void for_each_store_item(C);
    SLOBStoreItem store_item(U_INT);

    template<typename C>
    void for_each_item(C);
    std::string item(U_INT, U_SHORT);

private:
    void parse_header();
    void read_store_item_positions();
    void read_reference_positions();
    void read_references();

    std::string (*decompress)(const std::string &) { nullptr };

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

    SLOBHeader m_header;
    std::ifstream m_fp;
    size_t m_filesize;

    std::vector<SLOBReference> m_references;
    std::vector<U_LONG_LONG> m_reference_positions;
    size_t m_reference_data_offset;

    std::vector<U_LONG_LONG> m_store_item_positions;
    size_t m_store_items_data_offset;
};

template<typename C>
void SLOBReader::for_each_reference(C call)
{
    for (auto &ref : m_references)
        if (call(ref))
            break;
}

template<typename C>
void SLOBReader::for_each_tag(C call) const
{
    for (const auto &tag_pair : m_header.tags)
        if (call(tag_pair))
            break;
}

template<typename C>
void SLOBReader::for_each_content_type(C call) const
{
    for (const auto &type : m_header.content_types)
        if (call(type))
            break;
}

template<typename C>
void SLOBReader::for_each_store_item(C call)
{
    m_fp.seekg(m_store_items_data_offset);

    for (U_LONG_LONG &position : m_store_item_positions) {
        m_fp.seekg(m_store_items_data_offset + position);
        SLOBStoreItem item;

        U_INT bin_item_count = read_int();
        char packed_content_type_ids[bin_item_count];
        m_fp.read(packed_content_type_ids, bin_item_count);

        for (unsigned int i = 0; i < bin_item_count; i++)
            item.content_type_ids.push_back(packed_content_type_ids[i]);

        U_INT content_length = read_int();

        item.content.resize(content_length);
        m_fp.read(&item.content[0], content_length);

        if (decompress)
            item.content = decompress(item.content);

        if (call(item))
            break;
    }
}

template<typename C>
void SLOBReader::for_each_item(C call)
{
    m_fp.seekg(m_store_items_data_offset);

    for (U_LONG_LONG &position : m_store_item_positions) {
        m_fp.seekg(m_store_items_data_offset + position);
        SLOBStoreItem store_item;

        U_INT bin_item_count = read_int();
        char packed_content_type_ids[bin_item_count];
        m_fp.read(packed_content_type_ids, bin_item_count);

        for (unsigned int i = 0; i < bin_item_count; i++)
            store_item.content_type_ids.push_back(packed_content_type_ids[i]);

        U_INT content_length = read_int();

        store_item.content.resize(content_length);
        m_fp.read(&store_item.content[0], content_length);

        if (decompress)
            store_item.content = decompress(store_item.content);

        SLOBStorageBin storage_bin(store_item, bin_item_count);

        for (U_INT i = 0; i < bin_item_count; i++) {
            const std::string content = storage_bin.next();
            SLOBItem item = {
                content_type(store_item.content_type_ids[i]),
                content,
            };
            if (call(item))
                return;
        }
    }
}

#endif
