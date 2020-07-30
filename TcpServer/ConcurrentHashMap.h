#ifndef CONCURRENT_HASH_MAP
#define CONCURRENT_HASH_MAP


#include <functional>
#include <map>
#include <mutex>
#include <vector>
#include <algorithm>
#include <iostream>

template<typename K, typename V, typename Hash = std::hash<K>>
class ConcurrentHashMap
{
public:
    ConcurrentHashMap(int bucketNumber)
    : table_(bucketNumber)
    {
    }

    
    bool for_one(const K &key, V &p)
    {
        std::cout << "key: " << key << " code : " <<  hashcode(key) << std::endl;
       return table_[hashcode(key)].for_one(key, p);
    }

    void for_each(V &p)
    {
        for (auto &bucket : table_)
        {
            bucket.for_each(p);
        }
    }

    void insert(const K &key, V &&value)
    {
        table_[hashcode(key)].insert(key, std::move(value));
    }

    void put(const K &key, V &&value)
    {
        std::cout << "key: " << key << " code : " << hashcode(key) << std::endl;
        table_[hashcode(key)].put(key, std::move(value));
    }

    int table_length() {
        return table_.size();
    }
    
    std::vector<K> getkeys(int index) {
        return table_[index].getkeys();
    }

    void erase(const K &key)
    {
        table_[hashcode(key)].erase(key);
    }

    std::size_t size() const
    {
        std::size_t size = 0;
        for (auto &bucket : table_)
        {
            size += bucket.size();
        }
        return size;
    }

    std::size_t count(const K &key) const
    {
        return table_[hashcode(key)].count(key);
    }

private:
    
    class Bucket
    {
    public:
        void insert(const K &key, V &&value)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            item_.emplace(key, std::move(value));
        }

        void put(const K &key, V &&value)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            item_.erase(key);
            item_.emplace(key, std::move(value));
        }

        void erase(const K &key)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            item_.erase(key);
        }

       
        bool for_one(const K &key, V &p)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const ConstIterator it = item_.find(key);
            return it == item_.end() ? false : (p =it->second, true);
        }

       
        void for_each(V &p)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            std::for_each(item_.begin(), item_.end(), p);
        }

        std::size_t size() const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return item_.size();
        }

        std::size_t count(const K &key) const
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return item_.count(key);
        }

        std::vector<K> getkeys(){
            std::lock_guard<std::mutex> lock(mutex_);
            std::vector<K> result;
            for(auto it = item_.begin(); it != item_.end(); it++) {
                result.push_back(it -> first);
            }
            return result;
        }

    private:
        using Item = std::map<K, V>;
        using ConstIterator = typename Item::const_iterator;
        Item item_;
        mutable std::mutex mutex_;
    };

    inline std::size_t hashcode(const K &key)
    {
        return hash_(key) % table_.size();
    }

    std::vector<Bucket> table_;
    Hash hash_;
};

#endif
