#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>

using namespace std::chrono;

class CELLTimestamp {
protected:
    time_point<high_resolution_clock> begin;

public:
    CELLTimestamp() {
        update();
    }

    ~CELLTimestamp() {

    }

    void update() {
        begin = high_resolution_clock::now();
    }

    // second
    double GetElapsedSecond() {
        return this->GetElapsedTimeInMicroSec() * 0.000001;
    }

    // millisecond
    double GetElapsedTimeInMilliSec() {
        return this->GetElapsedTimeInMicroSec() * 0.001;
    }

    // microsecond
    long long GetElapsedTimeInMicroSec() {

        return duration_cast<microseconds>(high_resolution_clock::now() - begin).count();
    }

};

#endif