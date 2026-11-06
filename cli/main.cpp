#include <cassert>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <system_error>
#include <vector>
#include <chrono>
#include <charconv>
#include <utility>
#include <optional>
#include <algorithm>
#include <random>
#include <deque>
#include <cstdlib>
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>
#include <dobutsu/search.hpp>


namespace {

    enum class PlayerKind { HUMAN, RANDOM, SEARCH };

    std::string kind_to_char(const PlayerKind pk){
        switch(pk){
            case PlayerKind::HUMAN : return "human";
            case PlayerKind::RANDOM : return "random";
            case PlayerKind::SEARCH : return "search";
        }
        return "";
    }

    std::string color_to_name(dobutsu::Color c){
        return (c == dobutsu::BLACK ? "Black" : "White");
    }

    void print_result(dobutsu::GameResult result, dobutsu::Color side_to_move){
        if(result == dobutsu::GameResult::CATCH){
            std::cout << color_to_name(side_to_move) << " animal catch " << color_to_name(~side_to_move) << " lion.\n" << color_to_name(side_to_move) << " win.\n";
        }else if(result == dobutsu::GameResult::TRY){
            std::cout << color_to_name(~side_to_move) << " lion try " << color_to_name(side_to_move) << ".\n" << color_to_name(~side_to_move) << " win.\n";
        }else{
            std::cout << "Repetition.\nDraw.\n";
        }
    }

    dobutsu::Move pick_random(const dobutsu::Position& pos, std::mt19937& rng){
        dobutsu::MoveList ml(pos);
        assert(ml.size() > 0);
        std::uniform_int_distribution<int> dist(0, ml.size() - 1);
        int i = dist(rng);
        return *(ml.begin() + i);
    }


    /* 引数パーサー */
    std::optional<dobutsu::Position> position_from_args(const std::vector<std::string>& args, int first){
        // first 番目移行の引数から局面をつくる。引数がなければ startpos。無効な場合は std::nullopt で返す
        
        std::optional<dobutsu::Position> pos;
        std::string sfen = "";
        if(static_cast<int>(args.size()) - first > 0){
            if(args[first] == "startpos"){
                pos = dobutsu::Position::startpos();
            }else{
                for(int i = first; i < static_cast<int>(args.size()); i++){
                    sfen += args[i];
                    sfen += " ";
                }
                pos = dobutsu::Position::from_sfen(sfen);
                if(!pos.has_value()){
                    return std::nullopt;
                }
            }
        }else{
            pos = dobutsu::Position::startpos();
        }
        return pos;
    }
    
    std::optional<int> parse_int(const std::string& s){
        int value;
        auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
        if(ec != std::errc{} || ptr != s.data() + s.size()){
            return std::nullopt;
        }
        return std::make_optional(value);
    }

    std::string score_to_string(int score){
        if(std::abs(score) > dobutsu::VALUE_MATE - dobutsu::MAX_PLY){
            int n = std::abs(dobutsu::VALUE_MATE - std::abs(score));
            return std::to_string(score) + " (" + (score > 0 ? "win " : "lose ") + "in " + std::to_string(n) + ")";
        }else return std::to_string(score);
    }

}
void help(){
    std::cout << "dobutsu Version 0.4.0\n";
    std::cout << "Usage: dobutsu [options]\n";
    std::cout << "Options:\n";
    std::cout << "  help                    Display this information.\n";
    std::cout << "  show [sfen]             Dump board generated from SFEN.\n";
    std::cout << "                          If you enter \"startpos\" or without sfen, it will output board of Starting position according to general rules.\n";
    std::cout << "  perft <depth> [sfen]    Run perft at the specified depth.\n";
    std::cout << "                          If SFEN is specified, it runs from that board position; if not, it runs from the initial board position.\n";
    std::cout << "  play [--black human|random|search] [--white human|random|search] [--depth N] [--seed N] [sfen]\n";
    std::cout << "                          Play the game.\n";
    std::cout << "                          If you don't specify, it will set human, human, 6, random seed and startpos.\n";
    std::cout << "  go <depth> [sfen]       Search and dump bestmove, score, nodes and elapsed time.\n";

    std::cout << std::endl;
}

int show(std::vector<std::string> args){

    std::optional<dobutsu::Position> pos = position_from_args(args, 2);
    if(!pos.has_value()){
        std::cerr << "Unrecognized SFEN." << std::endl;
        return 1;
    }

    std::cout << pos->to_ascii();

    return 0;
}

