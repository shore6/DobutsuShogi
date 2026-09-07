#include <algorithm>
#include <cstdint>
#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <string>
#include <vector>

using namespace dobutsu;

namespace {
    const std::string sfen_list[] = {
        "gle/1c1/1C1/ELG b - 1",
        "+Cle/3/3/EL+c b 2G 7",
        "l2/1C1/3/2L b 2G2ec 5",
        "gle/1c1/1C1/ELG w - 101",
        "gle/3/3/ELG w 2c 170",
        "gle/1c1/1+C1/ELG b - 1",
        "gl1/1c1/1C1/1LG b Ee 1",
        "+Cle/3/3/EL+c w 2G 7",
        "l2/3/3/2L b 2GC2ec 5"
    };
}

TEST(Movegen, CheckGeneratedMoveAtStartpos){
    Position pos = Position::startpos();
    MoveList ml = MoveList(pos);
    std::vector<std::string> usi;
    EXPECT_EQ(ml.size(), 4);
    for(const Move m : ml){
        EXPECT_FALSE(move_to_usi(m).empty());
        usi.push_back(move_to_usi(m));
    }
    std::sort(usi.begin(), usi.end());
    EXPECT_EQ(usi, (std::vector<std::string>{"1d1c", "2c2b", "2d1c", "2d3c"}));
}

TEST(Movegen, DoAndUndoMovesFromMoveList){
    for(const std::string& sfen : sfen_list){
        auto pos = Position::from_sfen(sfen);
        for(const Move m : MoveList(pos.value())){
            StateInfo st;
            pos->do_move(m, st);
            EXPECT_TRUE(pos->is_consistent()) << move_to_usi(m);
            pos->undo_move(m);
            EXPECT_EQ(pos->sfen(), sfen) << move_to_usi(m);
        }
    }
}

TEST(Movegen, DetectDuplicatesOfGeneratedMoves){
    for(const std::string& sfen : sfen_list){
        auto pos = Position::from_sfen(sfen);
        MoveList ml = MoveList(pos.value());
        std::vector<Move> ms(ml.begin(), ml.end());
        std::sort(ms.begin(), ms.end());
        auto it = std::adjacent_find(ms.begin(), ms.end());
        EXPECT_TRUE(it == ms.end());
    }
}

TEST(Movegen, CheckForcePromotionOfChick){
    const std::string sfen_list_promotion[] = {
        "l2/1CC/3/ELG b eg 3",
        "l2/1cc/3/ELG w EG 10",
        "l2/3/1CC/ELG b eg 13",
        "l2/3/1cc/LEG w eg 22",
        "gle/1c1/1C1/ELG b - 1",
        "+Cle/3/3/EL+c b 2G 7",
        "l2/1C1/3/2L b 2G2ec 5",
        "gle/1c1/1C1/ELG w - 101",
        "gle/3/3/ELG w 2c 170",
        "gle/1c1/1+C1/ELG b - 1",
        "gl1/1c1/1C1/1LG b Ee 1",
        "+Cle/3/3/EL+c w 2G 7",
        "l2/3/3/2L b 2GC2ec 5"
    };
    for(const std::string& sfen : sfen_list_promotion){
        auto pos = Position::from_sfen(sfen);
        std::vector<std::uint16_t> moves_list;
        for(const Move m : MoveList(pos.value())){
            Color us = pos->side_to_move();
            bool to_innermost = rank_of(to_sq(m)) == (us == BLACK ? RANK_A : RANK_D);
            bool is_move = !is_drop(m);
            bool is_chick = is_move && type_of(pos->piece_on(from_sq(m))) == CHICK;
            EXPECT_EQ(is_promotion(m), to_innermost && is_move && is_chick);

            // 同じ移動で is_promotion が異なる Move を検出
            if(!is_promotion(m)){
                moves_list.push_back(std::uint16_t(m));
            }else{
                for(std::uint16_t mo : moves_list){
                    bool same_to = to_sq(Move(mo)) == to_sq(m);
                    bool same_from = from_sq(Move(mo)) == from_sq(m);
                    EXPECT_FALSE(same_to && same_from);
                }
            }
        }
    }
}
