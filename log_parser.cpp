#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct Record {
    std::string timestamp;
    int session_id;
    std::string event;
    double latency;

};

struct Session {
    int id;
    std::vector<double> latencies;
    double p50;
    double p95;
    double p99;
    double error_rate;
};

struct Overall_Stats {
    std::vector<Session> five_slowest;
    double error_rate;
    double p50;
    double p95;
    double p99;
};

int main() {

    std::ifstream log("log.txt");       // open

    if (!log.is_open()) {
        std::cerr << "Error: Could not open file.\n";
        return 1;
    }

    std::string curr_line;

    // loop through log line by line
    while(std::getline(log, curr_line)) {
        // process the line
    }

    log.close();

    return 0;

    // read line
    // skip bad lines and count them
    // pull out timestamp, session ID, event and latency
    // Use std::string_view or from_chars rather than regex. It’s much faster, and it makes a good README point.
    //for every session (and overall), report p50, p95, p99 latency, error rate, and top 5 slowest sessions
    // print a text histogram of latencies
    //Write a few assert-based tests for the parser and the percentile function
    // Split the file into chunks, parse them with std::thread workers, and merge the results.
    // Compare timings for 1, 2 and 4  threads. A speedup table makes the repo stand out.


    //make a hello world cmake on windows to use


}