int perft(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "perft requires depth." << std::endl;
        return 1;
    }

    std::optional<int> d = parse_int(args[2]);
    if(!d.has_value()){
        std::cerr << "Unrecognized depth input." << std::endl;
        return 1;
    }
    int depth = d.value();
    if(depth < 1){
        std::cerr << "Invalid depth value." << std::endl;
        return 1;
    }
    if(depth > 10){
        std::cerr << "warning : Too large value of depth. Processing may take a long time\n";
    }

    std::optional<dobutsu::Position> pos = position_from_args(args, 3);
    if(!pos.has_value()){
        std::cerr << "Unrecognized SFEN." << std::endl;
        return 1;
    }

    if(pos->is_terminal()){
        std::cout << "nodes: 1" << std::endl;
        return 0;
    }

    dobutsu::MoveList ml(pos.value());
    std::vector<std::pair<dobutsu::Move, std::uint64_t>> moves;
    for(dobutsu::Move m : ml){
        moves.push_back(std::pair<dobutsu::Move, std::uint64_t>(m, 0));
    }
    const auto start = std::chrono::steady_clock::now();
    for(auto& [m, nodes] : moves){
        dobutsu::StateInfo st;
        pos->do_move(m, st);
        nodes = dobutsu::perft(pos.value(), depth-1);
        pos->undo_move(m);
    }
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    
    std::uint64_t total_nodes = 0;
    for(const auto& [m, nodes] : moves){
        std::cout << dobutsu::move_to_usi(m) << ": " << nodes << "\n";
        total_nodes += nodes;
    }
    std::cout << "nodes: " << total_nodes << "\n";
    std::cout << "elapsed time: " << ms << "ms\n";
    if(ms == 0){
        std::cout << "NPS: -\n";
    }else{
        std::cout << "NPS: " << std::fixed << std::setprecision(1) << (total_nodes / (ms / 1000.0)) << "\n";
    }

    return 0;
}

int play(int argc, std::vector<std::string> args){

    /* 引数処理 */
    PlayerKind black = PlayerKind::HUMAN, white = PlayerKind::HUMAN;
    std::random_device rd;
    std::uint32_t seed = rd();
    std::optional<dobutsu::Position> pos;
    std::string sfen;
    int depth = 6;

    int i = 2;
    while(i < argc){
        const std::string& a = args[i++];
        if((a == "--black" || a == "--white" || a == "--depth" || a == "--seed") && i < argc){
            const std::string& v = args[i++];
            if(a == "--black"){ 
                if(v == "human") black = PlayerKind::HUMAN;
                else if(v == "random") black = PlayerKind::RANDOM; 
                else if(v == "search") black = PlayerKind::SEARCH;
                else{ std::cerr << "Invalid value. (--black)" << std::endl; return 1; }
            }
            else if(a == "--white"){
                if(v == "human") white = PlayerKind::HUMAN;
                else if(v == "random") white = PlayerKind::RANDOM;
                else if(v == "search") white = PlayerKind::SEARCH;
                else{ std::cerr << "Invalid value. (--white)" << std::endl; return 1; }
            }
            else if(a == "--depth"){
                auto d = parse_int(v);
                if (!d.has_value() || d.value() < 1){ std::cerr << "Invalid value. (--depth) \n"; return 1; }
                else depth = d.value();
                if(depth > 14) std::cerr << "warning : Too large value of depth. Processing may take a long time\n";
            }
            else if(a == "--seed"){
                auto s = parse_int(v);
                if(!s.has_value()){ std::cerr << "Invalid value. (--seed) \n"; return 1; }
                else seed = s.value();
            }
        }else{
            sfen += (sfen.empty() ? "" : " ") + a;   // sfen の断片を連結
        }
    }
    std::mt19937 rng(seed);

    if(sfen == "startpos" || sfen.empty()){
        pos = dobutsu::Position::startpos();
    }else{
        pos = dobutsu::Position::from_sfen(sfen);
        if(!pos.has_value()){
            std::cerr << "Invalid option or SFEN." << std::endl; return 1;
        }
    }

    /* 情報表示 */
    std::cout << "seed : " << seed << "\n";
    if(black == PlayerKind::SEARCH || white == PlayerKind::SEARCH) std::cout << "depth : " << depth << "\n";
    std::cout << "Black : " << kind_to_char(black) << ", White : " << kind_to_char(white) << "\n";

    /* 対局ループ */
    std::deque<dobutsu::StateInfo> states;
    std::vector<dobutsu::Move> moves;
    dobutsu::Searcher searcher{};
    while(1){
        std::cout << pos->to_ascii();
        // 終局判定と表示
        dobutsu::GameResult result = pos->result();
        if(result != dobutsu::GameResult::ONGOING){
            print_result(result, pos->side_to_move());
            return 0;
        }
        
        /* Move の入力 */
        dobutsu::Color sideToMove = pos->side_to_move();
        auto is_human = [&](dobutsu::Color c){ return c == dobutsu::BLACK ? (black == PlayerKind::HUMAN) : (white == PlayerKind::HUMAN); };
        auto is_random = [&](dobutsu::Color c){ return c == dobutsu::BLACK ? (black == PlayerKind::RANDOM) : (white == PlayerKind::RANDOM); };
        if(is_human(sideToMove)){
            while(1){
                std::string line; 
                std::cout << ">> ";
                if(!std::getline(std::cin, line)){ std::cerr << "Interrupt.\n"; return 1; }
                if(line == "moves" || line == "undo" || line == "resign"){
                    if(line == "moves"){
                        std::cout << "Valid Moves : ";
                        dobutsu::MoveList ml(pos.value());
                        for(dobutsu::Move m : ml){
                            std::cout << dobutsu::move_to_usi(m) << " ";
                        }
                        std::cout << std::endl;
                        continue;
                    }else if(line == "resign"){
                        std::cout << (sideToMove == dobutsu::BLACK ? "Black" : "White") << " resign.\n" << (sideToMove == dobutsu::BLACK ? "White" : "Black") << " win." << std::endl;
                        return 0;
                    }else{
                        if(moves.empty()){
                            std::cout << "No move to undo.\n";
                            continue;
                        }
                        do{
                            pos->undo_move(moves.back());
                            moves.pop_back();
                            states.pop_back();
                            sideToMove = pos->side_to_move();
                        }while(moves.size() > 0 && !is_human(sideToMove));
                        break;
                    }
                }else{
                    dobutsu::Move m = dobutsu::move_from_usi(line);
                    if(m == dobutsu::Move::NONE){ std::cerr << "Unrecognized Move.\n"; continue; }
                    dobutsu::MoveList ml(pos.value());
                    auto it = std::find(ml.begin(), ml.end(), m);
                    if(it == ml.end()){ std::cerr << "Invalid Move.\n"; continue; }
                    states.emplace_back();
                    pos->do_move(m, states.back());
                    moves.push_back(m);
                    break;
                }
            }
        }else if(is_random(sideToMove)){
            dobutsu::Move m = pick_random(pos.value(), rng);
            states.emplace_back();
            pos->do_move(m, states.back());
            moves.push_back(m);
            std::cout << "> " << dobutsu::move_to_usi(m) << "\n";
        }else{
            dobutsu::SearchResult result = searcher.search(pos.value(), dobutsu::SearchLimits{depth});
            std::cout << "score: " << score_to_string(result.score) << "\n";
            states.emplace_back();
            pos->do_move(result.bestMove, states.back());
            moves.push_back(result.bestMove);
            std::cout << "> " << dobutsu::move_to_usi(result.bestMove) << "\n";
        }
        std::cout << std::endl;
    }
}

