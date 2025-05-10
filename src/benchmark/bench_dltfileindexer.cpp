#include "dltfileindexer.h"
#include "qdltfile.h"

#include <benchmark/benchmark.h>

#include <filesystem>

static void DoSetup(const benchmark::State& /*state*/) { std::filesystem::remove_all("index/"); }


static void BN_dltfileindexer_run(benchmark::State& state) {
    QString filename = "testfile.dlt";
    for (auto _ : state) {
        QDltFile dltFile{};
        dltFile.open(filename);
        QDltPluginManager pluginManager;
        pluginManager.loadPlugins(QString{});

        DltFileIndexer fileIndexer{&dltFile, &pluginManager, nullptr, nullptr};
        fileIndexer.setFilterCacheEnabled(true);
        fileIndexer.run();
    }
}
BENCHMARK(BN_dltfileindexer_run)->Setup(DoSetup);

BENCHMARK_MAIN();
