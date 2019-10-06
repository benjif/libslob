#include "slob.h"
#include <zlib.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unicode/unistr.h>
#include <unicode/ustream.h>

static bool little_endian()
{
    short int n = 0x1;
    char *np = (char*)&n;
    return (np[0] == 1);
}

template <typename T>
static T endian_swap(T value)
{
    union {
        T u;
        unsigned char u8[sizeof(T)];
    } src, dst;
    src.u = value;
    for (size_t k = 0; k < sizeof(T); k++)
        dst.u8[k] = src.u8[sizeof(T) - k - 1];
    return dst.u;
}

static std::string zlib_inflate(std::string &in)
{
    z_stream inf_stream;
    memset(&inf_stream, 0, sizeof(inf_stream));

    if (inflateInit(&inf_stream) != Z_OK)
        throw std::runtime_error("zLib inflateInit() failed");

    inf_stream.next_in = (Bytef*)in.data();
    inf_stream.avail_in = in.size();

    int ret;
    char out_buffer[32768];
    std::string out_string;

    do {
        inf_stream.next_out = reinterpret_cast<Bytef*>(out_buffer);
        inf_stream.avail_out = sizeof(out_buffer);
        ret = inflate(&inf_stream, 0);
        if (out_string.size() < inf_stream.total_out) {
            out_string.append(out_buffer, inf_stream.total_out - out_string.size());
        }
    } while (ret = Z_OK);

    inflateEnd(&inf_stream);

    if (ret != Z_STREAM_END) {
        std::ostringstream oss;
        oss << "Exception occurred during zLib inflation: " << inf_stream.msg;
        throw std::runtime_error(oss.str());
    }

    return out_string;
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
        read = endian_swap(read);
    return read;
}

U_LONG_LONG SLOBReader::read_long()
{
    U_LONG_LONG read;
    m_fp.read(reinterpret_cast<char *>(&read), sizeof(read));
    if (little_endian())
        read = endian_swap(read);
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
        read = endian_swap(read);
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
    m_header.size = read_long();
    m_header.refs_offset = m_fp.tellg();

    m_header.print();
}
