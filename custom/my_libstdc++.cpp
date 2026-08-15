#include "wrapper.h"
#include <iostream>
#include <new>

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
    void my__ZNKSt9basic_iosIcSt11char_traitsIcEE4failEv(std::basic_ios<char, std::char_traits<char>>* self) {
        bool data = (self->rdstate() & (std::ios_base::failbit | std::ios_base::badbit)) != 0;
        RETURN()
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
    void my__Znwm(size_t size) {
        void* data = new char[size];
        RETURN()
    }
    void my__Znam(size_t size) {
        void* data = new char[size];
        RETURN()
    }
    void my__ZdaPv(void* ptr) {
        ::operator delete (ptr);
    }
    void my__ZdlPv(void* ptr) noexcept {
        ::operator delete (ptr);
    }
    void my__ZdlPvm(void* ptr, size_t size) {
        ::operator delete (ptr);
    }
    void my__ZdaPvm(void* ptr, size_t size) {
        ::operator delete (ptr);
    }
    void my__ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(std::ostream& os, const char* str) {
        os << str;
        RETURN()
    }
    void my__ZNSolsEl(std::ostream& os, long n) {
        os << n;
        RETURN()
    }
    void my__ZNSolsEi(std::ostream& os, int n) {
        os << n;
        RETURN()
    }
    void my__ZNSolsEf(std::ostream& os, float n) {
        os << n;
        RETURN()
    }
    void my__ZNSolsEd(std::ostream& os, double n) {
        os << n;
        RETURN()
    }
    void my__ZNSolsEm(std::ostream& os, unsigned long n) {
        os << n;
        RETURN()
    }
    void my__ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(std::ostream& os) {
        os << std::endl;
        RETURN()
    }
    void my__ZNSolsEPFRSoS_E(std::ostream& os, std::ostream& (*pf)(std::ostream&)) {
        std::ostream& pfos = pf(os);
        RETURN()
    }
    void my__ZNSirsERj(std::istream* self, unsigned int& n) {
        self->operator>>(n);
        RETURN()
    }
    void my__ZNSirsERt(std::istream* self, unsigned short& n) {
        self->operator>>(n);
        RETURN()
    }
    void my__ZNSirsERi(std::istream* self, int& n) {
        self->operator>>(n);
        RETURN()
    }
    void my__ZNSirsERf(std::istream* self, float& f) {
        self->operator>>(f);
        RETURN()
    }
    void my__ZNSo5flushEv(std::ostream& os) {
        os.flush();
    }
    WRAP_FUNC_VOID(__cxa_end_catch)
    WRAP_FUNC_VOID(__cxa_rethrow)
    WRAP_FUNC_VOID(__cxa_throw)
    WRAP_FUNC_VOID(__cxa_guard_abort)
    WRAP_FUNC_VOID(__cxa_guard_release)
    WRAP_FUNC(__cxa_guard_acquire)
}
