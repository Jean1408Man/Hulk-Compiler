#pragma once
#include <cstddef>

namespace hulk::common {

struct Position {
    std::size_t index = 0;   
    std::size_t line = 1;    // línea 1-based
    std::size_t column = 1;  // columna 1-based
};

}