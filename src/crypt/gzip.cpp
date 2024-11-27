#include <crypt/gzip.hpp>

std::vector<uint8_t> gzipCompress(const std::vector<uint8_t> &data) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = data.size();
    stream.next_in = (Bytef *)data.data();

    if (deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> compressedData(1024);
    stream.avail_out = compressedData.size();
    stream.next_out = (Bytef *)compressedData.data();

    if (deflate(&stream, Z_FINISH) != Z_STREAM_END) {
        deflateEnd(&stream);
        return std::vector<uint8_t>();
    }

    deflateEnd(&stream);
    compressedData.resize(stream.total_out);
    return compressedData;
}

std::vector<uint8_t> gzipDecompress(const std::vector<uint8_t> &data) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = data.size();
    stream.next_in = (Bytef *)data.data();

    if (inflateInit2(&stream, 15 | 16) != Z_OK) {
        return std::vector<uint8_t>();
    }

    std::vector<uint8_t> decompressedData(1024);
    stream.avail_out = decompressedData.size();
    stream.next_out = (Bytef *)decompressedData.data();

    if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
        inflateEnd(&stream);
        return std::vector<uint8_t>();
    }

    inflateEnd(&stream);
    decompressedData.resize(stream.total_out);
    return decompressedData;
}