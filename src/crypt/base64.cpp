#include <crypt/base64.hpp>

std::string base64Encode(const std::vector<uint8_t> &data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, data.data(), data.size());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);

    BIO_free_all(bio);

    return result;
}

std::vector<uint8_t> base64Decode(const std::string &data) {
    BIO *bio, *b64;

    std::vector<uint8_t> result(data.size());

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(data.data(), data.size());
    bio = BIO_push(b64, bio);

    int length = BIO_read(bio, result.data(), result.size());

    BIO_free_all(bio);

    result.resize(length);

    return result;
}