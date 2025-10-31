#include "qdltmsgwrapper.h"
#include <stdlib.h>

QDltMsgWrapper::QDltMsgWrapper() { dlt_message_init(&m_msg, 0); }

QDltMsgWrapper::~QDltMsgWrapper() { dlt_message_free(&m_msg, 0); }

DltMessage& QDltMsgWrapper::getMessage() { return m_msg; }

