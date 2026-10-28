#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <unordered_map>
#include <algorithm>

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
    double errors;
};

struct Overall_Stats {
    std::vector<Session> five_slowest;
    std::vector<double> all_latencies;
    int good_lines = 0;
    int bad_lines = 0;
    double error_rate;
    double p50;
    double p95;
    double p99;
};

bool confirm_valid_structure(std::string& curr_line, Overall_Stats& all_sessions);
void process_line(std::string curr_line, Overall_Stats& all_sessions, std::unordered_map<int, Session> seen_sessions);
std::string process_event(std::string event, Record& curr_record, Session& curr_session);
Session process_session(std::string session, Record& curr_record, std::unordered_map<int, Session> seen_sessions);
double process_latency(std::string latency, Session& curr_session);
bool is_valid_date(int year, int month, int day);
int count_digits(int num);
void print_session_stats(Overall_Stats& all_sessions, std::unordered_map<int, Session>& seen_sessions);
double calculate_percent(std::vector<double> latencies, double percent);
void sort_all_latencies(Overall_Stats& all_sessions, std::unordered_map<int, Session>& seen_sessions);

int main() {

    Overall_Stats all_sessions;     // holds stats for all sessions read
    std::unordered_map<int, Session> seen_sessions;  // holds sessions seen while reading

    std::ifstream log("log.txt");       // open

    if (!log.is_open()) {
        std::cerr << "Error: Could not open file.\n";
        return 1;
    }

    std::string curr_line;

    // loop through log line by line
    while(std::getline(log, curr_line)) {
        process_line(curr_line, all_sessions, seen_sessions);
    }

    // process data collected
    sort_all_latencies(all_sessions, seen_sessions);
    print_session_stats(all_sessions, seen_sessions);

    log.close();

    return 0;

    
    // Use std::string_view or from_chars rather than regex. It’s much faster, and it makes a good README point.

    //Write a few assert-based tests for the parser and the percentile function
    // Split the file into chunks, parse them with std::thread workers, and merge the results.
    // Compare timings for 1, 2 and 4  threads. A speedup table makes the repo stand out.
    //make a hello world cmake on windows to use


}

void sort_all_latencies(Overall_Stats& all_sessions, std::unordered_map<int, Session>& seen_sessions) {
    for (const auto& [key, value] : seen_sessions) {
        std::sort(value.latencies.begin(), value.latencies.end()); 
    }

    std::sort(all_sessions.all_latencies.begin(), all_sessions.all_latencies.end());
    
}

void print_session_stats(Overall_Stats& all_sessions, std::unordered_map<int, Session>& seen_sessions)
{
    
    // calculate p50, p95, p99 latency overall and per session. Find the slowest 5 sessions
    for (const auto& [key, value] : seen_sessions) {
        double p50 = calculate_percent(value.latencies, 50.0);
        double p95 = calculate_percent(value.latencies, 95.0);
        double p99 = calculate_percent(value.latencies, 99.0); 
    }

    //report p50, p95, p99 latency, error rate per session
    // report overall p50, p95, p99 latency, error rate, and top 5 slowest sessions
    // print text histogram of latencies
}

double calculate_percent(std::vector<double> latencies, double percent) {
    // calculate p50, p95, p99 latency overall and per session. Find the slowest 5 sessions
    if (latencies.empty()) {
        return 0.0;
    }

    // clamp it to the possible indices (make sure latencies is already sorted here)
    int index = std::clamp(ceil(percent / 100.0 * latencies.size()) - 1, 0, latencies.size() - 1);

    return latencies[index];


}

bool confirm_valid_structure(std::string& curr_line, Overall_Stats& all_sessions) {
    //confirm structure
    std::string event_phrase = "event=";
    std::string session_phrase = "session=";
    std::string latency_phrase = "latency_ms=";

    if (curr_line.find(event_phrase) == std::string::npos || curr_line.find(session_phrase) == std::string::npos || curr_line.find(latency_phrase) == std::string::npos) {
        all_sessions.bad_lines += 1;
        return false;
    }
    
    all_sessions.good_lines += 1;

    return true;
}


void process_line(std::string curr_line, Overall_Stats& all_sessions, std::unordered_map<int, Session> seen_sessions) {
    if (!confirm_valid_structure(curr_line, all_sessions)){
        // return and do nothing for bad lines
        return;
    }

    Record curr_record;
    std::istringstream text(curr_line);

    std::string timestamp;
    std::string session;
    std::string event;
    std::string latency;

    text >> timestamp >> session >> event >> latency;

    // process timestamp
    curr_record.timestamp = timestamp;

    // process session
    Session curr_session = process_session(session, curr_record, seen_sessions);

    // process event 
    curr_record.event = process_event(event, curr_record, curr_session);

    // process latency
    curr_record.latency = process_latency(latency, curr_session);
    all_sessions.all_latencies.insert(all_sessions.all_latencies.begin(), curr_record.latency);

    return;

}

std::string process_event(std::string event, Record& curr_record, Session& curr_session) {
    std::string phrase = "event=";
    event.erase(0, phrase.length());

    if (event == "frame") {
        curr_record.event = "frame";
    }

    if (event == "connect") {
        curr_record.event = "connect";
    }

    if (event == "disconnect") {
        curr_record.event = "disconnect";
    }

    if (event == "error") {
        curr_record.event = "error";
        curr_session.errors += 1;  
    }

    return event;
}

Session process_session(std::string session, Record& curr_record, std::unordered_map<int, Session> seen_sessions) {
    std::string phrase = "session=";
    session.erase(0, phrase.length());
    curr_record.session_id = std::stoi(session);

    // search seen_sessions and if we've seen it, return that current session
    if (seen_sessions.count(curr_record.session_id) == 1) {
        return seen_sessions.at(curr_record.session_id);
    }

    // if not seen, make a new session, add it to seen_sessions and return it
    Session curr_session = {curr_record.session_id};
    seen_sessions.insert(curr_record.session_id, curr_session);

    return curr_session;  // maybe return current session instead and store this in that?
}

double process_latency(std::string latency, Session& curr_session) {
    std::string phrase = "latency_ms=";
    latency.erase(0, phrase.length());

    return std::stod(latency);

}

bool is_valid_date(int year, int month, int day) {
    // bounds of month and day
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    bool valid_day = false;
    bool valid_year = false;

    int days_in_each_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        days_in_each_month[1] = 29;
    }

    // day has a valid number of days for the month
    if (day <= days_in_each_month[month - 1]) {
        valid_day = true;
    }

    // only count timestamps from years with 4 digits
    if (count_digits(year) == 4) {
        valid_year = true;
    }

    return valid_day && valid_year;

}

int count_digits(int num) {
    if (num == 0) {
        return 1;
    }
    return std::floor(std::log10(std::abs(num))) + 1;
}

bool is_leap_year(int year) {
    bool is_leap_year = false;

    // leap years are divisible by 4 & not divisible by 100, except if divisible by 400 (wtf is our calendar)
    if (year % 4 == 0){
        if (year % 100 != 0) {
            is_leap_year = true;
        } else{
            if (year % 400 == 0) {
                is_leap_year = true;
            }
        }
    } 
    return is_leap_year;
}