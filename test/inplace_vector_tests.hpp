#pragma once

#include <hft/containers/inplace_vector.hpp>
#include <gtest/gtest.h>

TEST(InplaceVector, Constructors) {
    hft::inplace_vector<int, 5> v(5, 12);
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v.capacity(), 5);
    EXPECT_EQ(v.empty(), false);
    EXPECT_EQ(v.full(), true);

    for(auto const num : v) {
        EXPECT_EQ(num, 12);
    }

    auto create_bad_vec = []{ hft::inplace_vector<int, 5>(10); };
    EXPECT_THROW(create_bad_vec(), std::length_error);
}

TEST(InplaceVector, Accessors) {
    hft::inplace_vector<int, 5> v;
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), 5);
    EXPECT_EQ(v.empty(), true);
    EXPECT_EQ(v.full(), false);

    EXPECT_EQ(v.push_back(12), true);
    EXPECT_EQ(v.size(), 1);
    EXPECT_EQ(v.empty(), false);
    EXPECT_EQ(v.full(), false);
}

TEST(InplaceVector, PushPop) {
    hft::inplace_vector<int, 10> v(9);
    EXPECT_EQ(v.size(), 9);
    EXPECT_EQ(v.capacity(), 10);
    EXPECT_EQ(v.empty(), false);
    EXPECT_EQ(v.full(), false);

    EXPECT_EQ(*v.begin(), 0);
    EXPECT_TRUE(v.push_back(982));
    EXPECT_EQ(v.size(), 10);
    EXPECT_TRUE(v.full());
    EXPECT_FALSE(v.push_back(4762));

    EXPECT_TRUE(v.try_pop_back());
    EXPECT_TRUE(v.try_pop_back());
    EXPECT_TRUE(v.try_pop_back());
    EXPECT_TRUE(v.try_pop_back());
    EXPECT_EQ(v.size(), 6);
}

TEST(InplaceVector, Destructor) {
    int constructions = 0;
    int destructions = 0;

    struct Tracker {
        int& constructions;
        int& destructions;

        Tracker(int& c, int& d)
            : constructions{c}
            , destructions{d}
        {
            ++constructions;
        }

        ~Tracker() {
            ++destructions;
        }
    };

    {
        hft::inplace_vector<Tracker, 7> v(7, constructions, destructions);
    }

    EXPECT_EQ(constructions, 7);
    EXPECT_EQ(destructions, 7);
}

TEST(InplaceVector, PointerRetrieval) {
    struct alignas(64) OverAligned {
        int value{};
    };

    hft::inplace_vector<OverAligned, 7> v(7);
    for(auto const over : v) {
        EXPECT_EQ(reinterpret_cast<std::uintptr_t>(&over) & (64 - 1),  0);
    }
}

// TEST(InplaceVector, FrontBack) {
//     hft::inplace_vector<float, 3> v;
//     v.emplace(v.begin(), 6.823f);
//     v.emplace(v.end(), 1.32458f);
//     EXPECT_EQ(v.front(), 1.32458f);
//     EXPECT_EQ(v.back(), 6.823f);
// }
