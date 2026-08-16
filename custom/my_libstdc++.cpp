#include "wrapper.h"

extern "C" {
    WRAP_FUNC_VOID(_ZNSt13runtime_errorD1Ev)
    WRAP_FUNC_VOID(_ZSt29_Rb_tree_insert_and_rebalancebPSt18_Rb_tree_node_baseS0_RS_)
    WRAP_FUNC_VOID(_ZNSt8__detail15_List_node_base7_M_hookEPS0_)
    WRAP_FUNC(_ZNKSt9basic_iosIcSt11char_traitsIcEE4failEv)


    WRAP_FUNC(_Znwm)
    WRAP_FUNC(_Znam)

    WRAP_FUNC_VOID(_ZdaPv)
    WRAP_FUNC_VOID(_ZdlPv)
    WRAP_FUNC_VOID(_ZdlPvm)
    WRAP_FUNC_VOID(_ZdaPvm)

    WRAP_FUNC(_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc)
    WRAP_FUNC(_ZNSolsEl)
    WRAP_FUNC(_ZNSolsEi)
    WRAP_FUNC(_ZNSolsEf)
    WRAP_FUNC(_ZNSolsEd)
    WRAP_FUNC(_ZNSolsEm)
    WRAP_FUNC(_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
    WRAP_FUNC(_ZNSolsEPFRSoS_E)
    WRAP_FUNC(_ZNSirsERj)
    WRAP_FUNC(_ZNSirsERt)
    WRAP_FUNC(_ZNSirsERi)
    WRAP_FUNC(_ZNSirsERf)
    WRAP_FUNC(_ZSt7getlineIcSt11char_traitsIcESaIcEERSt13basic_istreamIT_T0_ES7_RNSt7__cxx1112basic_stringIS4_S5_T1_EE)
    WRAP_FUNC_VOID(_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEED1Ev)
    WRAP_FUNC_VOID(_ZNSo5flushEv)
    
    WRAP_FUNC_VOID(__cxa_end_catch)
    WRAP_FUNC_VOID(__cxa_rethrow)
    WRAP_FUNC_VOID(__cxa_throw)
    WRAP_FUNC_VOID(__cxa_guard_abort)
    WRAP_FUNC_VOID(__cxa_guard_release)
    WRAP_FUNC(__cxa_guard_acquire)
}
