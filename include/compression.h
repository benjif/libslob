#ifndef _COMPRESSION_H
#define _COMPRESSION_H

#include <string>
#include <map>

typedef std::string (*decompress_function)(const std::string &);

// DECOMPRESSION FUNCTIONS
// COMPRESSION.at(<compression type>)
extern const std::map<std::string, decompress_function> COMPRESSION;

#endif
