#ifndef REGEX_SEARCH_REPLACE_H
#define REGEX_SEARCH_REPLACE_H

#include "project.h"
#include <regex>
#include <stdlib.h>


static void apply_regex_string(QString &data, const QString& regex_search, const QString& regex_replace)
{
    std::regex regex(regex_search.toStdString());

    std::string payload_str = std::regex_replace(data.toStdString(), regex, regex_replace.toStdString());

    data = QString::fromStdString(payload_str);
}

#endif // REGEX_SEARCH_REPLACE_H
