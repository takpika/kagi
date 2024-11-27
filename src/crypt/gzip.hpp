#ifndef CRYPT_GZIP_H
#define CRYPT_GZIP_H

#include <vector>
#include <zlib.h>

std::vector<uint8_t> gzipCompress(const std::vector<uint8_t> &data);
std::vector<uint8_t> gzipDecompress(const std::vector<uint8_t> &data);

#endif