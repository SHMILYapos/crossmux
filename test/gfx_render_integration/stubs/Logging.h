#pragma once

#include <cstdio>

// Host-test stub: route the firmware's log macros to stdout so a failing path
// (e.g. "Compressed font but no FontDecompressor set") shows up in test logs.
#define LOG_ERR(tag, fmt, ...) std::fprintf(stderr, "[" tag "] " fmt "\n", ##__VA_ARGS__)
#define LOG_WARN(tag, fmt, ...) std::fprintf(stderr, "[" tag "] " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(tag, fmt, ...) std::fprintf(stderr, "[" tag "] " fmt "\n", ##__VA_ARGS__)
#define LOG_DBG(tag, fmt, ...) std::fprintf(stderr, "[" tag "] " fmt "\n", ##__VA_ARGS__)
