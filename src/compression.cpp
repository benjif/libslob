#include "compression.h"
#include <zlib.h>
#include <lzma.h>
#include <cstring>
#include <sstream>
#include <iostream>
#include <stdexcept>

const std::map<std::string, std::string (*)(const std::string &)> COMPRESSION = {
    {
        "lzma2",
        [](const std::string &in) -> std::string {
            lzma_stream strm = LZMA_STREAM_INIT;
  
            lzma_options_lzma opt_lzma2 {
                .dict_size = 8U << 20
            };
  
            lzma_filter filters[] = {
                { LZMA_FILTER_LZMA2, &opt_lzma2 },
                { LZMA_VLI_UNKNOWN, NULL },
            };
  
            lzma_ret ret = lzma_raw_decoder(&strm, filters);
            if (ret != LZMA_OK)
                throw std::runtime_error("LZMA: lzma_raw_decoder failed");
  
            lzma_action action = LZMA_RUN;
  
            uint8_t out_buffer[BUFSIZ];
            std::string out_string;
  
            strm.next_in = (unsigned char*)in.data();
            strm.avail_in = in.size();
            strm.next_out = out_buffer;
            strm.avail_out = BUFSIZ;
  
            while (true) {
                strm.next_out = out_buffer;
                strm.avail_out = BUFSIZ;
                ret = lzma_code(&strm, action);
  
                if (strm.avail_out == 0 || ret == LZMA_STREAM_END) {
                    strm.next_out = out_buffer;
                    strm.avail_out = BUFSIZ;
                }
  
                if (ret != LZMA_OK) {
                    if (ret == LZMA_STREAM_END)
                        break;
                    const std::string errmsg = [&ret]() {
                        switch(ret) {
                        case LZMA_MEM_ERROR:
                            return "LZMA: Memory allocation failed";
                            break;
                        case LZMA_FORMAT_ERROR:
                            return "LZMA: Input is not in the .xz format";
                            break;
                        case LZMA_OPTIONS_ERROR:
                            return "LZMA: Unsupported compression options";
                            break;
                        case LZMA_DATA_ERROR:
                            return "LZMA: Compressed data is corrupt";
                            break;
                        case LZMA_BUF_ERROR:
                            return "LZMA: Compressed data is truncated or corrupt";
                            break;
                        default:
                            return "LZMA: Unknown error";
                            break;
                        }
                    }();
                    std::cout << errmsg << '\n';
                    throw std::runtime_error("LZMA: Error during LZMA2 decompression");
                }
  
                if (out_string.size() < strm.total_out)
                    out_string.append(reinterpret_cast<char *>(out_buffer), strm.total_out - out_string.size());
                else
                    action = LZMA_FINISH;
            }
  
            lzma_end(&strm);
            return out_string;
        },
    },
    {
        "zlib",
        [](const std::string &in) -> std::string {
            z_stream inf_stream;
            memset(&inf_stream, 0, sizeof(inf_stream));
  
            if (inflateInit(&inf_stream) != Z_OK)
                throw std::runtime_error("ZLIB: zLib inflateInit() failed");
  
            inf_stream.next_in = (Bytef*)in.data();
            inf_stream.avail_in = in.size();
  
            int ret;
            char out_buffer[BUFSIZ];
            std::string out_string;
  
            do {
                inf_stream.next_out = reinterpret_cast<Bytef*>(out_buffer);
                inf_stream.avail_out = BUFSIZ;
                ret = inflate(&inf_stream, Z_NO_FLUSH);
                if (out_string.size() < inf_stream.total_out) {
                    out_string.append(out_buffer, inf_stream.total_out - out_string.size());
                }
            } while (ret == Z_OK);
  
            inflateEnd(&inf_stream);
  
            if (ret != Z_STREAM_END) {
                std::ostringstream oss;
                oss << "ZLIB: Exception occurred during zLib inflation: " << inf_stream.msg;
                throw std::runtime_error(oss.str());
            }
  
            return out_string;
        }
    }
};
