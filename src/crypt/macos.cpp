#include "crypt/macos.h"
#include <iostream>
#include <Security/Security.h>

#define RSA_KEY_LABEL "jp.takpika.kagi.mainkey"

bool createRSAKeyPair() {
    int keySize = 2048; // 鍵サイズ
    CFStringRef label = CFStringCreateWithCString(kCFAllocatorDefault, RSA_KEY_LABEL, kCFStringEncodingUTF8);

    const void *privateKeyAttrsKeys[] = {kSecAttrIsPermanent, kSecAttrLabel};
    const void *privateKeyAttrsValues[] = {kCFBooleanTrue, label};

    CFDictionaryRef privateKeyAttrs = CFDictionaryCreate(
        kCFAllocatorDefault, privateKeyAttrsKeys, privateKeyAttrsValues,
        sizeof(privateKeyAttrsKeys) / sizeof(privateKeyAttrsKeys[0]),
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks
    );

    const void *keys[] = {kSecAttrKeyType, kSecAttrKeySizeInBits, kSecPrivateKeyAttrs};
    const void *values[] = {kSecAttrKeyTypeRSA, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &keySize), privateKeyAttrs};

    CFDictionaryRef parameters = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, sizeof(keys) / sizeof(keys[0]),
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks
    );

    SecKeyRef privateKey = SecKeyCreateRandomKey(parameters, NULL);

    // メモリ解放
    CFRelease(parameters);
    CFRelease(privateKeyAttrs);
    CFRelease(label);

    if (!privateKey) {
        std::cerr << "Error generating RSA key pair" << std::endl;
        return false;
    }

    CFRelease(privateKey);
    return true;
}

