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
    void printState( const std::string & foo )
    {
        std::cout << "=============" << foo << "=================" << std::endl
                  << std::boolalpha << "isAlreadyCheckedTime: " << isAlreadyCheckedTime << std::endl
                  << "stillPlay: " << stillPlay << std::endl
                  << "repetitions_: " << repetitions_ << std::endl
                  << "numbPings: " << numbPings << std::endl
                  << "numbPongs: " << numbPongs << std::endl
                  << "========================================" << std::endl;
    }
public:
    PingPong(int repetitions)
        : repetitions_(repetitions)
    {}

    void ping() {

        while( repetitions_ > 0  && stillPlay )
        {
            Lock locker ( mtxSwitcher );
            // printState( "ping" );
            cv.wait( locker, [=]{ return ( numbPings != ( numbPongs + 1 ) && isAlreadyCheckedTime ) || !stillPlay; } );
            isAlreadyCheckedTime = false;
            if( !stillPlay )
                break;
            if( repetitions_ > 0 )
            {
                std::stringstream ss;
                ss << "$ ping " << numbPings++ << '\n';
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

    void pong() {
        while( repetitions_ >= 0 && stillPlay )
        {
            Lock locker ( mtxSwitcher );
            // printState( "pong" );
            cv.wait( locker, [=]{ return numbPings == ( numbPongs + 1 ) || !stillPlay; } );
            if( !stillPlay )
                break;
            if ( repetitions_ != 0 )
            {
                std::stringstream ss;
                ss << "$ pong " << numbPongs++ << '\n';
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

    void stop([[maybe_unused]] chrono::seconds timeout) {
        // TODO: should stop execution after timeout
        while( timeout.count() >= 0 && repetitions_ >= 0) {
            Lock locker ( mtxSwitcher );
            
            cv.wait_for( locker, std::chrono::seconds( 1 ), [=]{ return numbPings != ( numbPongs + 1 ) && !isAlreadyCheckedTime; } );
            
            // printState( "stop" );
            // std::cout << "$ time counter :" <<  timeout.count() << std::endl;
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
