#ifndef QDLTPLUGIN_H
#define QDLTPLUGIN_H

#include "plugininterface.h"

#include <QDir>

#include "export_rules.h"

class QDLTPluginInterface;
class QDLTPluginDecoderInterface;
class QDltPluginViewerInterface;
class QDltPluginControlInterface;
class QDltPluginCommandInterface;
class QTableView;

//! Access class to a DLT Plugin to decode, view and control DLT messages
/*!
  This class loads a DLT Viewer Plugin library and provides functions to access the plugin.
*/
class QDLT_EXPORT QDltPlugin
{
public:

    //! Constructor
    QDltPlugin();

    //! Status of the plugin
    typedef enum { ModeDisable=0, ModeEnable, ModeShow } Mode;

    //! Plugin configuration type, file or directory
    enum { TypeFile, TypeDirectory };

    //! Load the plugin by attaching the interfaces
    void loadPlugin(QObject *plugin);

    //! Get the name of the plugin
    QString getName();

    //! Get the plugin version string
    QString getPluginVersion();

    //! Get the plugin interface version string
    QString getPluginInterfaceVersion();

    //! Get the running status of the plugin
    /*!
      Plugin can be disabled, enabled and viewed.
    */
    int getMode();

    //! Set the running status of the plugin
    /*!
      Plugin can be disabled, enabled and viewed.
    */
    void setMode(QDltPlugin::Mode _mode);

    //! Get the complete filename of the plugin including path
    QString getFilename();

    //! Set the complete filename of the plugin including path and load the plugin configuration
    void setFilename(QString _filename);

    //! Decode plugin if enabled and messages matches the decoder
    /*!
      \return True if decoded, false if not decoded
    */
    bool decodeMsg(QDltMsg &msg, int triggeredByUser);

    //! Check if this is a decoder plugin
    /*!
      \return True if it is a decoder plugin
    */
    bool isDecoder();

    //! Check if this is a viewer plugin
    /*!
      \return True if it is a viewer plugin
    */
    bool isViewer();

    //! Check if this is a control plugin
    /*!
      \return True if it is a control plugin
    */
    bool isControl();

    //! Check if this is a command plugin
    /*!
      \return True if it is a command plugin
    */
    bool isCommand();

    // generic plugin interfaces
    QStringList infoConfig();
    QString error();
    bool loadConfig(QString filename);

    // viewer plugin interfaces
    QWidget* initViewer();
    void initFileStart(QDltFile *file);
    void initFileFinish();
    void initMsg(int index, QDltMsg &msg);
    void initMsgDecoded(int index, QDltMsg &msg);
    void updateFileStart();
    void updateMsg(int index, QDltMsg &msg);
    void updateMsgDecoded(int index, QDltMsg &msg);
    void updateFileFinish();
    void selectedIdxMsg(int index, QDltMsg &msg);
    void selectedIdxMsgDecoded(int index, QDltMsg &msg);
    void initMessageDecoder(QDltMessageDecoder* messageDecoder);
    void initMainTableView(QTableView* pTableView);
    void configurationChanged();

    // control plugin interfaces
    bool initControl(QDltControl *control);
    bool initConnections(QStringList list);
    bool controlMsg(int index, QDltMsg &msg);
    bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState, QString hostname);
    bool autoscrollStateChanged(bool enabled);

    // command plugin interfaces
    bool command(QString cmd,QStringList params);

private:

    //! The complete filename of the plugin including path
    QString filename;

    //! The running status of the plugin
    Mode mode;

    //! Link to all the plugin interfaces, when plugin loaded
    /*!
      Pointers are zero, if plugin does not support the interface.
    */
    QDLTPluginInterface *plugininterface;
    QDLTPluginDecoderInterface *plugindecoderinterface;
    QDltPluginViewerInterface  *pluginviewerinterface;
    QDltPluginControlInterface *plugincontrolinterface;
    QDltPluginCommandInterface *plugincommandinterface;

};

#endif // QDLTPLUGIN_H
