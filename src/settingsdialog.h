#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    int defaultLogFile;
    QString defaultLogFileName;
    int defaultProjectFile;
    QString defaultProjectFileName;
    int pluginsPath;
    QString pluginsPathName;
    int autoConnect;
    int autoScroll;

    int showHeader;
    int showPayloadAscii;
    int showPayloadHex;
    int showPayloadMixed;

    int showIndex;
    int showTime;
    int showTimestamp;
    int showCount;
    int showEcuId;
    int showApId;
    int showCtId;
    int showType;
    int showSubtype;
    int showMode;
    int showNoar;
    int showPayload;

    int writeControl;

    void writeDlg();
    void readDlg();

    void writeSettings();
    void readSettings();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SettingsDialog *ui;

private slots:
    void on_tooButtonPluginsPath_clicked();
    void on_toolButtonDefaultProjectFile_clicked();
    void on_toolButtonDefaultLogFile_clicked();
};

#endif // SETTINGSDIALOG_H
