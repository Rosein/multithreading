#include <mutex>
#include <shared_mutex>
#include <vector>
#include <iostream>
#include <thread>
using namespace std;

vector<int> numbers = {};

int getNextValue() 
{
    static int i = 0;
    return i+=10;
}

void read(int index, std::mutex & coutMtx, std::shared_mutex & numbersMtx ) 
{
    std::shared_lock<std::shared_mutex> readLocker( numbersMtx );
    int value = numbers[index];
    readLocker.unlock();
    std::lock_guard<std::mutex> locker ( coutMtx );
    cout << value << " ";
}

void write( std::shared_mutex & numbersMtx ) 
{
    std::lock_guard<std::shared_mutex> locker ( numbersMtx );
    int newValue = getNextValue();
    numbers.emplace_back(newValue);
}

int main()
{
    std::shared_mutex numbersMtx;
    vector<thread> writers;
    for(int i = 0; i < 10; i++)
        writers.emplace_back(write, std::ref( numbersMtx ));
    for(auto && writer : writers)
    	writer.join();

    cout << "Writers produced: ";
    for(const auto & n : numbers)
        cout << n << " ";
    cout << endl;

    std::mutex coutMtx;
    cout << "Readers consumed: ";
    vector<thread> readers;
    for(int i = 0; i < 10; i++)
        readers.emplace_back( read, i, std::ref( coutMtx ) , std::ref( numbersMtx ));
    for(auto && reader : readers)
        reader.join();

    cout << endl;
    return 0;
}
