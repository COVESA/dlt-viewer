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
 * \file qdltfilterlist.cpp
 * For further information see http://www.genivi.org/.
 * @licence end@
 */

#include <QtDebug>

#include "qdlt.h"

extern "C"
{
#include "dlt_common.h"
}

QDltFilterList::QDltFilterList()
{

}

QDltFilterList::~QDltFilterList()
{

}

QDltFilterList& QDltFilterList::operator= (QDltFilterList const& _filterList)
{
    pfilter = _filterList.pfilter;
    nfilter = _filterList.nfilter;
    marker = _filterList.marker;

    return *this;
}

void QDltFilterList::clearFilter()
{
    pfilter.clear();
    nfilter.clear();
    marker.clear();
    qDebug() << "clearFilter: Clear filter";
}

void QDltFilterList::addPFilter(QDltFilter &_filter)
{
    pfilter.append(_filter);
    qDebug() << "addPFilter: Add Filter" << _filter.apid << _filter.ctid;
}

void QDltFilterList::addNFilter(QDltFilter &_filter)
{
    nfilter.append(_filter);
    qDebug() << "addNFilter: Add Filter" << _filter.apid << _filter.ctid;
}

void QDltFilterList::addMarker(QDltFilter &_filter)
{
    marker.append(_filter);
    qDebug() << "addMarker: Add Filter" << _filter.apid << _filter.ctid;
}

QColor QDltFilterList::checkMarker(QDltMsg &msg)
{
    QDltFilter filter;
    QColor color;

    for(int numfilter=0;numfilter<marker.size();numfilter++)
    {
        filter = marker[numfilter];

        if(filter.enableFilter){
            if(filter.match(msg))
            {
                color = filter.filterColour;
                break;
            }
        }
    }
    return color;
}

bool QDltFilterList::checkFilter(QDltMsg &msg)
{
    QDltFilter filter;
    bool found = false;
    bool filterActivated = false;

    /* If there are no positive filters, or all positive filters
     * are disabled, the default case is to show all messages. Only
     * negative filters will be applied */
    for(int numfilter=0;numfilter<pfilter.size();numfilter++)
    {
        filter = pfilter[numfilter];
        if(filter.enableFilter){
            filterActivated = true;
        }
    }

    if(filterActivated==false)
        found = true;
    else
        found = false;


    for(int numfilter=0;numfilter<pfilter.size();numfilter++)
    {
        filter = pfilter[numfilter];
        if(filter.enableFilter) {
            found = filter.match(msg);
            if (found)
              break;
        }
    }

    if (found || filterActivated==false ){
        //we need only to check for negative filters, if the message would be shown! If discarded anyway, there is no need to apply it.
        //if positive filter applied -> check for negative filters
        //if no positive filters are active or no one exists, we need also to filter negatively
        // if the message has been discarded by all positive filters before, we do not need to filter it away a second time

        for(int numfilter=0;numfilter<nfilter.size();numfilter++)
          {
            filter = nfilter[numfilter];
            if(filter.enableFilter){
                if (filter.match(msg))
                  {
                    // a negative filter has matched -> found = false
                    found = false;
                    break;
                  }
              }
          }
      }

    return found;
}
