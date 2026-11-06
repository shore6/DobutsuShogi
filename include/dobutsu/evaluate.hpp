#pragma once
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>

namespace dobutsu {
    
    constexpr int EVAL_MAX = 3000;
    int evaluate(const Position& pos);
}
