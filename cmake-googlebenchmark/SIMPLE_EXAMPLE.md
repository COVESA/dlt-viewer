# Simple Google Benchmark Example for Search Performance

## What This Example Does

This simple benchmark demonstrates how to measure search performance using Google Benchmark.

### Key Concepts

1. **Sample Data**: Creates a vector of 1,000 to 10,000 log messages
2. **Search Functions**: 
   - `linearSearch()` - Basic string search
   - `caseInsensitiveSearch()` - Case-insensitive search
3. **Benchmarks**: Measures how fast each search method is

### Understanding the Benchmark Code

```cpp
static void BM_LinearSearch_1K(benchmark::State& state) {
    auto messages = createSampleMessages(1000);  // Setup: create test data
    
    for (auto _ : state) {                       // Loop runs many times
        int result = linearSearch(messages, "ERROR");
        benchmark::DoNotOptimize(result);        // Prevent optimization
    }
    
    state.SetItemsProcessed(state.iterations() * messages.size());
}
BENCHMARK(BM_LinearSearch_1K);                   // Register the benchmark
```

### How to Build and Run

#### Option 1: Visual Studio 2022
1. Open the project in Visual Studio
2. Build the solution (Ctrl+Shift+B)
3. Run without debugging (Ctrl+F5)

#### Option 2: CMake Command Line
```powershell
# Configure
cmake --preset=windows-x64-debug

# Build
cmake --build --preset=windows-x64-debug

# Run
.\build\Desktop_Qt_6_8_3_MSVC2022_64bit-Debug\cmake-googlebenchmark.exe
```

#### Option 3: Qt Creator 6.7.3
1. Open CMakeLists.txt in Qt Creator
2. Configure with Desktop Qt 6.8.3 MSVC2022 64-bit kit
3. Build → Run

### Expected Output

```
Run on (X X-Core Processor)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
-------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations
-------------------------------------------------------------------------
BM_LinearSearch_1K                  15.2 us         15.1 us        46545
BM_LinearSearch_10K                  152 us          151 us         4621
BM_CaseInsensitiveSearch_10K         421 us          419 us         1667
BM_SearchComparison/0                76.1 us         75.8 us         9231
BM_SearchComparison/1                76.3 us         76.0 us         9204
```

### Reading the Results

- **Time**: Wall clock time per iteration
- **CPU**: CPU time per iteration (what we care about for CPU %)
- **Iterations**: How many times the benchmark ran to get stable results

### Key Observations

1. **Linear scaling**: 10K messages takes ~10x longer than 1K messages
2. **Case-insensitive is slower**: ~2.8x slower due to string conversion
3. **Search term doesn't matter much**: "ERROR" vs "WARNING" have similar times

### Next Steps

You can modify this example to:
- Test with real DLT message structures
- Add regex search benchmarks
- Test different dataset sizes
- Measure memory usage
- Profile specific bottlenecks in DLT Viewer search

### Understanding CPU Usage

Google Benchmark reports **CPU time**, which tells you how much CPU resource is used:
- Lower CPU time = more efficient search
- Compare different approaches to find the fastest
- Use `--benchmark_repetitions=10` for more stable results
