#include "NBTReader.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include "zlib.h"


namespace levishematic::nbtreader {

// ============================================================
// GZip 解压（zlib inflateInit2 with windowBits=15+16）
// ============================================================
std::string gzipDecompress(const void* src, size_t srcLen) {
    std::string out;
    out.resize(srcLen * 4); // 初始缓冲猜测

    z_stream strm{};
    strm.next_in  = reinterpret_cast<Bytef*>(const_cast<void*>(src));
    strm.avail_in = static_cast<uInt>(srcLen);

    // windowBits = 15+16 → 自动识别 GZip 格式
    if (inflateInit2(&strm, 15 + 16) != Z_OK) throw std::runtime_error("inflateInit2 failed");

    while (true) {
        strm.next_out  = reinterpret_cast<Bytef*>(out.data() + strm.total_out);
        strm.avail_out = static_cast<uInt>(out.size() - strm.total_out);

        int ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_END) break;
        if (ret == Z_BUF_ERROR || strm.avail_out == 0) {
            out.resize(out.size() * 2);
        } else if (ret != Z_OK) {
            inflateEnd(&strm);
            throw std::runtime_error(std::string("inflate failed: ") + (strm.msg ? strm.msg : "unknown"));
        }
    }
    out.resize(strm.total_out);
    inflateEnd(&strm);
    return out;
}

CompoundTag loadNbtFromFile(const std::filesystem::path& path) {
    // ---- 1. 读取原始字节 ----
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path.string());

    auto fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> rawData(fileSize);
    if (!file.read(reinterpret_cast<char*>(rawData.data()), static_cast<std::streamsize>(fileSize)))
        throw std::runtime_error("Failed to read file: " + path.string());

    // ---- 2. GZip 解压 ----
    std::string nbtData = gzipDecompress(rawData.data(), rawData.size());

    // ---- 3. 用 LeviLamina API 解析 Big-Endian NBT ----
    //   CompoundTag::fromBinaryNbt(data, isLittleEndian=false)
    //   .litematic 使用 Java Edition Big-Endian 格式
    auto result = CompoundTag::fromBinaryNbt(
        std::string_view{nbtData.data(), nbtData.size()},
        false // Big-Endian = Java Edition
    );

    if (!result) {
        throw std::runtime_error("Failed to parse NBT from '" + path.string() + "': " + result.error().message());
    }

    return std::move(*result);
}
} // namespace levishematic::nbtreader
