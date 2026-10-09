#include <gtest/gtest.h>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <string>
#include <map>
#include <utility>
#include <vector>

using namespace dobutsu;

namespace {
    Bitboard attacked_by(const Position& pos, Color c){
        Bitboard bb;
        Bitboard bb_attacker = pos.pieces(c);
        while(bb_attacker){
            Square s = bb_attacker.pop_lsb();
            bb |= attacks_from(pos.piece_on(s), s);
        }
        return bb;
    }
}

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

/* 以下 Move を含む */

TEST(Position, DoingSampleMoves){
    Position pos = Position::startpos();
    StateInfo st[3];
    Move m1 = make_move(SQ_2C, SQ_2B);
    pos.do_move(m1, st[0]);
    EXPECT_EQ(pos.sfen(), "gle/1C1/3/ELG w C 2");
    EXPECT_TRUE(pos.is_consistent());
    pos.undo_move(m1);
    EXPECT_EQ(pos.sfen(), Position::startpos().sfen());
    EXPECT_TRUE(pos.is_consistent());

    auto pos1 = Position::from_sfen("gle/1C1/3/ELG w C 2");
    Move m2 = make_move(SQ_1A, SQ_2B);
    pos1->do_move(m2, st[1]);
    EXPECT_EQ(pos1->sfen(), "gl1/1e1/3/ELG b Cc 3");
    EXPECT_TRUE(pos1->is_consistent());
    pos1->undo_move(m2);
    EXPECT_EQ(pos1->sfen(), "gle/1C1/3/ELG w C 2");
    EXPECT_TRUE(pos1->is_consistent());

    auto pos2 = Position::from_sfen("gle/1+c1/1C1/ELG b - 21");
    pos2->do_move(make_move(SQ_2C, SQ_2B), st[2]);
    EXPECT_EQ(pos2->sfen(), "gle/1C1/3/ELG w C 22");
    EXPECT_TRUE(pos2->is_consistent());
    pos2->undo_move(make_move(SQ_2C, SQ_2B));
    EXPECT_EQ(pos2->sfen(), "gle/1+c1/1C1/ELG b - 21");
    EXPECT_TRUE(pos2->is_consistent());
}

TEST(Position, DoingSampleMovesWithPromotion){
    auto pos = Position::from_sfen("g1e/lC1/2c/ELG b - 11");
    StateInfo st[3];
    pos->do_move(make_move(SQ_2B, SQ_2A, true), st[0]);
    EXPECT_EQ(pos->sfen(), "g+Ce/l2/2c/ELG w - 12");
    EXPECT_TRUE(pos->is_consistent());
    pos->undo_move(make_move(SQ_2B, SQ_2A, true));
    EXPECT_EQ(pos->sfen(), "g1e/lC1/2c/ELG b - 11");
    EXPECT_TRUE(pos->is_consistent());

    auto pos1 = Position::from_sfen("g+Ce/l2/2c/ELG w - 12");
    pos1->do_move(make_move(SQ_1C, SQ_1D, true), st[1]);
    EXPECT_EQ(pos1->sfen(), "g+Ce/l2/3/EL+c b g 13");
    EXPECT_TRUE(pos1->is_consistent());
    pos1->undo_move(make_move(SQ_1C, SQ_1D, true));
    EXPECT_EQ(pos1->sfen(), "g+Ce/l2/2c/ELG w - 12");
    EXPECT_TRUE(pos1->is_consistent());

    auto pos2 = Position::from_sfen("gel/1C1/3/1LG b Ec 21");
    pos2->do_move(make_move(SQ_2B, SQ_2A, true), st[2]);
    EXPECT_EQ(pos2->sfen(), "g+Cl/3/3/1LG w 2Ec 22");
    EXPECT_TRUE(pos2->is_consistent());
    pos2->undo_move(make_move(SQ_2B, SQ_2A, true));
    EXPECT_EQ(pos2->sfen(), "gel/1C1/3/1LG b Ec 21");
    EXPECT_TRUE(pos2->is_consistent());
}

TEST(Position, DoingSampleMovesWithDrops){
    auto pos = Position::from_sfen("gle/3/3/ELG w Cc 12");
    StateInfo st[2];
    pos->do_move(make_drop(CHICK, SQ_2C), st[0]);
    EXPECT_EQ(pos->sfen(), "gle/3/1c1/ELG b C 13");
    EXPECT_TRUE(pos->is_consistent());
    pos->undo_move(make_drop(CHICK, SQ_2C));
    EXPECT_EQ(pos->sfen(), "gle/3/3/ELG w Cc 12");
    EXPECT_TRUE(pos->is_consistent());

    auto pos1 = Position::from_sfen("gle/3/1c1/ELG b C 13");
    pos1->do_move(make_drop(CHICK, SQ_3B), st[1]);
    EXPECT_EQ(pos1->sfen(), "gle/C2/1c1/ELG w - 14");
    EXPECT_TRUE(pos1->is_consistent());
    pos1->undo_move(make_drop(CHICK, SQ_3B));
    EXPECT_EQ(pos1->sfen(), "gle/3/1c1/ELG b C 13");
    EXPECT_TRUE(pos1->is_consistent());
}

