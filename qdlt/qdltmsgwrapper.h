#ifndef QDLTMSGWRAPPER_H
#define QDLTMSGWRAPPER_H

#include "dlt_common.h"

#include <cstdlib>
#include <type_traits>
#include <vector>
#include <cstring>

/**
* C++ wrapper around raw C-DltMsg structure with RAII semantics.
*/

class QDltMsgWrapper {
  public:
    QDltMsgWrapper();

    template<typename T>
    QDltMsgWrapper(T t, const std::vector<uint8_t>& c) : QDltMsgWrapper() {
        static_assert(std::is_pod<T>::value, "T must be a POD type");

        constexpr uint32_t sizeT = sizeof(T);
        const uint32_t sizeC = c.size();
        m_msg.datasize = sizeT + sizeof(sizeC) + sizeC;
        m_msg.databuffer = static_cast<uint8_t*>(malloc(m_msg.datasize));
        if (m_msg.databuffer) {
            uint32_t offset = 0;
            memcpy(m_msg.databuffer, &t, sizeT);
            offset += sizeT;
            memcpy(m_msg.databuffer + offset, &sizeC, sizeof(sizeC));
            offset += sizeof(sizeC);
            memcpy(m_msg.databuffer + offset, c.data(), c.size());

        }
    }

    ~QDltMsgWrapper();

    // TODO: replace with constructor
    // serialize packed plan C structure into underlying DltMessage::databuffer
    template<typename T>
    uint32_t asDataBuffer(T t) {
        static_assert(std::is_pod<T>::value, "T must be a POD type");

        constexpr uint32_t size = sizeof(T);
        m_msg.datasize = size;
        m_msg.databuffer = (static_cast<uint8_t*>(malloc(size)));
        if (m_msg.databuffer) {
            memcpy(m_msg.databuffer, &t, size);
        }
        return size;
    }

    // access to underlying raw structure to be used in legacy API
    DltMessage& getMessage();

  private:
    DltMessage m_msg;
};

#endif // QDLTMSGWRAPPER_H
