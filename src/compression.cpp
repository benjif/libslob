#include "compression.h"
#include <zlib.h>
//#include <lzma.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdexcept>

std::string zlib_inflate(const std::string &in)
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

/*static void init_lzma_decoder(lzma_stream *strm)
{
    lzma_ret ret = lzma_stream_decoder(
            strm, UINT64_MAX, LZMA_CONCATENATED);

    if (ret == LZMA_OK)
        return;

    std::string msg;
    switch (ret) {
    case LZMA_MEM_ERROR:
        msg = "Memory allocation failed";
        break;
    case LZMA_OPTIONS_ERROR:
        msg = "Unsupported decompressor flags";
        break;
    default:
        msg = "Unknown error, possibly a bug";
        break;
    }

    throw std::runtime_error("Error initializing LZMA decoder: " + msg);
}

//std::string lzma_decompress(lzma_stream *strm, const char *inname, FILE *infile, FILE *outfile)
std::string lzma_decompress(lzma_stream *strm, const std::string &in)
{
    lzma_action action = LZMA_RUN;

    uint8_t inbuf[BUFSIZ];
    //uint8_t out_buffer[BUFSIZ];
    uint8_t out_buffer[BUFSIZ];

    strm->next_in = NULL;
    strm->avail_in = 0;
    strm->next_out = (unsigned char *)in.data();
    strm->avail_out = in.size();

    std::string out_string;

    while (true) {
        strm->next_out = reinterpret_cast<unsigned char *>(out_buffer);
        strm->avail_out = sizeof(out_buffer);

        if (out_string.size() >= strm->total_out)
            action = LZMA_FINISH;

        lzma_ret ret = lzma_code(strm, action);

        if (ret == LZMA_STREAM_END) {
            size_t write_size = sizeof(out_buffer) - strm->avail_out;
            out_string.append((char *)out_buffer, write_size);
            strm->next_out = out_buffer;
            strm->avail_out = sizeof(out_buffer);
        }

        if (ret != LZMA_OK) {
            if (ret == LZMA_STREAM_END)
                break;
            std::string msg;
            switch (ret) {
            case LZMA_MEM_ERROR:
                msg = "Memory allocation failed";
                break;
            case LZMA_FORMAT_ERROR:
                msg = "The input is not in the .xz format";
                break;
            case LZMA_OPTIONS_ERROR:
                msg = "Unsupported compression options";
                break;
            case LZMA_DATA_ERROR:
                msg = "Compressed file is corrupt";
                break;
            case LZMA_BUF_ERROR:
                msg = "Compressed file is truncated or "
                        "otherwise corrupt";
                break;
            default:
                msg = "Unknown error, possibly a bug";
                break;
            }
            throw std::runtime_error("LZMA decoder error: " + msg);
        }
    }

    return out_string;
}

std::string lzma_inflate(const std::string &in)
{
    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_options_lzma opt_lzma2;
    if (lzma_lzma_preset(&opt_lzma2, LZMA_PRESET_DEFAULT)) {
        std::cerr << "Unsupported preset" << std::endl;
        return "";
    }
    if (!init_lzma_decoder(&strm)) {
    }

}*/
