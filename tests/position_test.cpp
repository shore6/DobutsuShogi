#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>

using namespace dobutsu;

TEST(Position, PositionToAscii){
    EXPECT_EQ(Position::startpos().to_ascii(),
        "  3   2   1\n"
        "+---+---+---+\n"
        "| g | l | e | a\n"
        "+---+---+---+\n"
        "|   | c |   | b\n"
        "+---+---+---+\n"
        "|   | C |   | c\n"
        "+---+---+---+\n"
        "| E | L | G | d\n"
        "+---+---+---+\n"
        "hand: -\n"
        "turn: b\n"
        "ply: 1\n"
    );
    EXPECT_EQ(Position::from_sfen("l2/1C1/3/2L b 2G2ec 5").value().to_ascii(),
        "  3   2   1\n"
        "+---+---+---+\n"
        "| l |   |   | a\n"
        "+---+---+---+\n"
        "|   | C |   | b\n"
        "+---+---+---+\n"
        "|   |   |   | c\n"
        "+---+---+---+\n"
        "|   |   | L | d\n"
        "+---+---+---+\n"
        "hand: G*2 e*2 c*1\n"
        "turn: b\n"
        "ply: 5\n"
    );
    EXPECT_EQ(Position::from_sfen("+Cle/3/3/EL+c w 2G 7").value().to_ascii(),
        "  3   2   1\n"
        "+---+---+---+\n"
        "| H | l | e | a\n"
        "+---+---+---+\n"
        "|   |   |   | b\n"
        "+---+---+---+\n"
        "|   |   |   | c\n"
        "+---+---+---+\n"
        "| E | L | h | d\n"
        "+---+---+---+\n"
        "hand: G*2\n"
        "turn: w\n"
        "ply: 7\n"
    );
}

TEST(Position, StartposPieceSets){
    const Position pos = Position::startpos();

    EXPECT_EQ(pos.pieces(BLACK, LION), square_bb(SQ_2D));
    EXPECT_EQ(pos.pieces(BLACK, GIRAFFE), square_bb(SQ_1D));
    EXPECT_EQ(pos.pieces(BLACK, ELEPHANT), square_bb(SQ_3D));
    EXPECT_EQ(pos.pieces(BLACK, CHICK), square_bb(SQ_2C));
    EXPECT_EQ(pos.pieces(WHITE, LION), square_bb(SQ_2A));
    EXPECT_EQ(pos.pieces(WHITE, GIRAFFE), square_bb(SQ_3A));
    EXPECT_EQ(pos.pieces(WHITE, ELEPHANT), square_bb(SQ_1A));
    EXPECT_EQ(pos.pieces(WHITE, CHICK), square_bb(SQ_2B));

    EXPECT_EQ(pos.pieces(BLACK).popcount(), 4);
    EXPECT_EQ(pos.pieces(WHITE).popcount(), 4);
    EXPECT_EQ(pos.pieces().popcount(), 8);
    EXPECT_EQ(pos.pieces(HEN), Bitboard());
    
    EXPECT_EQ(pos.lion_square(BLACK), SQ_2D);
    EXPECT_EQ(pos.lion_square(WHITE), SQ_2A);

    EXPECT_EQ(pos.piece_on(SQ_3B), NO_PIECE);
    EXPECT_EQ(pos.hand_count(BLACK, CHICK), 0);
}


TEST(Position, GenSFENfromStartPos){
    EXPECT_EQ(Position::startpos().sfen(), "gle/1c1/1C1/ELG b - 1");
}

