#include <iostream>

extern "C" {
    void my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os, const char* str) {
        os << str;
    }
    void my__ZNSt13runtime_errorD1Ev(std::runtime_error* this_ptr) {
        // C++ will do the cleanup itself
    }
    std::ostream& my__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(std::ostream& os) {
        os << '\n';
        os.flush();
        return os;
    }
}
