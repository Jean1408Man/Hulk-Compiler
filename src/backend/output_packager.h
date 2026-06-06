#ifndef HULK_OUTPUT_PACKAGER_H
#define HULK_OUTPUT_PACKAGER_H

#include "../banner/banner_ir.h"

#include <optional>
#include <string>

namespace Hulk::Backend {

// Detect whether the running executable carries an embedded BannerProgram.
// Reads /proc/self/exe, checks the 16-byte footer [u64 ir_length][u64 MAGIC],
// and deserializes the blob if present. Returns nullopt when running as the
// plain compiler (./hulk) rather than a packaged program (./output).
std::optional<Banner::BannerProgram> try_load_embedded_ir();

// Produce an executable at out_path by copying the running compiler binary
// (/proc/self/exe) and appending the serialized program plus the footer.
// The result is marked executable. Throws std::runtime_error on I/O failure.
void package_output(const Banner::BannerProgram& prog, const std::string& out_path);

}

#endif
