#include "slob.h"
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unicode/unistr.h>
#include <unicode/ustream.h>

static bool little_endian()
{
    short int n = 0x1;
    char *np = (char *)&n;
    return (np[0] == 1);
}

template <typename T>
static T swap_endian(T value)
{
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } src, dst;
    src.u = value;
    for (size_t i = 0; i < sizeof(T); i++)
        dst.u8[i] = src.u8[sizeof(T) - i - 1];
    return dst.u;
}

// TODO: remove, this is for testing
std::ostream &operator<<(std::ostream &os, const SLOBHeader &h)
{
    os << "Encoding: " << h.encoding << '\n' <<
        "Compression: " << h.compression << '\n' <<
        "Tags: " << '\n';

    for (auto const& tag : h.tags) {
        os << "  " << tag.first <<
            " : " << tag.second << '\n';
    }

    os << "Content Types: " << '\n';

    for (auto const& content_type : h.content_types) {
        os << "  " << content_type << '\n';
    }

    os << "Blob Count: " << h.blob_count << '\n' <<
        "Store Offset: " << h.store_offset << '\n' <<
        "Refs Offset: " << h.refs_offset << '\n' <<
        "Size: " << h.size << '\n';

    return os;
}

SLOBStorageBin::SLOBStorageBin(const SLOBStoreItem &store_item, U_INT item_count)
    : m_store_item(store_item), m_item_count(item_count)
{
    m_stream.str(m_store_item.content);
    m_item_positions.reserve(m_item_count);
    for (U_INT i = 0; i < m_item_count; i++)
        m_item_positions[i] = read_int();
    m_items_data_offset = m_stream.tellg(); 
}

template <typename LenSpec>
std::string SLOBStorageBin::read_byte_string()
{
    std::string length_bytes(sizeof(LenSpec), '\0');
    m_stream.read(&length_bytes[0], sizeof(LenSpec));
    std::reverse(length_bytes.begin(), length_bytes.end());

    LenSpec length;
    std::memcpy(&length, &length_bytes[0], sizeof(LenSpec));

    std::string read_bytes(length, '\0');
    m_stream.read(&read_bytes[0], length);
    return read_bytes;
}

template <typename LenSpec>
std::string SLOBStorageBin::_read_text()
{
    size_t max_len = calcmax(LenSpec);
    std::string byte_string = read_byte_string<LenSpec>();
    if (byte_string.length() == max_len) {
        size_t terminator = byte_string.find('\0');
        if (terminator != std::string::npos) {
           byte_string = byte_string.substr(0, terminator);
        }
    }
    return byte_string;
}

std::string SLOBStorageBin::read_tiny_text()
{
    return _read_text<U_CHAR>();
}

std::string SLOBStorageBin::read_text()
{
    return _read_text<U_SHORT>();
}

U_INT SLOBStorageBin::read_int()
{
    U_INT read;
    m_stream.read(reinterpret_cast<char *>(&read), sizeof(read));
    if (little_endian())
        read = swap_endian(read);
    return read;
}

std::string SLOBStorageBin::next()
{
    return read_text();
}

std::string SLOBStorageBin::item(U_SHORT index)
{
    if (index > m_item_count)
        throw std::runtime_error("Item index out of bounds");

    m_stream.seekg(m_items_data_offset + m_item_positions[index]);
    U_INT length = read_int();
    std::string buffer;
    buffer.resize(length);
    m_stream.read(&buffer[0], length);
    return buffer;
}

SLOBReader::SLOBReader()
{
}

SLOBReader::~SLOBReader()
{
    m_fp.close();
}

template <typename LenSpec>
std::string SLOBReader::read_byte_string()
{
    std::string length_bytes(sizeof(LenSpec), '\0');
    m_fp.read(&length_bytes[0], sizeof(LenSpec));
    std::reverse(length_bytes.begin(), length_bytes.end());

    LenSpec length;
    std::memcpy(&length, &length_bytes[0], sizeof(LenSpec));

    std::string read_bytes(length, '\0');
    m_fp.read(&read_bytes[0], length);
    return read_bytes;
}

template <typename LenSpec>
std::string SLOBReader::_read_text()
{
    size_t max_len = calcmax(LenSpec);
    std::string byte_string = read_byte_string<LenSpec>();
    if (byte_string.length() == max_len) {
        size_t terminator = byte_string.find('\0');
        if (terminator != std::string::npos)
           byte_string = byte_string.substr(0, terminator);
    }
    return byte_string;
}

std::string SLOBReader::read_tiny_text()
{
    return _read_text<U_CHAR>();
}

std::string SLOBReader::read_text()
{
    return _read_text<U_SHORT>();
}