bool isRSAKeyPairCreated() {
    CFStringRef label = CFStringCreateWithCString(kCFAllocatorDefault, RSA_KEY_LABEL, kCFStringEncodingUTF8);
    const void *keys[] = {kSecClass, kSecAttrLabel, kSecReturnRef};
    const void *values[] = {kSecClassKey, label, kCFBooleanTrue};

    CFDictionaryRef query = CFDictionaryCreate(NULL, keys, values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    SecKeyRef key = NULL;
    OSStatus status = SecItemCopyMatching(query, (CFTypeRef *)&key);

    CFRelease(label);
    CFRelease(query);
    if (key) CFRelease(key);

    if (status == errSecItemNotFound) {
        return false;
    } else if (status != errSecSuccess) {
        std::cerr << "Error checking for RSA key pair: " << status << std::endl;
        return false;
    }
    return true;
}

bool encryptWithRSAKey(std::vector<uint8_t> &data, std::vector<uint8_t> &encryptedData) {
    if (!isRSAKeyPairCreated()) {
        if (!createRSAKeyPair()) {
            return false;
        }
    }

    CFStringRef label = CFStringCreateWithCString(kCFAllocatorDefault, RSA_KEY_LABEL, kCFStringEncodingUTF8);
    const void *keys[] = {kSecClass, kSecAttrLabel, kSecReturnRef};
    const void *values[] = {kSecClassKey, label, kCFBooleanTrue};

    CFDictionaryRef query = CFDictionaryCreate(NULL, keys, values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    SecKeyRef key = NULL;
    OSStatus status = SecItemCopyMatching(query, (CFTypeRef *)&key);
    SecKeyRef publicKey = SecKeyCopyPublicKey(key);

    CFRelease(label);
    CFRelease(query);
    if (status != errSecSuccess) {
        std::cerr << "Error getting RSA key: " << status << std::endl;
        return false;
    }

    CFDataRef dataRef = CFDataCreate(kCFAllocatorDefault, data.data(), data.size());
    CFErrorRef error = NULL;
    CFDataRef encryptedDataRef = SecKeyCreateEncryptedData(publicKey, kSecKeyAlgorithmRSAEncryptionOAEPSHA512, dataRef, &error);

    CFRelease(key);
    CFRelease(dataRef);

    if (error) {
        CFStringRef errorMsg = CFErrorCopyDescription(error);
        std::cerr << "Encryption error: " << CFStringGetCStringPtr(errorMsg, kCFStringEncodingUTF8) << std::endl;
        CFRelease(errorMsg);
        CFRelease(error);
        return false;
    }

    const UInt8 *bytes = CFDataGetBytePtr(encryptedDataRef);
    size_t length = CFDataGetLength(encryptedDataRef);
    encryptedData.assign(bytes, bytes + length);

    CFRelease(encryptedDataRef);
    return true;
}

bool decryptWithRSAKey(std::vector<uint8_t> &encryptedData, std::vector<uint8_t> &decryptedData) {
    if (!isRSAKeyPairCreated()) {
        if (!createRSAKeyPair()) {
            return false;
        }
    }

    CFStringRef label = CFStringCreateWithCString(kCFAllocatorDefault, RSA_KEY_LABEL, kCFStringEncodingUTF8);
    const void *keys[] = {kSecClass, kSecAttrLabel, kSecReturnRef};
    const void *values[] = {kSecClassKey, label, kCFBooleanTrue};

    CFDictionaryRef query = CFDictionaryCreate(NULL, keys, values, 3, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    SecKeyRef key = NULL;
    OSStatus status = SecItemCopyMatching(query, (CFTypeRef *)&key);

    CFRelease(label);
    CFRelease(query);
    if (status != errSecSuccess) {
        std::cerr << "Error getting RSA key: " << status << std::endl;
        return false;
    }

    CFDataRef encryptedDataRef = CFDataCreate(kCFAllocatorDefault, encryptedData.data(), encryptedData.size());
    CFErrorRef error = NULL;
    CFDataRef decryptedDataRef = SecKeyCreateDecryptedData(key, kSecKeyAlgorithmRSAEncryptionOAEPSHA256, encryptedDataRef, &error);

    CFRelease(key);
    CFRelease(encryptedDataRef);

    if (error) {
        CFStringRef errorMsg = CFErrorCopyDescription(error);
        std::cerr << "Decryption error: " << CFStringGetCStringPtr(errorMsg, kCFStringEncodingUTF8) << std::endl;
        CFRelease(errorMsg);
        CFRelease(error);
        return false;
    }

    const UInt8 *bytes = CFDataGetBytePtr(decryptedDataRef);
    size_t length = CFDataGetLength(decryptedDataRef);
    decryptedData.assign(bytes, bytes + length);

    CFRelease(decryptedDataRef);
    return true;
}

bool encryptKey(AESData key, RSAEncryptedData &encryptedKey) {
    std::vector<uint8_t> keyData(key.key, key.key + sizeof(key.key));
    std::vector<uint8_t> encryptedData;
    if (!encryptWithRSAKey(keyData, encryptedData)) {
        return false;
    }

    if (encryptedData.size() != sizeof(encryptedKey.key)) {
        std::cerr << "Encrypted key size mismatch" << std::endl;
        return false;
    }

    std::copy(encryptedData.begin(), encryptedData.end(), encryptedKey.key);
    return true;
}

bool decryptKey(RSAEncryptedData key, AESData &decryptedKey) {
    std::vector<uint8_t> encryptedData(key.key, key.key + sizeof(key.key));
    std::vector<uint8_t> decryptedData;
    if (!decryptWithRSAKey(encryptedData, decryptedData)) {
        return false;
    }

    if (decryptedData.size() != sizeof(decryptedKey.key)) {
        std::cerr << "Decrypted key size mismatch" << std::endl;
        return false;
    }

    std::copy(decryptedData.begin(), decryptedData.end(), decryptedKey.key);
    return true;
}

AESData generateAESKey() {
    AESData key;
    if (SecRandomCopyBytes(kSecRandomDefault, sizeof(key.key), key.key) != errSecSuccess) {
        std::cerr << "Error generating AES key" << std::endl;
    }
    return key;
}