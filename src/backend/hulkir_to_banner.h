#ifndef HULK_BACKEND_HULKIR_TO_BANNER_H
#define HULK_BACKEND_HULKIR_TO_BANNER_H

#include "../banner/banner_ir.h"
#include "../ir/ir.h"

namespace Hulk::Backend {

class HulkIRToBanner {
public:
    Banner::BannerProgram lower(const IR::IRProgram& program) const;

private:
    std::vector<Banner::BannerType> lower_types(const IR::IRProgram& program) const;
    Banner::BannerInstr lower_instr(const IR::IRInstr& instr,
                                    std::vector<Banner::BannerInstr>& out) const;
};

} 

#endif
