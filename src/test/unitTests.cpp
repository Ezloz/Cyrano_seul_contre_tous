#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <algorithm>

struct Character {
    int id;
};

void bubbleSort(std::vector<std::pair<Character*, float>>& vec) {
    int n = static_cast<int>(vec.size());

    for (int i = 0; i < n - 1; ++i) {
        for (int j = 0; j < n - i - 1; ++j) {
            if (vec[j].second > vec[j + 1].second) {
                std::swap(vec[j], vec[j + 1]);
            }
        }
    }
}

TEST(BubbleSortTest, SortBySecondAscending) {
    Character a{1}, b{2}, c{3}, d{4};

    std::vector<std::pair<Character*, float>> vec = {
        {&a, 3.5f},
        {&b, 1.2f},
        {&c, 4.8f},
        {&d, 2.0f}
    };

    bubbleSort(vec);

    ASSERT_EQ(vec.size(), 4u);
    EXPECT_EQ(vec[0].first->id, 2);
    EXPECT_FLOAT_EQ(vec[0].second, 1.2f);

    EXPECT_EQ(vec[1].first->id, 4);
    EXPECT_FLOAT_EQ(vec[1].second, 2.0f);

    EXPECT_EQ(vec[2].first->id, 1);
    EXPECT_FLOAT_EQ(vec[2].second, 3.5f);

    EXPECT_EQ(vec[3].first->id, 3);
    EXPECT_FLOAT_EQ(vec[3].second, 4.8f);
}