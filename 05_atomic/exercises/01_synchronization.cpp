// use store() /  load()
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <iomanip>
#include <iostream>
using namespace std;

vector<int> generateContainer() {
    std::mutex outputMtx;
    vector<int> input = {2, 4, 6, 8, 10, 1, 3, 5, 7, 9};
    vector<int> output;
    vector<thread> threads;
    for (auto i = 0u; i < input.size(); i++) {
        threads.emplace_back([&, i]{
            std::lock_guard<mutex> locker ( outputMtx );
            output.push_back(input[i]); 
        });
    }
    for (auto && t : threads) {
        t.join();
    }
    return output;
}

vector<int> generateOtherContainer() {
    atomic<int> start;
    start.store( 5 );
    atomic<bool> add ;
    add.store( true );
    std::mutex outputMtx;
    vector<int> output;
    vector<thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&, i]{
            
            if (add)
            {
                std::lock_guard<mutex> locker ( outputMtx );
                output.push_back(start+=i);
            }
            else
            {
                std::lock_guard<mutex> locker ( outputMtx );
                output.push_back(start-=i);
            }
            add = !add;
        });
    }
    for (auto && t : threads) {
        t.join();
    }
    return output;
}

void powerContainer(vector<int>& input) {
    vector<thread> threads;
    for (auto i = 0u; i < input.size(); i++) {
        threads.emplace_back([&, i ]{ input[ i ]*=input[ i ]; });
    }
    for (auto && t : threads) {
        t.join();
    }
}

void printContainer(const vector<int>& c) {
    for (const auto & value : c)
        cout << setw( 4 ) << value ;
    cout << endl;
}

int main() {
    auto container1 = generateContainer();
    printContainer(container1);
    powerContainer(container1);
    printContainer(container1);

    auto container2 = generateOtherContainer();
    printContainer(container2);
    powerContainer(container2);
    printContainer(container2);
    return 0;
}
