// SPDX-License-Identifier: MPL-2.0
// QTest coverage for TableModel — rowCount, columnCount, data() display,
// emptyForceFlag, loggingOnlyMode, and basic marker API.
// Uses a real .dlt file from testdata/ and a minimal Project/settings stub.

#include <QTest>
#include <QApplication>
#include <QTreeWidget>
#include <QFile>

#include "tablemodel.h"
#include "project.h"
#include "qdltfile.h"
#include "qdltsettingsmanager.h"
#include "qdltpluginmanager.h"
#include "fieldnames.h"

// ---------------------------------------------------------------------------
// Minimal helper: sets up the bare minimum Project + settings that
// TableModel needs without requiring the full MainWindow stack.
// ---------------------------------------------------------------------------
static Project* makeMinimalProject()
{
    auto *proj = new Project();

    // TableModel only ever reads from proj->settings — not the tree widgets.
    // Point it at the singleton.
    proj->settings = QDltSettingsManager::getInstance();

    // Ensure sensible defaults so data() won't crash.
    proj->settings->showArguments   = 0;
    proj->settings->automaticTimeSettings = 1;
    proj->settings->showApIdDesc    = 0;
    proj->settings->showCtIdDesc    = 0;

    return proj;
}

// ---------------------------------------------------------------------------
// Test fixture
// ---------------------------------------------------------------------------
class TestTableModel : public QObject
{
    Q_OBJECT

private:
    QDltFile         *m_file    = nullptr;
    Project          *m_project = nullptr;
    QDltPluginManager m_plugins;
    TableModel       *m_model   = nullptr;

    // Returns the path to the test .dlt file provided at compile-time.
    static QString testFilePath()
    {
        return QString(TEST_DATA_DIR) + "/testfile.dlt";
    }

private slots:
    void initTestCase()
    {
        // Verify the test data file exists
        QVERIFY2(QFile::exists(testFilePath()),
                 qPrintable(QString("Test data file not found: %1").arg(testFilePath())));
    }

    void init()
    {
        m_file    = new QDltFile();
        m_project = makeMinimalProject();
        m_model   = new TableModel("", nullptr);

        m_model->qfile        = m_file;
        m_model->project      = m_project;
        m_model->pluginManager = &m_plugins;
    }

    void cleanup()
    {
        delete m_model;
        delete m_file;
        delete m_project;
        m_model   = nullptr;
        m_file    = nullptr;
        m_project = nullptr;
    }

    // ------------------------------------------------------------------
    // Empty model (no file loaded)
    // ------------------------------------------------------------------
    void test_rowCount_emptyFile()
    {
        // QDltFile default-constructed returns 0 filtered messages
        QCOMPARE(m_model->rowCount(), 0);
    }

    void test_columnCount_notZero()
    {
        // Must have at least the base column count (non-Arg columns)
        QVERIFY(m_model->columnCount() > 0);
    }

    void test_data_invalidIndex_returnsEmpty()
    {
        QModelIndex invalid;
        QVERIFY(!m_model->data(invalid, Qt::DisplayRole).isValid());
    }

    // ------------------------------------------------------------------
    // emptyForceFlag: forces row count to 0
    // ------------------------------------------------------------------
    void test_emptyForceFlag()
    {
        m_file->open(testFilePath());
        m_file->createIndex();
        m_file->createIndexFilter();

        // Without force flag, should have messages
        int normal = m_model->rowCount();
        QVERIFY(normal >= 0);

        m_model->setForceEmpty(true);
        QCOMPARE(m_model->rowCount(), 0);

        m_model->setForceEmpty(false);
        QCOMPARE(m_model->rowCount(), normal);
    }

    // ------------------------------------------------------------------
    // loggingOnlyMode: forces exactly 1 row
    // ------------------------------------------------------------------
    void test_loggingOnlyMode_rowCountIsOne()
    {
        m_model->setLoggingOnlyMode(true);
        QCOMPARE(m_model->rowCount(), 1);
    }

    void test_loggingOnlyMode_payloadText()
    {
        m_model->setLoggingOnlyMode(true);

        // Payload column of the single row must contain the logging-only message
        QModelIndex idx = m_model->index(0, FieldNames::Payload);
        QString text = m_model->data(idx, Qt::DisplayRole).toString();
        QVERIFY2(text.contains("Logging only"), qPrintable(text));
    }

    void test_loggingOnlyMode_otherColumnEmpty()
    {
        m_model->setLoggingOnlyMode(true);

        // Any column other than Payload should return empty variant
        QModelIndex idx = m_model->index(0, FieldNames::AppId);
        QVERIFY(!m_model->data(idx, Qt::DisplayRole).isValid());
    }

    // ------------------------------------------------------------------
    // Load a real DLT file and verify message count is positive
    // ------------------------------------------------------------------
    void test_rowCount_afterLoadingFile()
    {
        bool opened = m_file->open(testFilePath());
        QVERIFY2(opened, qPrintable(QString("Failed to open: %1").arg(testFilePath())));

        m_file->createIndex();
        m_file->createIndexFilter();

        int rows = m_model->rowCount();
        QVERIFY2(rows > 0,
                 qPrintable(QString("Expected >0 messages but got %1").arg(rows)));
    }

    // ------------------------------------------------------------------
    // headerData — sanity check it returns a non-empty string
    // ------------------------------------------------------------------
    void test_headerData_horizontal()
    {
        QVariant hdr = m_model->headerData(0, Qt::Horizontal, Qt::DisplayRole);
        QVERIFY(hdr.isValid());
        QVERIFY(!hdr.toString().isEmpty());
    }

    void test_headerData_vertical_returnsRowNumber()
    {
        QVariant hdr = m_model->headerData(0, Qt::Vertical, Qt::DisplayRole);
        QVERIFY(hdr.isValid());
    }

    // ------------------------------------------------------------------
    // setMarker — should not crash and must return 0 on empty model
    // ------------------------------------------------------------------
    void test_setMarker_emptyModel_doesNotCrash()
    {
        // No file loaded; marker on row -1 should return -1 gracefully
        int result = m_model->setMarker(-1, QColor(Qt::red));
        QCOMPARE(result, -1);
    }

    // ------------------------------------------------------------------
    // setManualMarker — called with empty list should not crash
    // ------------------------------------------------------------------
    void test_setManualMarker_emptyList_doesNotCrash()
    {
        QList<unsigned long int> empty;
        int result = m_model->setManualMarker(empty, QColor(Qt::blue));
        QCOMPARE(result, 0);
    }
};

QTEST_MAIN(TestTableModel)
#include "test_tablemodel.moc"
