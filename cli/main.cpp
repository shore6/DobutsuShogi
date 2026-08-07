#include <iostream>
#include <dobutsu/types.hpp>

int main(){
    std::cout << "dobutsu Version 0.1.0" << std::endl;
    
    // USI表記の確認
    for(int i = 0; i < dobutsu::SQ_NB; i++){
        std::cout << dobutsu::square_to_usi(dobutsu::Square(i)) << ' ';
        if(i % 3 == 2) std::cout << std::endl;
    }

    return 0;
}
