/**
 * @licence app begin@
 * Copyright (C) 2011-2012  BMW AG
 *
 * This file is part of GENIVI Project Dlt Viewer.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \author Alexander Wenzel <alexander.aw.wenzel@bmw.de> 2011-2012
 *
 * \file qdlt.h
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#ifndef QDLT_FILE_H
#define QDLT_FILE_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QDateTime>
#include <QColor>
#include <QMutex>
#include <time.h>

#include "export_rules.h"

class QDLT_EXPORT QDltFileItem
{
public:
    //! DLT log file.
    QFile infile;

    //! Index of all DLT messages.
    /*!
      Index contains positions of beginning of DLT messages in DLT log file.
    */
    QList<unsigned long> indexAll;

};

//! Access to a DLT log file.
/*!
  This class provide access to DLT log file.
  This class is currently not multithread save.
*/
class QDLT_EXPORT QDltFile : public QDlt
{
public:
    //! The constructor.
    /*!
    */
    QDltFile();

    //! The destructor.
    /*!
      \return The name of the plugin
    */
    ~QDltFile();

    void clear();

    int getNumberOfFiles();

    //! Get the number of DLT message in the DLT log file.
    /*!
      \return the number of all DLT messages in the currently opened DLT file.
    */
    int size();

    //! Get the file size of the DLT log file.
    /*!
      \return the size of the DLT file.
    */
    unsigned long fileSize();

    //! Get the number of filtered DLT message in the DLT log file.
    /*!
      \return the number of filtered DLT messages in the currently opened DLT file.
    */
    int sizeFilter();

    //! Open a DLT log file.
    /*!
      The DLT log file is parsed and a index of all DLT log messages is created.
      \param filename The DLT filename.
      \return true if the file is successfully opened with no error, false if an error occured.
    */
    bool open(QString _filename,bool append = false);

    //! Close teh currently opened DLT log file.
    /*!
    */
    void close();

    //! Sets the internal index of all DLT messages.
    /*!
      \param New index list of all DLT messages
    */
    void setDltIndex(QList<unsigned long> &_indexAll,int num = 0);

    //! Clears the internal index of all DLT messages.
    /*!
    */
    void clearIndex();

    //! Create an internal index of all DLT messages of the currently opened DLT log file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool createIndex();

    //! Update the index of the currently opened DLT log file by checking if new DLT messages were added to the file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool updateIndex();

    //! Create an internal index of all filtered DLT messages of the currently opened DLT log file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool createIndexFilter();

    //! Update the filtered index of the currently opened DLT log file by checking if new DLT messages were added to the file.
    /*!
      \return true if the operation was successful, false if an error occured.
    */
    bool updateIndexFilter();

    //! Get one message of the DLT log file.
    /*!
      This function retrieves on DLT message of the log file
      \param index The number of the DLT message in the DLT file starting from zero.
      \param msg The message which contains the DLT message after the function returns.
      \return true if the message is valid, false if an error occured.
    */
    bool getMsg(int index,QDltMsg &msg);

    //! Get one DLT message of the DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number DLT messages in the file
      \return Byte array containing the complete DLT message.
    */
    QByteArray getMsg(int index);

    //! Get one DLT message of the filtered DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number of DLT messages in the file
      \return Byte array containing the complete DLT message.
    */
    QByteArray getMsgFilter(int index);

    //! Get the poition in the log file of the filtered DLT log file selected by index
    /*!
      \param index position of the DLT message in the log file up to the number of DLT messages in the file
      \return real position in log file, -1 if invalid.
    */
    int getMsgFilterPos(int index);

    //! Delete all filters and markers.
    /*!
      This includes all positive and negative filters and markers.
    */
    void clearFilter();

    //! Get current filter list
    /*!
      \return filter list.
    */
    QDltFilterList getFilterList();

    //! Set current filter list
    /*!
      \param filter list.
    */
    void setFilterList(QDltFilterList &_filterList);

    //! Add a filter to the filter list.
    /*!
      \param filter the filter configuration
    */
    void addFilter(QDltFilter *filter);

    //! Update presorted filter list.
    /*!
    */
    void updateSortedFilter();

    //! Get the status of the filter.
    /*!
      \return true if filtering is enabled, fals if filtering is disabled
    */
    bool isFilter();

    //! Enable or disable filtering.
    /*!
      \return state true if filtering is enabled, false if filtering is disabled
    */
    void enableFilter(bool state);

    //! Check if message matches the filter.
    /*!
      \param msg The message to be checked
      \return true if message wil be displayed, false if message will be filtered out
    */
    bool checkFilter(QDltMsg &msg);

    //! Clear the filter index.
    /*!
    */
    void clearFilterIndex();

    //! Add filter to the filter index.
    /*!
      \param index The position of the message in the allIndex to be added
    */
    void addFilterIndex (int index);

    //! Check if message will be marked.
    /*!
      Colours used are:
      1 = green
      2 = red
      3 = yellow
      4 = blue
      5 = light grey
      6 = dark grey
      \param msg The messages to be marked
      \return 0 if message will not be marked, colour if message will be marked
    */
    QColor checkMarker(QDltMsg &msg);

    //! Get file name of the underlying file object
    /*!
     * \return File name
     **/
    QString getFileName(int num = 0);

    //! Get Index of all DLT messages matching filter
    /*!
     * \return List of file positions
     **/
    QList<unsigned long> getIndexFilter();

    //! Set Index of all DLT messages matching filter
    /*!
     * \param _indexFilter List of file positions
     **/
    void setIndexFilter(QList<unsigned long> &_indexFilter);

protected:

private:
    //! Mutex to lock critical path for infile
    QMutex mutexQDlt;

    //!all files including indexes
    QList<QDltFileItem*> files;

    //! Index of all DLT messages matching filter.
    /*!
      Index contains positions of DLT messages in indexAll.
    */
    QList<unsigned long> indexFilter;

    //! This contains the list of filters.
    QDltFilterList filterList;

    //! Enabling filter.
    /*!
      true filtering is enabled.
      false filtering is disabled.
    */
    bool filterFlag;
};


#endif // QDLT_FILE_H
