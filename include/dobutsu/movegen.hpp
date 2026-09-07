#pragma once
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <cassert>

/* MAX_MOVES 概算
Move数 = 移動先の数 * 今いる場所の数;

移動 = 7 * 8 = 56;      // 盤上の同じ色の駒は最大7枚 * ライオンが最大8方向
打ち手 = 10 * 3 = 30;   // 最低でもライオンが2枚あるため
合計 = 56 + 30 = 86;
 */


namespace dobutsu {

    constexpr int MAX_MOVES = 86;

    class MoveList{
        private:
            Move moves_[MAX_MOVES];
            int  size_{};

            void add(Move m) { assert(size_ < MAX_MOVES); moves_[size_++] = m; }

        public: 
            explicit MoveList(const Position& pos);
            const Move* begin() const { return moves_; }
            const Move* end() const { return moves_ + size_; }
            int size() const { return size_; }
    };
}