int go(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "go requires depth." << std::endl;
        return 1;
    }

    std::optional<int> d = parse_int(args[2]);
    if(!d.has_value()){
        std::cerr << "Unrecognized depth input." << std::endl;
        return 1;
    }
    int depth = d.value();
    if(depth < 1){
        std::cerr << "Invalid depth value." << std::endl;
        return 1;
    }
    if(depth > 14){
        std::cerr << "warning : Too large value of depth. Processing may take a long time\n";
    }

    std::optional<dobutsu::Position> pos = position_from_args(args, 3);
    if(!pos.has_value()){
        std::cerr << "Unrecognized SFEN." << std::endl;
        return 1;
    }

    if(pos->is_terminal()){
        std::cout << "bestmove none" << std::endl;
        return 0;
    }
    dobutsu::Searcher searcher{};
    const auto start = std::chrono::steady_clock::now();
    dobutsu::SearchResult result = searcher.search(pos.value(), dobutsu::SearchLimits{depth});
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    std::cout << "bestmove " << dobutsu::move_to_usi(result.bestMove) << "\n";
    std::cout << "score " << score_to_string(result.score) << "\n";
    std::cout << "nodes " << result.nodes << "\n";
    std::cout << "time " << ms << "ms\n";
    return 0;
}


int main(int argc, char* argv[]){
    std::vector<std::string> args(argv, argv + argc);

    // 引数をサブコマンドとして処理
    if(argc < 2 || args[1] == "help"){
        help();
        return 0;
    }


    if(args[1] == "show"){
        return show(args);
    }else if(args[1] == "perft"){
        return perft(argc, args);
    }else if(args[1] == "play"){
        return play(argc, args);
    }else if(args[1] == "go"){
        return go(argc, args);
    }else{
        std::cerr << "Unrecognized command \"" << args[1] << "\". Use \"help\"." << std::endl;
        return 1;
    }


    return 0;
}
