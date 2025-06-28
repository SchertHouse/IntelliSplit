#include <vector>
#include <unordered_set>
#include <algorithm>

class Utils {
public:
    template <typename Set1>
    static auto intersection(const Set1& set1, const Set1& set2)
        -> std::vector<typename Set1::value_type> {

        using T = typename Set1::value_type;

        std::vector<T> result;

        const auto& smaller = (set1.size() < set2.size()) ? set1 : set2;
        const auto& larger = (set1.size() < set2.size()) ? set2 : set1;

        for (const auto& val : smaller) {
            if (larger.contains(val)) {
                result.push_back(val);
            }
        }

        return result;
    }
};
