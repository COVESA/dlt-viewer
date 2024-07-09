#ifndef DLTMESSAGEMATCHER_H
#define DLTMESSAGEMATCHER_H

class QDltMsg;

class DltMessageMatcher
{
public:
    DltMessageMatcher();

    bool match(const QDltMsg& message) const;
};

#endif // DLTMESSAGEMATCHER_H
