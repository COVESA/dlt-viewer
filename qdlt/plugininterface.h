/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of COVESA Project Dlt Viewer.
 *
 * Contributions are licensed to the COVESA Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file plugininterface.h
 * For further information see http://www.covesa.global/.
 * @licence end@
 */

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include "qdltconnection.h"
#include "qdltcontrol.h"
#include "qdltfile.h"
#include "qdltmessagedecoder.h"
#include "qdltmsg.h"

#include <QString>
#include <QTableView>

#define PLUGIN_INTERFACE_VERSION "1.0.1"

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

    //! The version number of the plugin.
    /*!
      The plugin has to return a version number with formt X.Y.Z.
      X counts up in case of real heavy changes (API changes or purpose changes)
      Y counts up when the module is reworked internally, functions are added etc
      Z counts up whenever a bug is fixed
      Recommondation: define <plugin name>_PLUGIN_VERSION "X.Y.Z" in your plugin header file.
      \return The version number of the plugin
    */
    virtual QString pluginVersion() = 0;

    //! The used plugin interface version number of the plugin.
    /*!
      The plugin has to return a version number of the used plugin interface.
      The plugin interface provides for this purpose the PLUGIN_INTERFACE_VERSION definition.
      \return The version number of the used plugin interface - PLUGIN_INTERFACE_VERSION in plugininterface.h
    */
    virtual QString pluginInterfaceVersion() = 0;

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
      \return True if the configuration is loaded successful. False if there was any error or function was not supported.
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
      \return True if the configuration is stored successful. False if there was any error or function was not supported.
    */
    virtual bool saveConfig(QString filename) = 0;

    //! Providing detaild information about the loaded configuration.
    /*!
      The plugin can provide a detailed list of the loaded configuration.
      This is very useful to check, if the configuration is loaded successfully.
      \return The detailed information about the loaded configuration. One string per configuration element should be used.
    */
    virtual QStringList infoConfig() = 0;


};

Q_DECLARE_INTERFACE(QDLTPluginInterface,
                    "org.genivi.DLT.Plugin.DLTPluginInterface/1.0")

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
      \param triggeredByUser Reason for this method call was a user interaction with the GUI, e.g. clicked on "Export to ASCII" in the menu. 0 = not triggered by user, 1 = triggered by user
      \return True if the message is handled by the plugin. False if the message is not handled by the plugin.
    */
    virtual bool isMsg(QDltMsg &msg, int triggeredByUser) = 0;

    //! Decode the message and provide back the decoded message.
    /*!
      The plugin converts the DLT message.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param msg The current DLT message and the decoded message information.
      \param triggeredByUser Reason for this method call was a user interaction with the GUI, e.g. clicked on "Export to ASCII" in the menu. 0 = not triggered by user, 1 = triggered by user
      \return True if the message is converted by the plugin. False if the conversion fails.
    */
    virtual bool decodeMsg(QDltMsg &msg, int triggeredByUser) = 0;

};

Q_DECLARE_INTERFACE(QDLTPluginDecoderInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginDecoderInterface/1.0")

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
      \return The form of the plugin.
    */
    virtual QWidget* initViewer() = 0;


    //! A new log file is opened by the DLT Viewer.
    /*! This function is called by the the viewer every time a new log file is opened by the viewer
      or a new log file is created and before all messages were processed with initMsg and initMsgDecoded.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param file The DLT log file handle.
    */
    virtual void initFileStart(QDltFile *file) = 0;

    //! A new undecoded DLT message is processed after a new log file is opened by the DLT Viewer
    /*! After a new log file is opened this function is called by the viewer every time
        a new undecoded message is processed.

      Important note! Be aware, that basic functionality may call this function from a separate worker-thread.
      It is a responsibility of plugin's developer to add synchronization of his internal state within this call.
      Best practice is to send a queued signal to the parent thread of the plugin, or to use QMetsObject::invokeMethod.

      \sa QDLTPluginInterface::error()
      \param index The current DLT message index
      \param msg The current undecoded DLT message
    */
    virtual void initMsg(int index, QDltMsg &msg ) = 0;

    //! A new decoded DLT message is processed after a new log file is opened by the DLT Viewer
    /*! After a new log file is opened this function is called by the viewer every time
        a new decoded message is processed.

      Important note! Be aware, that basic functionality may call this function from a separate worker-thread.
      It is a responsibility of plugin's developer to add synchronization of his internal state within this call.
      Best practice is to send a queued signal to the parent thread of the plugin, or to use QMetsObject::invokeMethod.

      \sa QDLTPluginInterface::error()
      \param index The current DLT message index
      \param msg The current decoded DLT message
    */
    virtual void initMsgDecoded(int index, QDltMsg &msg ) = 0;

    //! A new log file was opened by the DLT Viewer.
    /*! This function is called by the the viewer every time a new log file was opened by the viewer
      or a new log file is created and all messages were processed with initMsg and initMsgDecoded.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
    */
    virtual void initFileFinish() = 0;


    //! A new message was received before updateMsg and updateMsgDecoded.
    /*! This function is called by the viewer every time a new message was received
        before processing the message with updateMsg and updateMsgDecoded.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
    */
    virtual void updateFileStart( ) = 0;


    //! New message were added to the log file.
    /*! This function is called when new log entries are added to the log file.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param index The current DLT message index
      \param msg The current undecoded DLT message
    */
    virtual void updateMsg(int index, QDltMsg &msg ) = 0;


    //! New message were added to the log file.
    /*! This function is called when new log entries are added to the log file.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param index The current DLT message index
      \param msg The current decoded DLT message
    */
    virtual void updateMsgDecoded(int index, QDltMsg &msg ) = 0;


    //! A new message was received after updateMsg and updateMsgDecoded.
    /*! This function is called by the viewer every time a new message was received
        after processing the message with updateMsg and updateMsgDecoded.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
    */
    virtual void updateFileFinish( ) = 0;


    //! An undecoded log message was selected to show more detailed information.
    /*!
      An undecoded log message was selected. The viewer plugin can show more detailed information about this plugin.
      \param index The current DLT message index
      \param msg The current undecoded DLT message
    */
    virtual void selectedIdxMsg(int index, QDltMsg &msg) = 0;


    //! A decoded log message was selected to show more detailed information.
    /*!
      A decoded log message was selected. The viewer plugin can show more detailed information about this plugin.
      \param index The current DLT message index
      \param msg The current decoded DLT message
    */
    virtual void selectedIdxMsgDecoded(int index, QDltMsg &msg) = 0;

};

