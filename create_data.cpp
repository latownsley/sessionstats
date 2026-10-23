#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <ctime>                // std::tm
#include <iomanip>              // std::put_time
#include <string>
#include <sstream>              // allows you to treat a string as an object (ostringstream)
#include <cstdint>              // provides fixed width integer types and numeric limits (i.e., int64_t)
#include <vector>

struct Session {
    int id;
    bool connected = false;
    bool disconnected = false;          // ran through whole session do not reuse
    double baseline_latency;       //tba
    
};

// returns a string of the timestamp
std::string generate_timestamp(std::int64_t ms) {
    std::time_t secs = ms /1000;
    int millis = ms % 1000;
    std::tm tm = *std::gmtime(&secs);           // tm holds calendar date and time
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << millis;
    return out.str();
}

// returns a string for a random event
// if it hasn’t connected, connect; 
// otherwise a random number picks frame (about 98%), error (about 1%) or disconnect (~1%).
std::string generate_event(std::mt19937& rng, Session& curr_session) {
    std::string event_string;
    if (!curr_session.connected) {
        event_string = "event=connect";
        curr_session.connected = true;
    } else {
        std::uniform_int_distribution<int> freq_dist(1, 100);
        int random_freq = freq_dist(rng);

        if (random_freq <= 98) {
            // 98% of the time pick frame
            event_string = "event=frame";

        }

        else if (random_freq == 99) {
            event_string = "event=error";
        }

        else {
            event_string = "event=disconnect";
            curr_session.disconnected = true;
        }

    }

    return event_string;

}

// returns a string of randomly generated latency
std::string generate_latency(Session& curr_session, std::string event_type, std::mt19937& rng, double jitter,  std::uniform_real_distribution<double> chance_of_spike, std::uniform_real_distribution<double> spike) {
    // generate latency using std::lognormal_distribution
    std::ostringstream latency; 
    if (event_type =="event=connect") {
        //setup latency = 50-150 uniform
        std::uniform_real_distribution<double> latency_options(50.0, 150.0);
        double connect_latency = latency_options(rng);

        // set the baseline
        std::lognormal_distribution<double> dist(log(20), 0.3);
        curr_session.baseline_latency = dist(rng);

        //to_string gives six decimals so set it to 1 before
        latency << "latency_ms=" << std::fixed << std::setprecision(1) << connect_latency;
        return latency.str();

    } else if (event_type == "event=frame") {
        double new_latency = curr_session.baseline_latency * jitter;

        if (chance_of_spike(rng) < 0.02) {
            new_latency *= spike(rng);
        }

        latency << "latency_ms=" << std::fixed << std::setprecision(1) << new_latency;
        return latency.str();
    } else {
        return "latency_ms=0";
    }
}

// create vec of 50 active sessions
std::vector<Session> generate_active_sessions(std::mt19937& rng) {
    std::vector<Session> active_sessions(50);
    std::uniform_int_distribution<int> min_max_sessions(1, 3000);
    std::int64_t next_id = min_max_sessions(rng);

    for (auto& session : active_sessions) {
        session.id = next_id;
        next_id++;
    }

    return active_sessions;
    
}

// get random session index
int generate_random_session_index (std::mt19937& rng) {
    std::uniform_int_distribution<int> index(0, 49);
    int random_index = index(rng);
    return random_index;
}

int main() {

    // create file
    std::ofstream logFile("log.txt");

    // initialize with seed (currently 12345 for reproducable tests and timing) use std:: random_device{}() for actual random
    std::mt19937 rng(12345);

    // define time range (Jan 1 2026 - Jan 1 2027) Ok if it spills into 2027
    std::uniform_int_distribution<std::time_t> dist(1767225600, 1798761600);

    // generate random timestamp 
    std::int64_t now_ms = dist(rng) * 1000;
    std::uniform_int_distribution<int> step(0, 5);          //0 - 5 ms per line to spawn

    // count number of lines
    int count = 0;
    int bad_line_count = 0;

    // random vec of 50 active random sessions
    std::vector<Session> active_sessions = generate_active_sessions(rng);

    int next_id = active_sessions.back().id + 1;

    // generate random numbers for latency
    std::lognormal_distribution<double> jitter(0.0, 0.15);
    std::uniform_real_distribution<double> chance_of_spike(0.0, 1.0);
    std::uniform_real_distribution<double> spike(3.0, 6.0); 

    double jitter_num = jitter(rng);

    // check if file was successfully created
    if (logFile.is_open()) {
       
        // write log data to file (timestamp session=ID event=NAME latency_ms=VALUE)
        while (count < 1000000) {

            int idx = generate_random_session_index(rng);

            // connect to random session
            Session& s = active_sessions[idx];

            // if session disconnect, reset to new session
            if (s.disconnected) {
                s = Session{};
                s.id = next_id++;
            }

            count += 1;
            now_ms += step(rng);            // advance clock
            std::string timestamp = generate_timestamp(now_ms);
            std::string event = generate_event(rng, s);
            std::string latency = generate_latency(s, event, rng, jitter_num, chance_of_spike, spike);

            // bad lines to skip
            std::uniform_int_distribution<int> bad_line_freq(0, 100);
            int bad_line = bad_line_freq(rng);
            if (bad_line < 1) {
                logFile << "this is a bad line. you should probably skip me log parser." << '\n';
                bad_line_count += 1;
            }

            // print 
            logFile << timestamp << " session=" << s.id << " " << event << " " << latency << '\n';


        }

        // close file and free memory
        logFile.close();
    } else {
        std::cerr << "Error: Could not create or open file.\n";
    }

    std::cerr << "There were " << bad_line_count <<" bad lines.";

    return 0;
}