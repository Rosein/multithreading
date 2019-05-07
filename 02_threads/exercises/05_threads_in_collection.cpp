#include<vector>
#include<iostream>
#include<thread>
#include<sstream>
#include<mutex>

std::mutex coutMtx;
using namespace std;

int main() {

    std::vector<std::thread> threads;

    for( int i = 0; i < 20; ++i )
        threads.emplace_back( std::thread{[i](){
            std::this_thread::sleep_for( 1s );
            std::stringstream ss;
            ss << i << "\t" << std::this_thread::get_id() << "\n";
            coutMtx.lock();
            std::cout << ss.str();
            coutMtx.unlock();
        }});
    
    for( auto && t : threads )
        t.join();
    

    return 0;
}
