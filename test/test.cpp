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

TEST_F(RangeViewTest, transform_to_new_rv) {
	auto rv = view::ints(5)
				| view::transform([](int i) { return std::to_string(i); })
				| view::take(10)
				| view::remove_if([](std::string i) { return i.size() > 1; });

	ASSERT_EQ(accumulate(rv), "56789");
}

TEST_F(RangeViewTest, check_take_method_without_ints) {
	auto rv = (*v) | view::remove_if([](int i) { return i % 2 == 0; })
					| view::take(2);

	ASSERT_EQ(rv.to_vector(), {1, 3});
}

TEST_F(RangeViewTest, extcollection_is_immutable) {
	auto rv = (*v) | view::remove_if([](int i) { return i % 2 == 0; })
					| view::take(1);

	ASSERT_EQ(*v, {1, 2, 3, 4, 5, 6, 7});
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}