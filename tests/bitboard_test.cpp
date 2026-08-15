#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>

using namespace dobutsu;

/* コンパイル時評価 */
static_assert(sizeof(Bitboard) == 2);
static_assert(square_bb(SQ_2C).lsb() == SQ_2C);
static_assert((~square_bb(SQ_3A)).popcount() == 11);

constexpr int count_all(){
    Bitboard bb;
    for(int i = 0; i < SQ_NB; i++){
        bb.set(Square(i));
    }
    int n = 0;
    while(bb){
        bb.pop_lsb(); n++;
    }
    return n;
}
static_assert(count_all() == SQ_NB, "どこかで constexpr がぬけてる");

/* 単一マスと基本の性質 */
TEST(Bitboard, FromAllSquares){
    for(int s = 0; s < SQ_NB; s++){
        Bitboard b = square_bb(Square(s));
        EXPECT_EQ(b.popcount(), 1) << "(popcount) s = " << s;  // 要素数1
        EXPECT_EQ(b.lsb(), Square(s)) << "(lsb) s = " << s;
        EXPECT_TRUE(b.test(Square(s))) << "(test) s = " << s;
    }
}

/* set reset test */
TEST(Bitboard, SetResetTest){
    Bitboard b;
    EXPECT_EQ(b.popcount(), 0);
    EXPECT_FALSE(b.test(SQ_2A));
    // 追加
    b.set(SQ_2A); 
    EXPECT_EQ(b.popcount(), 1);
    EXPECT_TRUE(b.test(SQ_2A));
    // 同じところに追加
    b.set(SQ_2A);
    EXPECT_EQ(b.popcount(), 1);

    // 削除
    b.reset(SQ_2A);
    EXPECT_EQ(b.popcount(), 0);
    EXPECT_FALSE(b.test(SQ_2A));
}
TEST(Bitboard, SampleBoardSetting){
    // 適当な盤面で実験
    Bitboard b(0xFFFF);
    EXPECT_EQ(b.popcount(), 12);
    b.reset(SQ_2D);
    EXPECT_EQ(b.popcount(), 11);
    EXPECT_FALSE(b.test(SQ_2D));
    EXPECT_TRUE(b.test(SQ_1D));
    EXPECT_TRUE(b.test(SQ_3D));
    EXPECT_TRUE(b.test(SQ_2C));
    EXPECT_TRUE(b.test(SQ_3C));
    EXPECT_TRUE(b.test(SQ_1C));
    b.reset(SQ_2A);
    b.reset(SQ_2A);
    EXPECT_EQ(b.popcount(), 10);
}

/* 上位4bit が常に0 */
TEST(Bitboard, MasksUpperBits){
    Bitboard b(0x0171);
    EXPECT_EQ(b.popcount(), 5);
    b = ~b;
    EXPECT_EQ(b.popcount(), 7);
    EXPECT_EQ(~~b, b);
}

/* 集合演算と走査 */
TEST(Bitboard, SetOperations){
    Bitboard b0(0x0171);
    Bitboard b1(0xF053);
    Bitboard b;

    b = b0 | b1;
    EXPECT_EQ(b.popcount(), 6);
    EXPECT_TRUE(b.test(SQ_3A));
    EXPECT_TRUE(b.test(SQ_2A));
    EXPECT_TRUE(b.test(SQ_2B));
    EXPECT_TRUE(b.test(SQ_1B));
    EXPECT_TRUE(b.test(SQ_3C));
    EXPECT_TRUE(b.test(SQ_1C));
    
    b = b0 & b1;
    EXPECT_EQ(b.popcount(), 3);
    EXPECT_TRUE(b.test(SQ_3A));
    EXPECT_TRUE(b.test(SQ_2B));
    EXPECT_TRUE(b.test(SQ_3C));

    b = b0 ^ b1;    // 0001 0010 0010
    EXPECT_EQ(b.popcount(), 3);
    EXPECT_TRUE(b.test(SQ_2A));
    EXPECT_TRUE(b.test(SQ_1B));
    EXPECT_TRUE(b.test(SQ_1C));

    b = b0 & ~b1;   // 0001 0010 0000
    EXPECT_EQ(b.popcount(), 2);
    EXPECT_TRUE(b.test(SQ_1B));
    EXPECT_TRUE(b.test(SQ_1C));
    // 追加で b1 が壊れないことを確認
    EXPECT_EQ(b1.popcount(), 4);
    EXPECT_TRUE(b1.test(SQ_3A));
    EXPECT_TRUE(b1.test(SQ_2A));
    EXPECT_TRUE(b1.test(SQ_2B));
    EXPECT_TRUE(b1.test(SQ_3C));

}
TEST(Bitboard, PopLsbVisitsAllSquaresInOrder){
    // pop_lsb の走査
    Bitboard b(0x0FFF);
    Bitboard b1(0x0FFF);
    int square_it = 0;
    while(b){
        Square s = b.pop_lsb();
        EXPECT_EQ(s, Square(square_it));
        b1.reset(Square(square_it));
        EXPECT_EQ(b, b1);
        square_it++;
    }
    EXPECT_EQ(square_it, SQ_NB);
}
