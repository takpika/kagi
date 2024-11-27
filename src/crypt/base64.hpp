#ifndef CRYPT_BASE64_H
#define CRYPT_BASE64_H

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <string>
#include <vector>

std::string base64Encode(const std::vector<uint8_t> &data);
std::vector<uint8_t> base64Decode(const std::string &data);

#endif