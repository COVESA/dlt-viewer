#include <gtest/gtest.h>

#include <qdltoptmanager.h>

#include <QCommandLineParser>


QString logMessageSink;

void messageHandler(QtMsgType type, const QMessageLogContext &,
                    const QString &msg) {
    if (type == QtDebugMsg) {
        logMessageSink.append(msg);
    }
}

class OptManagerTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        m_manager = QDltOptManager::getInstance();

        qInstallMessageHandler(messageHandler);
    }

    void SetUp() override {
        m_manager->reset();
    }

    void TearDown() override {
        logMessageSink.clear();
    }

    static QDltOptManager* m_manager;
};

QDltOptManager* OptManagerTest::m_manager = nullptr;

TEST_F(OptManagerTest, txtConversion) {
    auto args = QStringList() << "executable" << "-t" << "-c" << "output.txt" << "input.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.txt");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, txtConversionSilentUtf8Mode) {
    auto args = QStringList() << "executable" << "-t" << "-s" << "-u" << "-c" << "output.txt" << "input.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_TRUE(m_manager->issilentMode());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.txt");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_EQ(m_manager->get_convertionmode(), e_UTF8);
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, txtConversionSilentAsciiMode) {
    auto args = QStringList() << "executable" << "-t" << "-s" << "-d" << "-c" << "output.txt" << "input.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_TRUE(m_manager->issilentMode());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.txt");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_EQ(m_manager->get_convertionmode(), e_DLT);
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, csvConversionSilentMode) {
    auto args = QStringList() << "executable" << "-t" << "-s" << "-csv" << "-c" << "output.csv" << "input.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_TRUE(m_manager->issilentMode());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.csv");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_EQ(m_manager->get_convertionmode(), e_CSV);
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, txtConversionSilentDdlMode) {
    auto args = QStringList() << "executable" << "-t" << "-s" << "decoded.dlp" << "-dd" << "-c" << "output.dlt" << "input.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_TRUE(m_manager->issilentMode());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.dlt");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_EQ(m_manager->get_convertionmode(), e_DDLT);
    EXPECT_TRUE(m_manager->isCommandlineMode());
    EXPECT_EQ(m_manager->getProjectFile(), "decoded.dlp");
}

TEST_F(OptManagerTest, pluginPostCommands) {
    auto args = QStringList() << "executable"
                              << "-p"
                              << "export.dlp"
                              << "-e"
                              << "\"Filetransfer Plugin|export|ftransferdir\""
                              << "input.dlt";
    m_manager->parse(args);

    EXPECT_EQ(m_manager->getProjectFile(), "export.dlp");
    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_TRUE(m_manager->getPostPluginCommands().contains("\"Filetransfer Plugin|export|ftransferdir\""));
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, pluginPreCommands) {
    auto args = QStringList() << "executable"
                              << "-b"
                              << "\"Filetransfer Plugin|export|ftransferdir\""
                              << "input.dlt";
    m_manager->parse(args);

    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_TRUE(m_manager->getPrePluginCommands().contains("\"Filetransfer Plugin|export|ftransferdir\""));
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, multipleLogFiles) {
    auto args = QStringList() << "executable" << "input1.dlt" << "input2.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->getLogFiles().contains("input1.dlt"));
    EXPECT_TRUE(m_manager->getLogFiles().contains("input2.dlt"));
}

TEST_F(OptManagerTest, pcapFile) {
    auto args = QStringList() << "executable" << "-t" << "-c" << "output.txt" << "input.pcap";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.txt");
    EXPECT_TRUE(m_manager->getPcapFiles().contains("input.pcap"));
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, mf4Files) {
    auto args = QStringList() << "executable" << "-t" << "-c" << "output.txt" << "input1.mf4" << "input2.mf4";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_EQ(m_manager->getConvertDestFile(), "output.txt");
    EXPECT_TRUE(m_manager->getMf4Files().contains("input1.mf4"));
    EXPECT_TRUE(m_manager->getMf4Files().contains("input2.mf4"));
    EXPECT_TRUE(m_manager->isCommandlineMode());
}

TEST_F(OptManagerTest, filterFile) {
    auto args = QStringList() << "executable"
                              << "input.dlt"
                              << "filter.dlf"
                              << "-c"
                              << "out.dlt"
                              << "-d"
                              << "-s"
                              << "-t";

    m_manager->parse(args);

    EXPECT_TRUE(m_manager->getLogFiles().contains("input.dlt"));
    EXPECT_TRUE(m_manager->getFilterFiles().contains("filter.dlf"));
    EXPECT_EQ(m_manager->getConvertDestFile(), "out.dlt");
    EXPECT_TRUE(m_manager->isCommandlineMode());
    EXPECT_TRUE(m_manager->isTerminate());
    EXPECT_TRUE(m_manager->issilentMode());
}

TEST_F(OptManagerTest, version) {
    // impossible to check just version because there is a call to exit(0) in the qdltoptmanager
    // but any output will be enough to check the version call because it is always printed
    auto args = QStringList() << "executable" << "some.dlt";

    m_manager->parse(args);

    EXPECT_TRUE(logMessageSink.contains("Executable Name:"));
    EXPECT_TRUE(logMessageSink.contains("Build time:"));
    EXPECT_TRUE(logMessageSink.contains("Version:"));
}
