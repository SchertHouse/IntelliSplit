#pragma once
#include <bitset>
#include <vector>
#include <array>
#include <type_traits>
#include <cstddef>

template<typename T, T Min, T Max>
class SmallSet {
    static_assert(std::is_integral_v<T>, "SmallSet requires integral type");
    static constexpr size_t Range = static_cast<size_t>(Max - Min + 1);

public:
    void add(T value) {
        size_t idx = index(value);
        if (!presence[idx]) {
            presence.set(idx);
            positions[idx] = elements.size();
            elements.push_back(value);
        }
    }

    auto remove(T value) {
        size_t idx = index(value);
        if (!presence[idx]) return elements.end();

        presence.reset(idx);

        size_t pos = positions[idx];
        T last = elements.back();

        elements[pos] = last;
        positions[index(last)] = pos;

        elements.pop_back();
        return elements.begin() + pos;
    }

    bool contains(T value) const {
        return presence[index(value)];
    }

    void clear() {
        presence.reset();
        elements.clear();
    }

    const std::vector<T>& values() const {
        return elements;
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
