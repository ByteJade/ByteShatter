#include <iostream>

extern "C" {
    extern std::ostream my__ZSt4cout;
    
    void my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os_ptr, const char* str) {
        os_ptr << str;
    }
}