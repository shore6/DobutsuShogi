#pragma once
#include <cstdint>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/evaluate.hpp>


namespace dobutsu {
    
    constexpr int VALUE_MATE     = 32000;
    constexpr int VALUE_INFINITE = 32001;
    constexpr int MAX_PLY        = 128;

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

static_assert(dobutsu::VALUE_INFINITE > dobutsu::VALUE_MATE);
static_assert(dobutsu::EVAL_MAX < dobutsu::VALUE_MATE - dobutsu::MAX_PLY);
