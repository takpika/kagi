#ifndef CRYPT_MACOS_H
#define CRYPT_MACOS_H

#include <structs/keys.hpp>

bool encryptKey(AESData key, RSAEncryptedData &encryptedKey);
bool decryptKey(RSAEncryptedData key, AESData &decryptedKey);
AESData generateAESKey();

#endif