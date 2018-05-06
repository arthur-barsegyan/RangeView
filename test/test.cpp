#include "views.hpp"
#include "gtest/gtest.h"

#include <vector>

class RangeViewTest : public ::testing::Test  {
protected:
    void SetUp() {
        v = new std::vector<int>({1, 2, 3, 4, 5, 6, 7});
    }

    void TearDown() {
        delete v;
    }

    std::vector<int> *v;
};

TEST_F(RangeViewTest, create_rv_from_collection) {
    auto rv = (*v) | view::remove_if([](int i) { return i % 2 == 0; });
}

TEST_F(RangeViewTest, accumulate_with_transform) {
    int sum = accumulate(view::ints(1)
                         | view::transform([](int i){return i*i;})
                         | view::take(10));
    ASSERT_EQ(sum, 285);
}

TEST_F(RangeViewTest, ints_check) {
    auto rv = view::ints(1)
                | view::take(10);
}

TEST_F(RangeViewTest, accumulate_with_pred) {
    auto rv = view::ints(1)
                | view::remove_if([](int i) { return i % 2 == 0; })
                | view::take(10);

    ASSERT_EQ(accumulate(rv), 25);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}