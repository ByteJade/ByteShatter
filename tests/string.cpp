#include <iostream>
#include <string>
#include <iostream>
#include <cstring>
#include <cassert>

void test_string_data() {
    std::string str = "Hello";
    
    const char* ptr1 = str.c_str();
    const char* ptr2 = str.data();
    
    std::cout << ptr1 << " " << ptr2 << "\n";

    assert(ptr1 == ptr2);
    assert(ptr1[0] == 'H');
    assert(ptr1[5] == '\0');
    assert(strlen(ptr1) == 5);
    
    std::cout << "Basic data access test passed\n";
}

void test_empty_string() {
    std::string empty;
    const char* ptr = empty.c_str();
    assert(ptr != nullptr);
    assert(ptr[0] == '\0');
    
    std::cout << "Empty string test passed\n";
}

void test_string_modification() {
    std::string str = "Hello";
    str += " World";
    
    const char* ptr = str.c_str();
    assert(strcmp(ptr, "Hello World") == 0);
    
    std::cout << "String modification test passed\n";
}

void test_copy_constructor() {
    std::string original = "Test string";
    std::string copy = original;
    
    assert(copy == original);
    assert(copy.c_str() != original.c_str());
    
    std::cout << "Copy constructor test passed\n";
}

void test_move_constructor() {
    std::string original = "Move me";
    const char* original_ptr = original.c_str();
    
    std::string moved = std::move(original);
    
    assert(moved == "Move me");
    
    std::cout << "Move constructor test passed\n";
}

void test_assignment() {
    std::string str1 = "First";
    std::string str2 = "Second";
    
    str1 = str2;
    assert(str1 == "Second");
    
    str1 = "Direct";
    assert(str1 == "Direct");
    
    std::cout << "Assignment test passed\n";
}

int main() {
    test_string_data();
    test_empty_string();
    test_string_modification();
    test_copy_constructor();
    test_move_constructor();
    test_assignment();
}