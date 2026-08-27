#include <gtest/gtest.h>
#include <string>
#include <dobutsu/types.hpp>

using namespace dobutsu;

TEST(Moves, GenAndCompareAllMoves){
    for(int from = 0; from < SQ_NB; from++){
        for(int to = 0; to < SQ_NB; to++){
            for(bool promote : {false, true}){
                if(from == to) continue;    // 未定義動作?
                Move m = make_move(Square(from), Square(to), promote);
                EXPECT_FALSE(m == Move::NONE) << from << ":" << to << ":" << promote << "none";
                EXPECT_EQ(from_sq(m), Square(from)) << from << ":" << to << ":" << promote << "from";
                EXPECT_EQ(to_sq(m), Square(to)) << from << ":" << to << ":" << promote << "to";
                EXPECT_EQ(dropped_piece(m), NO_PIECE_TYPE) << from << ":" << to << ":" << promote << "pt";
                EXPECT_FALSE(is_drop(m)) << from << ":" << to << ":" << promote << "drop";
                EXPECT_EQ(is_promotion(m), promote) << from << ":" << to << ":" << promote << "pr";
            
                std::string usi = move_to_usi(m); 
                EXPECT_EQ(move_from_usi(usi), m) << from << ":" << to << ":" << promote << "usi";
            }
        }
    }
}

TEST(Moves, GenAndCompareAllDrops){
    for(int to = 0; to < SQ_NB; to++){
        for(PieceType pt : {GIRAFFE, ELEPHANT, CHICK}){
            Move m = make_drop(pt, Square(to));
            EXPECT_FALSE(m == Move::NONE) << to << ":" << pt << "none";
            EXPECT_EQ(to_sq(m), Square(to)) << to << ":" << pt << "to";
            EXPECT_EQ(from_sq(m), SQ_NONE) << to << ":" << pt << "from";
            EXPECT_EQ(dropped_piece(m), pt) << to << ":" << pt << "pt";
            EXPECT_TRUE(is_drop(m)) << to << ":" << pt << "drop";
            EXPECT_FALSE(is_promotion(m)) << to << ":" << pt << "pr";
            
            std::string usi = move_to_usi(m);
            EXPECT_EQ(move_from_usi(usi), m) << to << ":" << pt << "usi";
        }
    }
}

TEST(Moves, GenMoveFromSampleUSI){
    const std::string usi_list[] = {
        "2b1c", "3c3d+", "1a2b", "2c3d+", "1b2b", "2b2c",
        "G*3a", "G*1c", "E*2d", "E*3c", "C*1b", "C*2a"
    };
    for(std::string usi : usi_list){
        EXPECT_EQ(move_to_usi(move_from_usi(usi)), usi) << usi;
    }
}

TEST(Moves, CheckValuesOfMoveFromUSI){
    Move m1 = move_from_usi("2b2a+");
    Move m2 = move_from_usi("G*3c");

    EXPECT_EQ(to_sq(m1), SQ_2A);
    EXPECT_EQ(from_sq(m1), SQ_2B);
    EXPECT_EQ(dropped_piece(m1), NO_PIECE_TYPE);
    EXPECT_EQ(is_drop(m1), false);
    EXPECT_EQ(is_promotion(m1), true);
    EXPECT_EQ(move_to_usi(m1), "2b2a+");

    EXPECT_EQ(to_sq(m2), SQ_3C);
    EXPECT_EQ(from_sq(m2), SQ_NONE);
    EXPECT_EQ(dropped_piece(m2), GIRAFFE);
    EXPECT_EQ(is_drop(m2), true);
    EXPECT_EQ(is_promotion(m2), false);
    EXPECT_EQ(move_to_usi(m2), "G*3c");
}

TEST(Moves, GenMovesFromIncorrectUSI){
    const std::string usi_list[] = {
        "2b", "1bb", "3b3c1a", "2b2c++", "1a11",
        "3e2b", "1a5c", "5a1c", "3a1g", "1111",
        "2b2a-", "L*2b", "E+1a", "2b2c*", "C*5d"
    };  // '+' を省略する、最終段でないのに'+'を付ける、などは Move の責任ではない。検出しないしできない
    for(std::string usi : usi_list){
        EXPECT_EQ(move_from_usi(usi), Move::NONE) << usi;
    }
}