Q_DECLARE_INTERFACE(QDltPluginViewerInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginViewerInterface/1.2")

//! Extended DLT Control Plugin Interface used by control plugins.
/*!
  This is an extended DLT Plugin Interface.
  This interface must be used by control plugins.
  The control plugin interface can send control requests to the DLT daemon and receives control responses from the DLT daemon.
  The plugin gets informed about the available connections to DLT daemons.
*/
class QDltPluginControlInterface
{
public:

    //! A control interface is provided to the plugin.
    /*!
      This function is called once during initialisation of the plugin.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param control The control interface.
      \return True if everything went ok. False if there was an error.
    */
    virtual bool initControl(QDltControl *control) = 0;

    //! The connection table is initialised or changed.
    /*!
      This function is called by the the user changes the connection table to available targets.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param list The DLT log file handle.
      \return True if everything went ok. False if there was an error.
    */
    virtual bool initConnections(QStringList list) = 0;

    //! control message received.
    /*!
      A control message was received.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param index The connection where the ctrl message was received.
      \param msg The received control DLT message response.
      \return True if everything went ok. False if there was an error.
    */
    virtual bool controlMsg(int index, QDltMsg &msg) = 0;

    //! TCP connection state changed
    /*!
      The TCP connection state changed for a specific ECU item.

      \param ecuItem The ecuItem changed the connection state
      \param connectionState The new connection state of the ecuItem
      \param hostname The hostname of the connected ECU, if it is a TCP connection, else empty
      \return True if everything went ok. False if there was an error.
    */
    virtual bool stateChanged(int index, QDltConnection::QDltConnectionState connectionState,QString hostname) = 0;

    //! Autoscroll state changed
    /*!
      The Autoscroll feature of the viewer was enabled or disabled

      \param enabled True if Autoscroll is enabled, false if not
      \return True if everything went ok. False if there was an error.
    */
    virtual bool autoscrollStateChanged(bool enabled) = 0;

    //! A message decoder is injected into the plugin
    /*!
      This function is called by the the viewer once on creation of the plugin
      Motivation - plugin might need to be able to decode a messages, if it directly uses the QDltFile.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param messageDecoder - pointer to a message decoder, which allows a plugin to decode instances of QDltMsg.
    */
    virtual void initMessageDecoder(QDltMessageDecoder* /*messageDecoder*/) = 0;

    //! A main message's table is injected via this call
    /*!
      This function is called by the the viewer once on creation of the plugin
      Motivation - plugin might need to be able to scroll a main table view to a certain selected message.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
      \param pTableView - pointer to a table view, which contains all loaded messages.
    */
    virtual void initMainTableView(QTableView* pTableView) = 0;

    //! Notifies the plugin, that used configuration has changed.
    /*!
      Motivation - plugin might need to discard previous results, or to restart the analysis in case of a change in configuration.
      E.g. another set of filters was applied, a sort by time was applied, or filtering was turned off completely.
      Errors should be reported by providing an error message.
      \sa QDLTPluginInterface::error()
    */
    virtual void configurationChanged() = 0;
};

Q_DECLARE_INTERFACE(QDltPluginControlInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginControlInterface/1.2")

//! DLT Command Plugin Interface
/*!
 This interface can be used to create plugins which execute from the command line of
 DLT-viewer. The API is very simple: Implement command() function, and return either success
 or failure.  You can return further information about the error with error()
*/
class QDltPluginCommandInterface
{
public:

    //! A command is executed in the plugin
    /*!
      This function is called from DLT-viewer, when the -e command line
      parameter is used with this plugin.
      \sa QDLTPluginInterface::error()
      \param command The requested command. First parameter from command line
      \param params List of parameters to the command. Rest of the command line parameters.
      \return True if everything went ok. False if there was an error.
    */
    virtual bool command(QString command, QList<QString> params) = 0;
};

Q_DECLARE_INTERFACE(QDltPluginCommandInterface,
                    "org.genivi.DLT.Plugin.DLTViewerPluginCommandInterface/1.1")

#endif // PLUGININTERFACE_H
