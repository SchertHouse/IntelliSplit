#pragma once
#include <vector>
#include <mutex>
#include <algorithm>

template<typename T>
class ThreadSafeVector {
public:
    void push_back(const T& item) {
        std::lock_guard<std::mutex> lock(mutex_);
        vec_.push_back(item);
    }

    std::vector<T> snapshot() {
        std::lock_guard<std::mutex> lock(mutex_);
        return vec_;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        vec_.clear();
    }

    template<typename Predicate>
    void remove_if(Predicate pred) {
        std::lock_guard<std::mutex> lock(mutex_);
        vec_.erase(std::remove_if(vec_.begin(), vec_.end(), pred), vec_.end());
    }

    void remove_value(const T& value) {
        remove_if([&](const T& item) { return item == value; });
    }

    bool contains(const T& value) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return std::find(vec_.begin(), vec_.end(), value) != vec_.end();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return vec_.size();
    }
private:
    std::vector<T> vec_;
    mutable std::mutex mutex_;
};