TEST(Position, PieceSetsOfPositionFromSFEN){
    auto p = Position::from_sfen("+Cle/3/3/EL+c b 2G 7");
    ASSERT_TRUE(p.has_value());

    EXPECT_EQ(p->pieces(BLACK, LION), square_bb(SQ_2D));
    EXPECT_EQ(p->pieces(BLACK, ELEPHANT), square_bb(SQ_3D));
    EXPECT_EQ(p->pieces(BLACK, HEN), square_bb(SQ_3A));
    EXPECT_EQ(p->pieces(WHITE, LION), square_bb(SQ_2A));
    EXPECT_EQ(p->pieces(WHITE, ELEPHANT), square_bb(SQ_1A));
    EXPECT_EQ(p->pieces(WHITE, HEN), square_bb(SQ_1D));

    EXPECT_EQ(p->pieces(BLACK).popcount(), 3);
    EXPECT_EQ(p->pieces(WHITE).popcount(), 3);
    EXPECT_EQ(p->pieces().popcount(), 6);
    
    EXPECT_EQ(p->pieces(HEN), square_bb(SQ_3A) | square_bb(SQ_1D));
    EXPECT_EQ(p->pieces(CHICK), Bitboard());
}

TEST(Position, GenPositionFromSFEN){
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
    for(const std::string& s : sfen_list){
        auto p = Position::from_sfen(s);
        ASSERT_TRUE(p.has_value()) << s;
        EXPECT_TRUE(p->is_consistent()) << s;
        EXPECT_EQ(p->sfen(), s) << s;
    }
}

TEST(Position, GenPositionFromIncorrectSFEN){
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1L1 b GEC 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle//1C1/ELG w c 1").has_value());
    EXPECT_FALSE(Position::from_sfen("/1l1/1C1/ELG w ge 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c2/1C1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/3/1C1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/5/1C1/ELG b C 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle33ELG b Cc 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b -").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b - 1 ABC").has_value());
    EXPECT_FALSE(Position::from_sfen("xle/1c1/1C1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/3/1CC1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("g1e/1c1/1CL/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG w C 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG w 3E 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/E1G w L 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b H 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b h 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG c - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG w - 0").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b - -1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1Cc/ELG w - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1+H1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1cc/1C1/ELG b - 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b - 700000").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELG b - 1a2b").has_value());
    EXPECT_FALSE(Position::from_sfen("glG/1c1/1C1/EL1 b 2G2G 1").has_value());
    EXPECT_FALSE(Position::from_sfen("gle/1c1/1C1/ELGG b - 1").has_value());    // 一応
}


TEST(Position, CheckValuesOfPositionFromSFEN){
    auto p = Position::from_sfen("l2/1C1/3/2L b 2G2ec 5");
    ASSERT_TRUE(p.has_value());
    EXPECT_EQ(p->sfen(), "l2/1C1/3/2L b 2G2ec 5");

    EXPECT_EQ(p->piece_on(SQ_3A), W_LION);
    EXPECT_EQ(p->piece_on(SQ_2B), B_CHICK);
    EXPECT_EQ(p->piece_on(SQ_1D), B_LION);

    EXPECT_EQ(p->piece_on(SQ_2A), NO_PIECE); EXPECT_EQ(p->piece_on(SQ_1A), NO_PIECE);
    EXPECT_EQ(p->piece_on(SQ_3B), NO_PIECE); EXPECT_EQ(p->piece_on(SQ_1B), NO_PIECE);
    EXPECT_EQ(p->piece_on(SQ_3C), NO_PIECE); EXPECT_EQ(p->piece_on(SQ_2C), NO_PIECE); EXPECT_EQ(p->piece_on(SQ_1C), NO_PIECE);
    EXPECT_EQ(p->piece_on(SQ_3D), NO_PIECE); EXPECT_EQ(p->piece_on(SQ_2D), NO_PIECE);

    EXPECT_EQ(p->hand_count(BLACK, GIRAFFE), 2);
    EXPECT_EQ(p->hand_count(WHITE, ELEPHANT), 2);
    EXPECT_EQ(p->hand_count(WHITE, CHICK), 1);

    EXPECT_EQ(p->hand_count(BLACK, ELEPHANT), 0);
    EXPECT_EQ(p->hand_count(BLACK, CHICK), 0);
    EXPECT_EQ(p->hand_count(WHITE, GIRAFFE), 0);

    EXPECT_EQ(p->side_to_move(), BLACK);
    EXPECT_EQ(p->ply(), 5);
}

