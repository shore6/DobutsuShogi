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
#include <dobutsu/types.hpp>
#include <dobutsu/bitboard.hpp>
#include <dobutsu/position.hpp>
#include <dobutsu/movegen.hpp>


namespace {
    void print_result(dobutsu::GameResult result, dobutsu::Color side_to_move){
        if(result == dobutsu::GameResult::CATCH){
            std::cout << (side_to_move == dobutsu::BLACK ? "Black" : "White") << " lion catch " << (side_to_move == dobutsu::BLACK ? "white" : "black") << " lion.\n" << (side_to_move == dobutsu::BLACK ? "Black" : "White") << " win.\n";
        }else if(result == dobutsu::GameResult::TRY){
            std::cout << (side_to_move == dobutsu::BLACK ? "White" : "Black") << " lion try " << (side_to_move == dobutsu::BLACK ? "black" : "white") << ".\n" << (side_to_move == dobutsu::BLACK ? "White" : "Black") << " win.\n";
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
}
void help(){
    std::cout << "dobutsu Version 0.3.0\n";
    std::cout << "Usage: dobutsu [options]\n";
    std::cout << "Options:\n";
    std::cout << "  help                    Display this information.\n";
    std::cout << "  show <sfen>             Dump board generated from SFEN.\n";
    std::cout << "                          (if you enter \"startpos\" instead, it will output board of Starting position according to general rules.)\n";
    std::cout << "  perft <depth> [sfen]    Run perft at the specified depth.\n";
    std::cout << "                          If SFEN is specified, it runs from that board position; if not, it runs from the initial board position.\n";
    std::cout << "  play [--black human|random] [--white human|random] [--seed N] [sfen]\n";
    std::cout << "                          Play the game.\n";
    std::cout << "                          If you don't specify, it will set human, human, random seed and startpos.\n";

    std::cout << std::endl;
}

int show(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "show requires an SFEN or \"startpos\"." << std::endl;
        return 1;
    }
    if(args[2] == "startpos"){
        std::cout << dobutsu::Position::startpos().to_ascii();
        return 0;
    }
    std::string sfen = "";
    for(int i = 2; i < argc; i++){
        sfen += args[i];
        sfen += " ";
    }
    auto p = dobutsu::Position::from_sfen(sfen);
    if(!p.has_value()){
        std::cerr << "Unrecognized SFEN." << std::endl;
        return 1;
    }else{
        std::cout << p->to_ascii();
    }

    return 0;
}

int perft(int argc, std::vector<std::string> args){
    if(argc < 3){
        std::cerr << "perft requires depth." << std::endl;
        return 1;
    }

    int depth;
    auto [ptr, ec] = std::from_chars(args[2].data(), args[2].data() + args[2].size(), depth);
    if(ec != std::errc{} || ptr != args[2].data() + args[2].size()){
        std::cerr << "Unrecognized depth input." << std::endl;
        return 1;
    }
    if(depth < 1){
        std::cerr << "Invalid depth value." << std::endl;
        return 1;
    }
    if(depth > 10){
        std::cerr << "warning : Too large value of depth. Processing may take a long time\n";
    }

    std::optional<dobutsu::Position> pos;
    std::string sfen = "";
    if(argc > 3){
        if(args[3] == "startpos"){
            pos = dobutsu::Position::startpos();
        }else{
            for(int i = 3; i < argc; i++){
                sfen += args[i];
                sfen += " ";
            }
            pos = dobutsu::Position::from_sfen(sfen);
            if(!pos.has_value()){
                std::cerr << "Unrecognized SFEN." << std::endl;
                return 1;
            }
        }
    }else{
        pos = dobutsu::Position::startpos();
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
    bool black_is_human = true, white_is_human = true;
    std::random_device rd;
    std::uint32_t seed = rd();
    std::optional<dobutsu::Position> pos;
    std::string sfen;

    int i = 2;
    while(i < argc){
        const std::string& a = args[i++];
        if((a == "--black" || a == "--white" || a == "--seed") && i < argc){
            const std::string& v = args[i++];
            if(a == "--black"){ 
                if(v == "human" || v == "random") black_is_human = (v == "human" ? true : false); 
                else{ std::cerr << "Invalid value. (--black)" << std::endl; return 1; }
            }
            else if(a == "--white"){
                if(v == "human" || v == "random") white_is_human = (v == "human" ? true : false); 
                else{ std::cerr << "Invalid value. (--white)" << std::endl; return 1; }
            }
            else {
                const char* end = v.data() + v.size();
                auto [ptr, ec] = std::from_chars(v.data(), end, seed);
                if (ec != std::errc{} || ptr != end) { std::cerr << "Invalid value. (--seed) \n"; return 1; }
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
    std::cout << "Black : " << (black_is_human ? "human" : "random") << ", White : " << (white_is_human ? "human" : "random") << "\n";

    /* 対局ループ */
    std::deque<dobutsu::StateInfo> states;
    std::vector<dobutsu::Move> moves;
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
        auto is_human = [&](dobutsu::Color c){ return c == dobutsu::BLACK ? black_is_human : white_is_human; };
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
        }else{
            dobutsu::Move m = pick_random(pos.value(), rng);
            states.emplace_back();
            pos->do_move(m, states.back());
            moves.push_back(m);
            std::cout << dobutsu::move_to_usi(m) << "\n";
        }
        std::cout << std::endl;
    }
}


int main(int argc, char* argv[]){
    std::vector<std::string> args(argv, argv + argc);

    // 引数をサブコマンドとして処理
    if(argc < 2 || args[1] == "help"){
        help();
        return 0;
    }
    if(args[1] == "show"){
        return show(argc, args);
    }else if(args[1] == "perft"){
        return perft(argc, args);
    }else if(args[1] == "play"){
        return play(argc, args);
    }else{
        std::cerr << "Unrecognized command \"" << args[1] << "\". Use \"help\"." << std::endl;
        return 1;
    }


    return 0;
}
