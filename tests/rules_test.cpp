#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <algorithm>

using namespace dobutsu;


/* 千日手判定 */

TEST(Rules, CheckThatRepetitionCountIsThree){
    Position pos = Position::startpos();
    StateInfo st[16];
    EXPECT_EQ(pos.result(), GameResult::ONGOING);

    pos.do_move(make_move(SQ_1D, SQ_1C), st[0]);
    pos.do_move(make_move(SQ_3A, SQ_3B), st[1]);
    pos.do_move(make_move(SQ_1C, SQ_1D), st[2]);
    pos.do_move(make_move(SQ_3B, SQ_3A), st[3]);
    EXPECT_EQ(pos.result(), GameResult::ONGOING);

    pos.do_move(make_move(SQ_1D, SQ_1C), st[4]);
    pos.do_move(make_move(SQ_3A, SQ_3B), st[5]);
    pos.do_move(make_move(SQ_1C, SQ_1D), st[6]);
    pos.do_move(make_move(SQ_3B, SQ_3A), st[7]);
    EXPECT_EQ(pos.result(), GameResult::REPETITION);
    
}

TEST(Rules, DoesNotBreakWhenCalledAtOddTimesFromRoot){
    Position pos = Position::startpos();
    StateInfo st[4];
    EXPECT_EQ(pos.result(), GameResult::ONGOING);
    pos.do_move(make_move(SQ_1D, SQ_1C), st[0]);
    EXPECT_EQ(pos.result(), GameResult::ONGOING);
    pos.do_move(make_move(SQ_3A, SQ_3B), st[1]);
    EXPECT_EQ(pos.result(), GameResult::ONGOING);
    pos.do_move(make_move(SQ_1C, SQ_1D), st[2]);
    EXPECT_EQ(pos.result(), GameResult::ONGOING);
    pos.do_move(make_move(SQ_3B, SQ_3A), st[3]);
    EXPECT_EQ(pos.result(), GameResult::ONGOING);
}

TEST(Rules, DoesNotCountTheSameBoardWithDistinctHands){
    auto pos = Position::from_sfen("1l1/3/3/1L1 b GECgec 11");
    StateInfo st[10];
    EXPECT_EQ(pos->result(), GameResult::ONGOING);
    pos->do_move(make_drop(CHICK, SQ_2B), st[0]);
    pos->do_move(make_move(SQ_2A, SQ_2B), st[1]);
    pos->do_move(make_drop(GIRAFFE, SQ_2A), st[2]);
    pos->do_move(make_move(SQ_2B, SQ_2A), st[3]);
    pos->do_move(make_move(SQ_2D, SQ_1D), st[4]);
    pos->do_move(make_drop(CHICK, SQ_2D), st[5]);
    pos->do_move(make_move(SQ_1D, SQ_2D), st[6]);
    pos->do_move(make_move(SQ_2A, SQ_2B), st[7]);
    pos->do_move(make_drop(CHICK, SQ_2A), st[8]);
    pos->do_move(make_move(SQ_2B, SQ_2A), st[9]);
    EXPECT_EQ(pos->result(), GameResult::ONGOING);
}

TEST(Rules, DoesNotCountWhenBoardAndHandAreSameWithDifferentTurn){
    auto pos = Position::from_sfen("1l1/3/3/1L1 b GECgec 11");
    StateInfo st[12];
    EXPECT_EQ(pos->result(), GameResult::ONGOING);
    pos->do_move(make_move(SQ_2D, SQ_1C), st[0]);
    pos->do_move(make_move(SQ_2A, SQ_3A), st[1]);
    pos->do_move(make_move(SQ_1C, SQ_1D), st[2]);
    pos->do_move(make_move(SQ_3A, SQ_2A), st[3]);
    pos->do_move(make_move(SQ_1D, SQ_2D), st[4]);
    EXPECT_EQ(pos->result(), GameResult::ONGOING);

    pos->do_move(make_move(SQ_2A, SQ_3A), st[5]);
    pos->do_move(make_move(SQ_2D, SQ_1D), st[6]);
    pos->do_move(make_move(SQ_3A, SQ_2A), st[7]);
    pos->do_move(make_move(SQ_1D, SQ_1C), st[8]);
    pos->do_move(make_move(SQ_2A, SQ_3A), st[9]);
    pos->do_move(make_move(SQ_1C, SQ_2D), st[10]);
    pos->do_move(make_move(SQ_3A, SQ_2A), st[11]);
    EXPECT_EQ(pos->result(), GameResult::ONGOING);

}

TEST(Rules, CheckThatCorrectGameResultIsReturned){
    {
        auto pos = Position::from_sfen("3/1l1/1L1/3 b GECgec 21");
        EXPECT_EQ(pos->result(), GameResult::CATCH);
    }{
        auto pos = Position::from_sfen("1L1/3/l2/3 w GECgec 30");
        EXPECT_EQ(pos->result(), GameResult::TRY);
    }{
        auto pos = Position::from_sfen("3/1l1/1L1/3 w GECgec 16");
        EXPECT_EQ(pos->result(), GameResult::CATCH);
    }{
        auto pos = Position::from_sfen("3/3/3/l1L b GECgec 41");
        EXPECT_EQ(pos->result(), GameResult::TRY);
    }

    /* 最奥段のライオンを取る場合 */
    {
        auto pos = Position::from_sfen("L2/l2/3/3 w GECgec 88");
        EXPECT_EQ(pos->result(), GameResult::CATCH);
    }{
        auto pos = Position::from_sfen("3/3/1L1/2l b GECgec 77");
        EXPECT_EQ(pos->result(), GameResult::CATCH);
    }
}

TEST(Rules, GenerateEffectiveMovesIgnoringCATCH){
    auto pos = Position::from_sfen("1l1/3/1e1/L2 b GECgc 59");
    ASSERT_TRUE(pos->is_attacked(pos->lion_square(BLACK), WHITE));
    MoveList ml(pos.value());
    {
        auto it = std::find(ml.begin(), ml.end(), make_drop(GIRAFFE, SQ_2D));
        EXPECT_NE(it, ml.end());
    }{
        auto it = std::find(ml.begin(), ml.end(), make_drop(CHICK, SQ_2B));
        EXPECT_NE(it, ml.end());
    }
}
