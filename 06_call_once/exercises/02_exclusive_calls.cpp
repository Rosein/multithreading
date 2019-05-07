#include <vector>
#include <iostream>
#include <mutex>
using namespace std;



class X {
    vector<double> values;
    std::once_flag flag;
    void initializeOne()   
    {
        std::cout << "initializeOne\n";
        std::call_once( flag, [&]{ values = {1.0};
                                    std::cout << "Call once initializeOne\n"; } ); 
    }
    void initializeTwo()   
    { 
        std::cout << "initializeTwo\n";
        std::call_once( flag, [&]{ values = {1.0, 2.0};
                                    std::cout << "Call once initializeTwo\n"; } ); 
    }
    void initializeThree() 
    {
        std::cout << "initializeThree\n";
        std::call_once( flag, [&]{ values = {1.0, 2.0, 3.0};
                                    std::cout << "Call once initializeThree\n"; } ); 
    }

public:
    explicit X(int i) noexcept {
        switch (i) {
        case 2:  // top priority
            initializeTwo();
            [[fallthrough]];
        case 3:
            initializeThree();
            [[fallthrough]];
        default:  // least priority
            initializeOne();
        // yes, replacing [[fallthrough]] with break is a better solution,
        // but this code is for educational purpose only ;)
        }
    }

    void print() {
        for (const auto & value : values)
            cout << value << " ";
        cout << "\n\n";
    }
};

int main() {
    X x2{2};
    x2.print();
    
    X x3{3};
    x3.print();
    
    X x0{0};
    x0.print();
    
    return 0;
}
