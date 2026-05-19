#ifndef HULK_BANNER_PRINTER_H
#define HULK_BANNER_PRINTER_H

#include "banner_ir.h"

#include <string>

namespace Hulk::Banner {

class BannerPrinter {
public:
    std::string print(const BannerProgram& program) const;

private:
    std::string print_instr(const BannerInstr& instr) const;
    std::string escape_string(const std::string& value) const;
    std::string join_args(const std::vector<std::string>& args) const;
};

} 
#endif
