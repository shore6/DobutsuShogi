#include <dobutsu/types.hpp>
#include <string>
#include <string_view>

namespace dobutsu {
    
    std::string square_to_usi(Square s){
        if(!is_ok(s)) return "";
        const char c1 = char('3' - file_of(s));
        const char c2 = char('a' + rank_of(s));
        return {c1, c2};
    }

    Square square_from_usi(std::string_view usi){
        if(usi.size() != 2 || usi[0] < '1' || usi[0] > '3' || usi[1] < 'a' || usi[1] > 'd') return SQ_NONE;
        return make_square(File('3' - usi[0]), Rank(usi[1] - 'a'));
    }
}
