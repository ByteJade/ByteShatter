#include "wrapper.h"
#include <iostream>

namespace std {
    namespace __detail {
        struct _List_node_base {
            _List_node_base* _M_next;
            _List_node_base* _M_prev;
        };
    }
}

extern "C" {
    void my__ZNSt13runtime_errorD1Ev(std::runtime_error* self) {
        // C++ will do the cleanup itself
    }
    bool my__ZNKSt9basic_iosIcSt11char_traitsIcEE4failEv(std::basic_ios<char, std::char_traits<char>>* self) {
        return (self->rdstate() & (std::ios_base::failbit | std::ios_base::badbit)) != 0;
    }
    void my__ZNSt8__detail15_List_node_base7_M_hookEPS0_(
        std::__detail::_List_node_base* self, 
        std::__detail::_List_node_base* position
    ) {
        self->_M_next = position;
        self->_M_prev = position->_M_prev;
        
        position->_M_prev->_M_next = self;
        position->_M_prev = self;
    }
    void* _Znwm(size_t size) {
        return malloc(size);
    }
    void* _Znam(size_t size) {
        return malloc(size);
    }
    void _ZdaPv(void* ptr) {
        free(ptr);
    }
    void _ZdlPv(void* ptr) noexcept {
        free(ptr);
    }
    void _ZdlPvm(void* ptr, size_t size) {
        free(ptr);
    }
    void _ZdaPvm(void* ptr, size_t size) {
        free(ptr);
    }
    std::ostream& my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os, const char* str) {
        return os << str;
    }
    std::ostream& my__ZNSolsEl(std::ostream& os, long n) {
        return os << n;
    }
    std::ostream& my__ZNSolsEi(std::ostream& os, int n) {
        return os << n;
    }
    std::ostream& my__ZNSolsEf(std::ostream& os, float n) {
        return os << n;
    }
    std::ostream& my__ZNSolsEd(std::ostream& os, double n) {
        return os << n;
    }
    std::ostream& my__ZNSolsEm(std::ostream& os, unsigned long n) {
        return os << n;
    }
    std::ostream& my__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(std::ostream& os) {
        return os << std::endl;
    }
    std::istream& my__ZNSirsERj(std::istream* self, unsigned int& n) {
        return self->operator>>(n);
    }
    std::istream& my__ZNSirsERt(std::istream* self, unsigned short& n) {
        return self->operator>>(n);
    }
    std::istream& my__ZNSirsERf(std::istream* self, float& f) {
        return self->operator>>(f);
    }
    void _ZNSo5flushEv(std::ostream& os) {
        os.flush();
    }
    WRAP_FUNC_VOID(__cxa_finalize)
}
