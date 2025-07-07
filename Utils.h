#include <vector>
#include <unordered_set>
#include <algorithm>
#include <type_traits>

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

    // Verifica se un vector contiene un valore
    template <typename T>
    static bool contains(const std::vector<T>& vec, const T& value) {
        return std::find(vec.begin(), vec.end(), value) != vec.end();
    }

    // Rimuove tutte le occorrenze di 'value' dal vector
    template <typename T>
    static void remove(std::vector<T>& vec, const T& value) {
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
    }

    static inline int64_t GetCurrentTimeMilliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
    }
};
