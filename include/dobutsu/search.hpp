#pragma once
#include <cstdint>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>

namespace dobutsu {
    
    constexpr int VALUE_MATE     = 32000;
    constexpr int VALUE_INFINITE = 32001;

    struct SearchLimits { int depth = 0; };

    struct SearchResult {
        Move bestMove       = Move::NONE;
        int score           = 0;
        int depth           = 0;
        std::uint64_t nodes = 0;
    };

    class Searcher {
        public:
            SearchResult search(Position& pos, const SearchLimits& limits);

        private:
            int alphabeta(Position& pos, int depth, int ply, int alpha, int beta);
            std::uint64_t nodes_{};
    };
}
