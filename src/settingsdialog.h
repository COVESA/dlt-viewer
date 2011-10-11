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

    int fontSize;
    int showIndex;
    int showTime;
    int showTimestamp;
    int showCount;
    int showEcuId;
    int showApId;
    int showApIdDesc;
    int showCtId;
    int showCtIdDesc;
    int showType;
    int showSubtype;
    int showMode;
    int showNoar;
    int showPayload;

    QString workingDirectory;

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
    void on_groupBoxAppId_clicked(bool checked);
    void on_groupBoxConId_clicked(bool checked);
    void on_tooButtonPluginsPath_clicked();
    void on_toolButtonDefaultProjectFile_clicked();
    void on_toolButtonDefaultLogFile_clicked();
};

#endif // SETTINGSDIALOG_H
