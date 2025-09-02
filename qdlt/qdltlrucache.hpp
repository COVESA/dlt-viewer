#ifndef QDLTLRUCACHE_HPP
#define QDLTLRUCACHE_HPP

#include <unordered_map>
#include <list>
#include <stdexcept>

template<typename Key, typename Value>
class QDltLruCache {

    struct CacheEntry {
        Key key;
        Value value;
    };

    using CacheListIterator = typename std::list<CacheEntry>::iterator;
public:

    QDltLruCache(size_t capacity) :
        m_capacity(capacity) {
    }

    void put(const Key& key, const Value& value) {
        auto it = m_keyIteratorsMap.find(key);
        m_cacheItems.push_front(CacheEntry{key, value});
        if (it != m_keyIteratorsMap.end()) {
            m_cacheItems.erase(it->second);
            m_keyIteratorsMap.erase(it);
        }
        m_keyIteratorsMap[key] = m_cacheItems.begin();

        if (m_keyIteratorsMap.size() > m_capacity) {
            auto last = std::prev(m_cacheItems.end());
            m_keyIteratorsMap.erase(last->key);
            m_cacheItems.pop_back();
        }
    }

    const Value& get(const Key& key) {
        auto it = m_keyIteratorsMap.find(key);
        if (it == m_keyIteratorsMap.end()) {
            throw std::range_error("no such key in cache found");
        }

        m_cacheItems.splice(m_cacheItems.begin(), m_cacheItems, it->second);
        return it->second->value;
    }

    bool exists(const Key& key) const {
        return m_keyIteratorsMap.find(key) != m_keyIteratorsMap.end();
    }

private:
    std::list<CacheEntry> m_cacheItems;
    std::unordered_map<Key, CacheListIterator> m_keyIteratorsMap;
    const size_t m_capacity;
};

#endif // QDLTLRUCACHE_HPP
