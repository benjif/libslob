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
        if (terminator != std::string::npos) {
           byte_string = byte_string.substr(0, terminator);
        }
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

void SLOBReader::open_file(const char *file)
{
    m_fp.open(file, std::ios::in | std::ios::binary);
    if (!m_fp)
        throw std::invalid_argument("Could not open SLOB file");

    m_fp.seekg(0, m_fp.end);
    filesize = m_fp.tellg();
    m_fp.seekg(0, m_fp.beg);

    parse_header();
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
    if (m_header.compression.compare(DEFAULT_COMPRESSION) != 0)
        throw std::runtime_error("Compression unsupported (zlib only)");

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
    if (m_header.store_offset > filesize)
        throw std::runtime_error("Store offset too large");

    m_header.size = read_long();
    m_header.refs_offset = m_fp.tellg();

    if (filesize != m_header.size)
        throw std::runtime_error("Filesize and header size don't match");

    // TODO: remove, this is for testing
    m_header.print();
}

std::string SLOBReader::content_type(U_CHAR id)
{
    if (id >= m_header.content_types.size())
        throw std::runtime_error("Content type ID is out of bounds");

    return m_header.content_types[id];
}
