#include "output_packager.h"

#include "../banner/banner_serializer.h"

#include <cstdint>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef __linux__
#  include <sys/stat.h>
#  include <unistd.h>
#endif

namespace Hulk::Backend {
namespace {

std::string self_exe_path() {
#ifdef __linux__
    char buf[4096] = {};
    const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) throw std::runtime_error("output_packager: cannot resolve /proc/self/exe");
    return std::string(buf, static_cast<std::size_t>(n));
#else
    throw std::runtime_error("output_packager: packaging requires Linux (/proc/self/exe)");
#endif
}

std::vector<char> read_all_bytes(const std::string& path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) throw std::runtime_error("output_packager: cannot open '" + path + "'");
    f.seekg(0, std::ios::end);
    const std::streamoff size = f.tellg();
    if (size < 0) throw std::runtime_error("output_packager: tellg failed on '" + path + "'");
    f.seekg(0, std::ios::beg);
    std::vector<char> bytes(static_cast<std::size_t>(size));
    if (size > 0) f.read(bytes.data(), size);
    if (!f && !f.eof()) throw std::runtime_error("output_packager: read failed on '" + path + "'");
    return bytes;
}

} 

std::optional<Banner::BannerProgram> try_load_embedded_ir() {
#ifndef __linux__
    return std::nullopt;
#else
    std::string exe;
    try { exe = self_exe_path(); } catch (...) { return std::nullopt; }

    std::ifstream in(exe, std::ios::in | std::ios::binary);
    if (!in) return std::nullopt;

    in.seekg(0, std::ios::end);
    const std::streamoff file_size = in.tellg();
    if (file_size < 16) return std::nullopt;

    // Footer: [u64 ir_length][u64 MAGIC]
    in.seekg(-16, std::ios::end);
    uint64_t ir_length = 0, magic = 0;
    in.read(reinterpret_cast<char*>(&ir_length), 8);
    in.read(reinterpret_cast<char*>(&magic), 8);
    if (!in || magic != Banner::SERIALIZER_MAGIC) return std::nullopt;
    if (ir_length == 0 ||
        static_cast<std::streamoff>(ir_length) + 16 > file_size) return std::nullopt;

    in.seekg(-(static_cast<std::streamoff>(ir_length) + 16), std::ios::end);
    if (!in) return std::nullopt;

    try {
        return Banner::read_binary(in);
    } catch (...) {
        return std::nullopt;
    }
#endif
}

void package_output(const Banner::BannerProgram& prog, const std::string& out_path) {
    const std::string exe = self_exe_path();
    const std::vector<char> exe_bytes = read_all_bytes(exe);

    // Serialize the program into a buffer so its length is known up front.
    std::ostringstream blob(std::ios::out | std::ios::binary);
    Banner::write_binary(prog, blob);
    const std::string ir = blob.str();
    const uint64_t ir_length = static_cast<uint64_t>(ir.size());
    const uint64_t magic = Banner::SERIALIZER_MAGIC;

    // Layout: [exe bytes][IR][u64 ir_length][u64 MAGIC]
    std::ofstream out(out_path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("output_packager: cannot create '" + out_path + "'");
    out.write(exe_bytes.data(), static_cast<std::streamsize>(exe_bytes.size()));
    out.write(ir.data(), static_cast<std::streamsize>(ir.size()));
    out.write(reinterpret_cast<const char*>(&ir_length), 8);
    out.write(reinterpret_cast<const char*>(&magic), 8);
    if (!out) throw std::runtime_error("output_packager: write failed for '" + out_path + "'");
    out.close();

#ifdef __linux__
    if (::chmod(out_path.c_str(), 0755) != 0)
        throw std::runtime_error("output_packager: chmod failed for '" + out_path + "'");
#endif
}

} 
