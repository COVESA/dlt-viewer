// cmake-googlebenchmark.cpp : Simple search performance benchmark example
//

#include "cmake-googlebenchmark.h"
#include <benchmark/benchmark.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

// Sample data: simulate log messages
vector<string> createSampleMessages(int count) {
    vector<string> messages;
    for (int i = 0; i < count; i++) {
        messages.push_back("Message " + to_string(i) + " with some ERROR text");
    }
    return messages;
}

// Simple linear search function
int linearSearch(const vector<string>& messages, const string& searchTerm) {
    int count = 0;
    for (const auto& msg : messages) {
        if (msg.find(searchTerm) != string::npos) {
            count++;
        }
    }
    return count;
}

// Case-insensitive search function
int caseInsensitiveSearch(const vector<string>& messages, const string& searchTerm) {
    int count = 0;
    string lowerSearchTerm = searchTerm;
    transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    
    for (const auto& msg : messages) {
        string lowerMsg = msg;
        transform(lowerMsg.begin(), lowerMsg.end(), lowerMsg.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        if (lowerMsg.find(lowerSearchTerm) != string::npos) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// BENCHMARKS
// ============================================================================

// Benchmark 1: Linear search on 1,000 messages
static void BM_LinearSearch_1K(benchmark::State& state) {
    auto messages = createSampleMessages(1000);
    
    // This loop runs multiple times to get accurate measurements
    for (auto _ : state) {
        int result = linearSearch(messages, "ERROR");
        benchmark::DoNotOptimize(result); // Prevent compiler optimization
    }
    
    // Report throughput
    state.SetItemsProcessed(state.iterations() * messages.size());
}
BENCHMARK(BM_LinearSearch_1K);

// Benchmark 2: Linear search on 10,000 messages
static void BM_LinearSearch_10K(benchmark::State& state) {
    auto messages = createSampleMessages(10000);
    
    for (auto _ : state) {
        int result = linearSearch(messages, "ERROR");
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * messages.size());
}
BENCHMARK(BM_LinearSearch_10K);

// Benchmark 3: Case-insensitive search on 10,000 messages
static void BM_CaseInsensitiveSearch_10K(benchmark::State& state) {
    auto messages = createSampleMessages(10000);
    
    for (auto _ : state) {
        int result = caseInsensitiveSearch(messages, "error");
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * messages.size());
}
BENCHMARK(BM_CaseInsensitiveSearch_10K);

// Benchmark 4: Compare different search terms
static void BM_SearchComparison(benchmark::State& state) {
    auto messages = createSampleMessages(5000);
    string searchTerm = (state.range(0) == 0) ? "ERROR" : "WARNING";
    
    for (auto _ : state) {
        int result = linearSearch(messages, searchTerm);
        benchmark::DoNotOptimize(result);
    }
    
    state.SetItemsProcessed(state.iterations() * messages.size());
}
// Run benchmark with two different arguments (0 and 1)
BENCHMARK(BM_SearchComparison)->Arg(0)->Arg(1);

// Main entry point for Google Benchmark
BENCHMARK_MAIN();
