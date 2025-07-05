#include "dltfileindexer.h"

#include "qdltdefaultfilter.h"
#include "qdltfile.h"
#include "qdltpluginmanager.h"

#include <QTest>
#include <qtestcase.h>

#include <filesystem>

class TestDltFileIndexer : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() { std::filesystem::remove_all("index/"); }

    void constructor() { QVERIFY_THROWS_NO_EXCEPTION(DltFileIndexer{}); }

    void constructor_withargs() {
        QDltFile qfile{};
        QDltPluginManager pluginManager{};
        QDltDefaultFilter defaultFilter{};
        DltFileIndexer fileIndexert{&qfile, &pluginManager, &defaultFilter, nullptr};
    }

    void run() {
        QString filename = "testfile.dlt";
        QDltFile dltFile{};
        dltFile.open(filename);
        QDltPluginManager pluginManager;
        pluginManager.loadPlugins(QString{});

        DltFileIndexer fileIndexer{&dltFile, &pluginManager, nullptr, nullptr};
        fileIndexer.setFilterCacheEnabled(true);
        fileIndexer.run();

        // QVERIFY(result);
    }
};

QTEST_MAIN(TestDltFileIndexer)
#include "test_dltfileindexer.moc"
