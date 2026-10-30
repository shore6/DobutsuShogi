#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/evaluate.hpp>
#include <dobutsu/position.hpp>

using namespace dobutsu;

TEST(Evaluate, ScoreIsZeroAtStartpos){
    Position pos = Position::startpos();
    EXPECT_EQ(evaluate(pos), 0);
}

TEST(Evaluate, ScoreSignIsReversedWhenSameBoardSameHandsButDifferentTurn){
    auto pos1 = Position::from_sfen("gle/1C1/3/ELG b c 1");
    auto pos2 = Position::from_sfen("gle/1C1/3/ELG w c 1");
    EXPECT_EQ(evaluate(pos1.value()), -evaluate(pos2.value()));
}

TEST(Evaluate, ScoreIsSameForReversedColorBoard){
    auto pos1 = Position::from_sfen("1l1/1c1/2C/1L1 b 2E2g 1");
    auto pos2 = Position::from_sfen("1l1/c2/1C1/1L1 w 2G2e 1");
    EXPECT_EQ(evaluate(pos1.value()), evaluate(pos2.value()));
}

TEST(Evaluate, CheckSignOfScore){
    auto pos1 = Position::from_sfen("gle/1C1/3/ELG b C 2");
    EXPECT_TRUE(evaluate(pos1.value()) > 0);
    auto pos2 = Position::from_sfen("gle/1C1/3/ELG w C 2");
    EXPECT_TRUE(evaluate(pos2.value()) < 0);
}

TEST(Evaluate, CheckScoreWithConstValue){
    auto pos1 = Position::from_sfen("g+Ce/l2/3/ELc b g 13");
    EXPECT_EQ(evaluate(pos1.value()), -400);
    auto pos2 = Position::from_sfen("gl1/3/1C1/ELG b EC 1");
    EXPECT_EQ(evaluate(pos2.value()), 800);
}
