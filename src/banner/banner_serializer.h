#ifndef HULK_BANNER_SERIALIZER_H
#define HULK_BANNER_SERIALIZER_H

#include "banner_ir.h"

#include <cstdint>
#include <iosfwd>

namespace Hulk::Banner {

// Footer written at the end of a packaged ./output binary.
inline constexpr uint64_t SERIALIZER_MAGIC   = 0x48554C4B49524600ULL; // "HULKIR\0\0"
inline constexpr uint32_t SERIALIZER_VERSION = 1;

// Serialize prog into out (binary). Throws std::runtime_error on I/O failure.
void write_binary(const BannerProgram& prog, std::ostream& out);

// Deserialize a BannerProgram from in (binary). Throws std::runtime_error
BannerProgram read_binary(std::istream& in);

} 

#endif
