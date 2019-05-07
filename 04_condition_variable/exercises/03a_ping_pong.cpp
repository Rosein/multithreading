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
    int repetitions_;
    int numbPings{};
    int numbPongs{};
    std::mutex mtxSwitcher;
    std::condition_variable cv;
    std::atomic<bool> isAlreadyCheckedTime = true;
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
            cv.wait( locker, [=]{ return ( numbPings != ( numbPongs + 1 ) && isAlreadyCheckedTime ) || !stillPlay; } );
            isAlreadyCheckedTime = false;
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
            }
        }   
    }

    void stop([[maybe_unused]] chrono::seconds timeout) 
    {
        while( timeout.count() >= 0 && repetitions_ >= 0) 
        {
            Lock locker ( mtxSwitcher );
            cv.wait_for( locker, std::chrono::seconds( 1 ), [=]{ return numbPings != ( numbPongs + 1 ) && !isAlreadyCheckedTime; } );
            std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
            timeout = timeout - std::chrono::seconds( 1 );
            isAlreadyCheckedTime = true;
            if( timeout <= std::chrono::seconds(0) )
            {
                stillPlay = false;
                std::cout << "Timeout" << std::endl;
                cv.notify_all();
                break;
            }
            cv.notify_all();
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
