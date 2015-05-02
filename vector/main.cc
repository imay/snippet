// This is a test to test constructor and deconstructor of vector
//
#include <vector>
#include <iostream>

class foo {
public:
    foo() {
        std::cout << "foo constructor 1" << std::endl;
    }
    foo(const foo& other) {
        std::cout << "foo constructor 2" << std::endl;
    }
    ~foo() {
        std::cout << "foo deconstructor" << std::endl;
    }
};

void test1() {
    std::cout << "########## test1  stats ##########" << std::endl;
    std::vector<foo> vec;
    vec.resize(10);
    std::cout << "########## test1 finish ##########" << std::endl;
}

void test2() {
    std::cout << "########## test2  stats ##########" << std::endl;
    std::vector<foo> vec;
    vec.push_back(foo());
    std::cout << "########## test2 finish ##########" << std::endl;
}

int main() {
    test1();
    test2();
    return 0;
}
