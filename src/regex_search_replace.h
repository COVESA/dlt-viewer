#ifndef REGEX_SEARCH_REPLACE_H
#define REGEX_SEARCH_REPLACE_H

#include "project.h"
#include <regex>

static void apply_regex(QDltArgument &argument, const QString& regex_search, const QString& regex_replace)
{
    QByteArray argument_data_to_decode = argument.getData();
    std::string payload_str = argument_data_to_decode.toStdString();

    std::regex regex(regex_search.toStdString());
    payload_str = std::regex_replace(payload_str, regex, regex_replace.toStdString());

    argument.setData(QByteArray::fromStdString(payload_str));
}

#endif // REGEX_SEARCH_REPLACE_H
