# sessionstats

A C++17 tool that generates a synthetic game-streaming session log and
analyzes it: latency percentiles (p50/p95/p99), error rate, and slowest sessions.

This is a small version of the telemetry analysis used to monitor cloud
gaming services.

## Status

Work in progress.

- [x] `create_data.cpp`: generates a log of 1,000,000 lines
- [x] Log lines with session, event and latency
- [ ] `log_parser.cpp`: parse, percentiles, error rate, histogram
- [ ] Tests
- [ ] Multithreaded parsing and benchmarks

## Build (Windows, MSVC)

    cl /std:c++17 /EHsc /O2 create_data.cpp
    .\create_data.exe

This writes `log.txt`. The log is not committed because of its size.

## Build (Linux)

    g++ -std=c++17 -O2 -o create_data create_data.cpp
    ./create_data
