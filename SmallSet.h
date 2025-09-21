#pragma once
#include <bitset>
#include <vector>
#include <array>
#include <type_traits>
#include <cstddef>
#include <mutex>

template<typename T, T Min, T Max>
class SmallSet {
    static_assert(std::is_integral_v<T>, "SmallSet requires integral type");
    static constexpr size_t Range = static_cast<size_t>(Max - Min + 1);

public:
    using value_type = T;
    std::mutex dataMutex;

    void add(T value) {
        std::lock_guard<std::mutex> lock(dataMutex);
        size_t idx = index(value);
        if (!presence[idx]) {
            presence.set(idx);
            positions[idx] = elements.size();
            elements.push_back(value);
        }
    }

    auto remove(T value, boolean mutex = true) {
        if(mutex)
            std::lock_guard<std::mutex> lock(dataMutex);
        size_t idx = index(value);
        if (!presence[idx]) return elements.end();

        presence.reset(idx);

        size_t pos = positions[idx];
        T last = elements.back();

        if (value != last) {
            elements[pos] = last;
            positions[index(last)] = pos;
        }

        elements.pop_back();

        if (pos < elements.size()) {
            return elements.begin() + pos;
        }
        else {
            return elements.end();
        }
    }

    bool contains(T value) const {
        return presence[index(value)];
    }

    size_t size() const {
        return elements.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(dataMutex);
        presence.reset();
        elements.clear();
    }

    const std::vector<T>& values() const {
        return elements;
    }

    template<typename Predicate, typename ModifyFunc>
    void remove_if_modify(Predicate pred, ModifyFunc modify) {
        std::lock_guard<std::mutex> lock(dataMutex);
        for (size_t i = 0; i < elements.size(); ) {
            T val = elements[i];
            if (pred(val)) {
                modify(val);
                remove(val, false);
            }
            else {
                modify(val);
                ++i;
            }
        }
    }

    auto begin() { return elements.begin(); }
    auto end() { return elements.end(); }
    auto begin() const { return elements.begin(); }
    auto end()   const { return elements.end(); }

private:
    static constexpr size_t index(T value) {
        return static_cast<size_t>(value - Min);
    }

    std::bitset<Range> presence;
    std::vector<T> elements;
    std::array<size_t, Range> positions{};
};
