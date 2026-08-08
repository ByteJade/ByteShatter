#include <iostream>

extern "C" {
    std::ostream ZSt4cout(std::cout.rdbuf());

    void my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os, const char* str) {
        os << str;
    }
    std::ostream& my__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(std::ostream& os) {
        os << '\n';
        os.flush();
        return os;
    }
}