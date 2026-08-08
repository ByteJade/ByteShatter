#include <iostream>

extern "C" {
    std::ostream _ZSt4cout(std::cout.rdbuf());
    std::ostream _ZSt4cerr(std::cerr.rdbuf());

    const std::type_info& _ZTISt13runtime_error = typeid(std::runtime_error);

    void my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os, const char* str) {
        os << str;
    }
    void _ZNSt13runtime_errorD1Ev(std::runtime_error* this_ptr) {
        
    }
    std::ostream& my__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(std::ostream& os) {
        os << '\n';
        os.flush();
        return os;
    }
}