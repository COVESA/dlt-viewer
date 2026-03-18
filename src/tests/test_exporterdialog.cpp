// SPDX-License-Identifier: MPL-2.0
// QTest coverage for ExporterDialog — format and selection setter/getter
// round-trips, range handling, and failure cases.

#include <QTest>

#include "exporterdialog.h"
#include "qdltexporter.h"

class TestExporterDialog : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------
    // Format round-trips — one test per enum value
    // ------------------------------------------------------------------
    void test_format_dlt()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatDlt);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatDlt);
    }

    void test_format_ascii()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatAscii);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatAscii);
    }

    void test_format_utf8()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatUTF8);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatUTF8);
    }

    void test_format_csv()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatCsv);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatCsv);
    }

    void test_format_dltDecoded()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatDltDecoded);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatDltDecoded);
    }

    // ------------------------------------------------------------------
    // Selection round-trips
    // ------------------------------------------------------------------
    void test_selection_all()
    {
        ExporterDialog dlg;
        dlg.setSelection(QDltExporter::SelectionAll);
        QCOMPARE(dlg.getSelection(), QDltExporter::SelectionAll);
    }

    void test_selection_filtered()
    {
        ExporterDialog dlg;
        dlg.setSelection(QDltExporter::SelectionFiltered);
        QCOMPARE(dlg.getSelection(), QDltExporter::SelectionFiltered);
    }

    void test_selection_selected()
    {
        ExporterDialog dlg;
        dlg.setSelection(QDltExporter::SelectionSelected);
        QCOMPARE(dlg.getSelection(), QDltExporter::SelectionSelected);
    }

    // ------------------------------------------------------------------
    // Range round-trips
    // ------------------------------------------------------------------
    void test_range_basic()
    {
        ExporterDialog dlg;
        dlg.setRange(10, 200);
        unsigned long start = 0, stop = 0;
        dlg.getRange(&start, &stop);
        QCOMPARE(start, 10ul);
        QCOMPARE(stop, 200ul);
    }

    void test_range_zero()
    {
        ExporterDialog dlg;
        dlg.setRange(0, 0);
        unsigned long start = 1, stop = 1;
        dlg.getRange(&start, &stop);
        QCOMPARE(start, 0ul);
        QCOMPARE(stop, 0ul);
    }

    void test_range_large()
    {
        ExporterDialog dlg;
        dlg.setRange(0, 1000000);
        unsigned long start = 0, stop = 0;
        dlg.getRange(&start, &stop);
        QCOMPARE(stop, 1000000ul);
    }

    void test_range_start_equals_stop()
    {
        ExporterDialog dlg;
        dlg.setRange(42, 42);
        unsigned long start = 0, stop = 0;
        dlg.getRange(&start, &stop);
        QCOMPARE(start, 42ul);
        QCOMPARE(stop, 42ul);
    }

    // ------------------------------------------------------------------
    // Switching format multiple times — last value must win
    // ------------------------------------------------------------------
    void test_format_overwrite()
    {
        ExporterDialog dlg;
        dlg.setFormat(QDltExporter::FormatAscii);
        dlg.setFormat(QDltExporter::FormatCsv);
        QCOMPARE(dlg.getFormat(), QDltExporter::FormatCsv);
    }

    // ------------------------------------------------------------------
    // Default format when no setter called should not crash getFormat()
    // ------------------------------------------------------------------
    void test_default_getFormat_noCrash()
    {
        ExporterDialog dlg;
        // Should return some valid enum value without crashing
        QDltExporter::DltExportFormat fmt = dlg.getFormat();
        // Just verify it returns one of the known enum values
        QVERIFY(fmt == QDltExporter::FormatDlt    ||
                fmt == QDltExporter::FormatAscii  ||
                fmt == QDltExporter::FormatUTF8   ||
                fmt == QDltExporter::FormatCsv    ||
                fmt == QDltExporter::FormatDltDecoded);
    }
};

QTEST_MAIN(TestExporterDialog)
#include "test_exporterdialog.moc"
