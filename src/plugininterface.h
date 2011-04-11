#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QString>
#include "dlt.h"
#include "qdlt.h"

//! Standard DLT Viewer Plugin Interface.
/*!
  This is the standard DLT Viewer Plugin Interface.
  This interface must be inherited by each DLT Viewer plugin.
*/

class QDLTPluginInterface
{
public:

    //! The name and identifier of the plugin.
    /*!
      The plugin must provide the name of the plugin.
      This name is used for identifie the plugin in the project configuration.
      \return The name of the plugin
    */
    virtual QString name() = 0;

    //! The description of the plugin.
    /*!
      The plugin can provide a more detailed description of the plugin.
      \return The description of the plugin
    */
    virtual QString description() = 0;

    //! The error message of the last plugin interface call.
    /*!
      The plugin can provide a error message of the last failed function call.
      \return Error message.
    */
    virtual QString error() = 0;

    //! Loading the configuration of the plugin.
    /*!
      The plugin can use a configuration stored in a file.
      The configuration can be a single file or a directory containing several files.
      Example of a configuration is an XML description file of the DLT non-verbose mode.
      Errors should be reported by providing error message.
      \sa error()
      \param filename This can be a filename or a directory name.
      \return True if the configuration is loaded succesful. False if there was any error or function was not supported.
    */
    virtual bool loadConfig(QString filename) = 0;

    //! Storing the configuration of the plugin.
    /*!
      The plugin can use a configuration stored in a file.
      The configuration can be a single file or a directory containing several files.
      Example of a configuration is an XML description file of the DLT non-verbose mode.
      Errors should be reported by providing error message.
      \sa error()
      \param filename This can be a filename or a directory name.
      \return True if the configuration is stored succesful. False if there was any error or function was not supported.
    */
    virtual bool saveConfig(QString filename) = 0;

    //! Providing detaild information about the loaded configuration.
    /*!
      The plugin can provide a detailed list of the loaded configuration.
      This is very useful to check, if the configuration is loaded succesfully.
      \return The detailed information about the loaded configuration. One string per configuration element should be used.
    */
    virtual QStringList infoConfig() = 0;


};

Q_DECLARE_INTERFACE(QDLTPluginInterface,
                    "org.genivi.DLT.Plugin.DLTPluginInterface/1.0");

//! Extended DLT Viewer Plugin Interface used by decoder plugins.
/*!
  This is an extended DLT Plugin Interface.
  This interface must be used by decoder plugins.
  DLT messages which are displayed are checked by the plugin, if they are valid and then decoded by the plugin.
*/
class QDLTPluginDecoderInterface
{
public:
    //! Check if the DLT message is handled by the plugin.
    /*!
      The plugin checks, if the DLT message is handled by the plugin.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param msg The current DLT message.
      \return True if the message is handled by the plugin. False if the message is not handled by the plugin.
    */
    virtual bool isMsg(QDltMsg &msg) = 0;

    //! Decode the message and provide back the decoded message.
    /*!
      The plugin converts the DLT message.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param msg The current DLT message and the decoded message information.
      \return True if the message is converted by the plugin. False if the conversion fails.
    */
    virtual bool decodeMsg(QDltMsg &msg) = 0;

};

Q_DECLARE_INTERFACE(QDLTPluginDecoderInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginDecoderInterface/1.0");

//! Extended DLT Viewer Plugin Interface used by viewer plugins.
/*!
  This is an extended DLT Plugin Interface.
  This interface must be used by viewer plugins.
  The viewer plugin gets full access to the loaded DLT file.
*/
class QDltPluginViewerInterface
{
public:

    //! Initialise the viewer window.
    /*!
      The plugin manager provides the handle of the parent window.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param parent The parent window for the viewer plugin window.
      \return True if everything went ok. False if there was an error.
    */
    virtual QWidget* initViewer() = 0;

    //! A new log file is opened by the DLT Viewer.
    /*!
      This function is called by the the viewer everytime a new log file is opened by the viewer
      or a new log file is created.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param file The DLT log file handle.
      \return True if everything went ok. False if there was an error.
    */
    virtual bool initFile(QDltFile *file) = 0;

    //! New DLT messages are added to the log file.
    /*!
      This function is called when new log entries are added to the log file.
    */
    virtual void updateFile() = 0;

    //! A log message was selected to show more detailed information.
    /*!
      A log message was selected. The viewer plugin can show more detailed information about this plugin.
      \param index The current DLT message and the decoded message information.
    */
    virtual void selectedIdxMsg(int index) = 0;

};

Q_DECLARE_INTERFACE(QDltPluginViewerInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginViewerInterface/1.0");

#endif // PLUGININTERFACE_H
