#pragma once

#include <vector>
#include <unordered_set>
#include <algorithm>
#include <type_traits>
#define DECANCY_TH 0.01f

class Utils {
public:
    template <typename Container1, typename Container2>
    static std::vector<typename std::common_type<typename Container1::value_type, typename Container2::value_type>::type>
        intersect(const Container1& c1, const Container2& c2) {
        using T1 = typename Container1::value_type;
        using T2 = typename Container2::value_type;
        using CommonT = typename std::common_type<T1, T2>::type;

        std::unordered_set<CommonT> set2(c2.begin(), c2.end());

        std::vector<CommonT> result;
        for (const auto& val : c1) {
            if (set2.find(val) != set2.end()) {
                result.push_back(val);
            }
        }

        // Rimuovo duplicati e ordino (se serve)
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());

        return result;
    }

    static inline int64_t GetCurrentTimeMilliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }

    static float ComputeSmoothingP(float updateIntervalMs, float totalDecayTimeMs, float threshold = DECANCY_TH) {
        float iterations = totalDecayTimeMs / updateIntervalMs;
        float p = 1.0f - std::pow(threshold, 1.0f / iterations);
        return p;
    }

    static float Smooth(float b, float newVal, float p) {
        float val = b * (1 - p) + newVal * p;
        return val < DECANCY_TH ? 0 : val;
    }
};