TEST(Position, DoingMovesWithCaptureLION){
    auto pos = Position::from_sfen("gle/1G1/3/ELC b C 33");
    StateInfo st;
    pos->do_move(make_move(SQ_2B, SQ_2A), st);
    EXPECT_EQ(pos->sfen(), "gGe/3/3/ELC w C 34");
    EXPECT_TRUE(pos->is_consistent());
    EXPECT_FALSE(pos->is_legal_position());
    pos->undo_move(make_move(SQ_2B, SQ_2A));
    EXPECT_EQ(pos->sfen(), "gle/1G1/3/ELC b C 33");
    EXPECT_TRUE(pos->is_consistent());
    EXPECT_TRUE(pos->is_legal_position());
}

TEST(Position, DoAndUndoSeriesOfMoves){
    Position pos = Position::startpos();
    StateInfo st[3];
    std::string before = pos.sfen();
    Move m0 = make_move(SQ_2C, SQ_2B);
    Move m1 = make_move(SQ_2A, SQ_2B);
    Move m2 = make_move(SQ_3D, SQ_2C);
    pos.do_move(m0, st[0]); pos.do_move(m1, st[1]); pos.do_move(m2, st[2]);
    EXPECT_EQ(pos.sfen(), "g1e/1l1/1E1/1LG w Cc 4");
    pos.undo_move(m2); pos.undo_move(m1); pos.undo_move(m0);
    EXPECT_EQ(pos.sfen(), before);
}

TEST(Position, CheckAccuracyOfIsAttackedAtStartpos){
    Position pos = Position::startpos();
    EXPECT_FALSE(pos.is_attacked(SQ_2A, BLACK));
    EXPECT_TRUE(pos.is_attacked(SQ_2B, BLACK));
    EXPECT_TRUE(pos.is_attacked(SQ_2C, BLACK));
    EXPECT_TRUE(pos.is_attacked(SQ_2D, BLACK));

    EXPECT_TRUE(pos.is_attacked(SQ_2A, WHITE));
    EXPECT_TRUE(pos.is_attacked(SQ_2B, WHITE));
    EXPECT_TRUE(pos.is_attacked(SQ_2C, WHITE));
    EXPECT_FALSE(pos.is_attacked(SQ_2D, WHITE));
}

TEST(Position, CheckAttackingDirectionOfChick){
    auto pos = Position::from_sfen("3/C2/c1l/2L w 2G2E 44");
    
    EXPECT_TRUE(pos->is_attacked(SQ_3A, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_3C, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_3B, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_3D, WHITE));
}

TEST(Position, CheckIsAttackedOfEmptySquare){
    auto pos = Position::from_sfen("3/1l1/1L1/3 w GECgec 66");

    EXPECT_TRUE(pos->is_attacked(SQ_3A, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_2A, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_1A, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_3B, WHITE));
    EXPECT_FALSE(pos->is_attacked(SQ_2B, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_1B, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_3C, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_2C, WHITE));
    EXPECT_TRUE(pos->is_attacked(SQ_1C, WHITE));
    EXPECT_FALSE(pos->is_attacked(SQ_3D, WHITE));
    EXPECT_FALSE(pos->is_attacked(SQ_2D, WHITE));
    EXPECT_FALSE(pos->is_attacked(SQ_1D, WHITE));

    EXPECT_TRUE(pos->is_attacked(SQ_3B, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_2B, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_1B, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_3C, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_2C, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_1C, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_3D, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_2D, BLACK));
    EXPECT_TRUE(pos->is_attacked(SQ_1D, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_3A, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_2A, BLACK));
    EXPECT_FALSE(pos->is_attacked(SQ_1A, BLACK));
}


TEST(Position, CheckAndCompareIsAttackedWithFromScratch){
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
    for(const std::string& sfen : sfen_list){
        auto pos = Position::from_sfen(sfen);
        for(Color c : {BLACK, WHITE}){
            Bitboard bb = attacked_by(*pos, c);
            for(int s = 0; s < SQ_NB; s++){
                EXPECT_EQ(pos->is_attacked(Square(s), c), bb.test(Square(s)));
            }
        }
    }
}

