#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <ctime>                // std::tm
#include <iomanip>              // std::put_time
#include <string>
#include <sstream>              // allows you to treat a string as an object (ostringstream)
#include <cstdint>              // provides fixed width integer types and numeric limits (i.e., int64_t)

// returns a string of the timestamp
std::string generate_timestamp(std::int64_t ms) {
    std::time_t secs = ms /1000;
    int millis = ms % 1000;
    std::tm tm = *std::gmtime(&secs);           // tm holds calendar date and time
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << millis;
    return out.str();
}

int main() {

    // create file
    std::ofstream logFile("log.txt");

    // initialize with seed (currently 12345 for reproducable tests and timing) used std:: random_device{}() for actual random
    std::mt19937 rng(12345);

    // define time range (Jan 1 2026 - Jan 1 2027) Ok if it spills into 2027
    std::uniform_int_distribution<std::time_t> dist(1767225600, 1798761600);

    // generate random timestamp 
    std::int64_t now_ms = dist(rng) * 1000;
    std::uniform_int_distribution<int> step(0, 5);          //0 - 5 ms per line to spawn

    // count number of lines
    int count = 0;

    // check if file was successfully created
    if (logFile.is_open()) {
       
        // write log data to file
        while (count < 1000000) {
            count += 1;
            now_ms += step(rng);

            // print 
            logFile << generate_timestamp(now_ms) << "rest of info / latency / etc" << '\n';

        }

        // close file and free memory
        logFile.close();
    } else {
        std::cerr << "Error: Could not create or open file.\n";
    }

    return 0;
}