U_INT SLOBReader::read_int()
{
    U_INT read;
    m_fp.read(reinterpret_cast<char *>(&read), sizeof(read));
    if (little_endian())
        read = swap_endian(read);
    return read;
}

U_LONG_LONG SLOBReader::read_long()
{
    U_LONG_LONG read;
    m_fp.read(reinterpret_cast<char *>(&read), sizeof(read));
    if (little_endian())
        read = swap_endian(read);
    return read;
}

U_CHAR SLOBReader::read_byte()
{
    char read;
    m_fp.read(&read, 1);
    return (U_CHAR)read;
}

U_SHORT SLOBReader::read_short()
{
    U_SHORT read;
    m_fp.read(reinterpret_cast<char *>(&read), sizeof(read));
    if (little_endian())
        read = swap_endian(read);
    return read;
}

void SLOBReader::open_file(const char *filename)
{
    m_fp.open(filename, std::ios::in | std::ios::binary);
    if (!m_fp)
        throw std::invalid_argument("Could not open SLOB file");

    m_fp.seekg(0, m_fp.end);
    m_filesize = m_fp.tellg();
    m_fp.seekg(0, m_fp.beg);

    parse_header();
    read_reference_positions();
    read_store_item_positions();
}

void SLOBReader::parse_header()
{
    std::string read_magic(8, '\0');
    m_fp.read(&read_magic[0], 8);

    if (read_magic.compare(MAGIC) != 0)
        throw std::runtime_error("Incorrect magic text value");

    m_fp.read(&(m_header.uuid[0]), 16);

    m_header.encoding = read_byte_string<U_CHAR>();
    if (m_header.encoding.compare(UTF8) != 0)
        throw std::runtime_error("Encoding unsupported (utf-8 only)");

    m_header.compression = read_tiny_text();

    if (!m_header.compression.empty())
        decompress = COMPRESSION.at(m_header.compression);

    U_CHAR count = read_byte();
    for (U_CHAR i = 0; i < count; i++) {
        m_header.tags.insert(std::make_pair(read_tiny_text(), read_tiny_text()));
    }

    count = read_byte();
    for (U_CHAR i = 0; i < count; i++) {
        m_header.content_types.push_back(read_text());
    }

    m_header.blob_count = read_int();
    m_header.store_offset = read_long();
    if (m_header.store_offset > m_filesize)
        throw std::runtime_error("Store offset too large");

    m_header.size = read_long();
    m_header.refs_offset = m_fp.tellg();

    if (m_filesize != m_header.size)
        throw std::runtime_error("Incorrect filesize");
}

void SLOBReader::read_store_item_positions()
{
    m_fp.seekg(m_header.store_offset);
    U_INT item_positions_count = read_int();

    m_store_item_positions.resize(item_positions_count);

    for (U_INT i = 0; i < item_positions_count; i++)
        m_store_item_positions[i] = read_long();

    m_store_items_data_offset = m_fp.tellg();
}

void SLOBReader::read_reference_positions()
{
    m_fp.seekg(m_header.refs_offset);
    U_INT reference_positions_count = read_int();

    m_reference_positions.resize(reference_positions_count);

    for (U_INT i = 0; i < reference_positions_count; i++)
        m_reference_positions[i] = read_long();

    m_reference_data_offset = m_fp.tellg();
}

std::string SLOBReader::content_type(U_CHAR id) const
{
    if (id > m_header.content_types.size())
        throw std::runtime_error("Content type ID is out of bounds");

    return m_header.content_types[id];
}

SLOBReference SLOBReader::reference(U_INT index)
{
    if (index >= m_reference_positions.size())
        throw std::runtime_error("Reference index out of bounds");

    m_fp.seekg(m_header.refs_offset + m_reference_positions[index]);

    return {
        read_text(),
        read_int(),
        read_short(),
        read_tiny_text()
    };
}

SLOBStoreItem SLOBReader::store_item(U_INT index)
{
    if (index >= m_store_item_positions.size())
        throw std::runtime_error("Store item index out of bounds");

    m_fp.seekg(m_store_items_data_offset + m_store_item_positions[index]);

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

    return item;
}

std::string SLOBReader::item(U_INT bin_index, U_SHORT bin_item_index)
{
    if (bin_index >= m_store_item_positions.size())
        throw std::runtime_error("Bin index out of bounds");

    m_fp.seekg(m_store_items_data_offset + m_store_item_positions[bin_index]);

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

    //for (int i = 0; i < bin_item_count; i++)
    //    std::cout << storage_bin.next() << '\n';

    return storage_bin.item(bin_item_index);
}
