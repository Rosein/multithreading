#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

class PingPong 
{
    std::atomic<int> repetitions_;
    int numbPings{};
    int numbPongs{};
    std::mutex mtxSwitcher;
    std::condition_variable cv;
    std::atomic<bool> stillPlay = true;
    using Lock = std::unique_lock<mutex>;
public:
    PingPong(int repetitions)
        : repetitions_(repetitions)
    {}

    void ping() {

        while( repetitions_ > 0  && stillPlay )
        {
            Lock locker ( mtxSwitcher );
            cv.wait( locker, [=]{ return ( numbPings != ( numbPongs + 1 ) ) || !stillPlay; } );
            if( !stillPlay )
                break;
            if( repetitions_ > 0 )
            {
                std::stringstream ss;
                ss << "ping " << numbPings++ << '\n';
                std::cout << ss.str();
                cv.notify_all();
            }
            else
            {
                std::cout << "Ping reached repetitions limit\n";
                numbPings++;
                cv.notify_all();
            }
        }
    }

    void pong() 
    {
        while( repetitions_ >= 0 && stillPlay )
        {
            Lock locker ( mtxSwitcher );
            cv.wait( locker, [=]{ return numbPings == ( numbPongs + 1 ) || !stillPlay; } );
            if( !stillPlay )
                break;
            if ( repetitions_ != 0 )
            {
                std::stringstream ss;
                ss << "pong " << numbPongs++ << '\n';
                std::cout << ss.str();
                repetitions_--;
                cv.notify_all();
            }
            else
            {
                std::cout << "Pong reached repetitions limit\n";
                repetitions_--;
                cv.notify_all();
                stillPlay = false;
            }
        }   
    }

void stop( [[maybe_unused]] chrono::seconds timeout ) 
    {
        std::chrono::steady_clock::time_point beg = std::chrono::steady_clock::now();
        Lock locker ( mtxSwitcher );
        cv.wait( locker, [=](){ 
                                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                                // std::stringstream ss;
                                // ss << ( std::chrono::duration_cast<std::chrono::seconds>(end - beg) ).count() << '\n';
                                // std::cout << ss.str();
                                return !stillPlay ||
                                       ( std::chrono::duration_cast<std::chrono::seconds>(end - beg) ).count() >= timeout.count(); 
                                });
        {
            if ( stillPlay )
            {   
                stillPlay = false;
                std::cout << "Timeout" << std::endl;
                cv.notify_all();
            }
        }
    }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        return -1;
    }
    int repetitions = stoi(argv[1]);
    int timeout = stoi(argv[2]);
    PingPong pp(repetitions);
    thread t1(&PingPong::ping, &pp);
    thread t2(&PingPong::pong, &pp);
    thread t3(&PingPong::stop, &pp, chrono::seconds( timeout ));
    t1.join();
    t2.join();
    t3.join();
    return 0;
}
