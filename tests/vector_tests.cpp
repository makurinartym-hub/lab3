#include "vector.hpp"

#include <gtest/gtest.h>
#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

static_assert(Numeric<int>);
static_assert(Numeric<double>);
static_assert(!Numeric<bool>);
static_assert(!Numeric<char>);
static_assert(!Numeric<std::string>);

static_assert(std::random_access_iterator<Vector<int>::Iterator>);

TEST(VectorTest, EmptyVector) {
    Vector<int> v;

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0u);
    EXPECT_EQ(v.capacity(), 0u);
    EXPECT_TRUE(v.begin() == v.end());
}

TEST(VectorTest, PushBack) {
    Vector<int> v;
    int number = 10;

    v.push_back(number);
    v.push_back(20);
    v.push_back(30);

    ASSERT_EQ(v.size(), 3u);
    EXPECT_GE(v.capacity(), v.size());

    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST(VectorTest, Access) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);

    v[0] = 15;
    v.at(1) = 25;

    EXPECT_EQ(v.front(), 15);
    EXPECT_EQ(v.back(), 25);
    EXPECT_THROW(v.at(2), std::out_of_range);

    const Vector<int>& c = v;

    EXPECT_EQ(c[0], 15);
    EXPECT_EQ(c.at(1), 25);
    EXPECT_EQ(c.front(), 15);
    EXPECT_EQ(c.back(), 25);
    EXPECT_THROW(c.at(2), std::out_of_range);
}

TEST(VectorTest, PopBackAndClear) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);

    v.pop_back();

    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v.back(), 10);

    auto capacity = v.capacity();
    v.clear();

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.capacity(), capacity);

    v.push_back(30);

    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v.front(), 30);
}

TEST(VectorTest, ReserveAndResize) {
    Vector<int> v;
    v.push_back(10);
    v.reserve(10);

    EXPECT_GE(v.capacity(), 10u);
    EXPECT_EQ(v.size(), 1u);

    v.resize(3);

    ASSERT_EQ(v.size(), 3u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 0);
    EXPECT_EQ(v[2], 0);

    v.resize(1);

    ASSERT_EQ(v.size(), 1u);
    EXPECT_EQ(v[0], 10);
    EXPECT_GE(v.capacity(), 10u);
}

TEST(VectorTest, Copy) {
    Vector<int> original;
    original.push_back(10);
    original.push_back(20);

    Vector<int> copy(original);
    copy[0] = 99;

    EXPECT_EQ(original[0], 10);
    EXPECT_EQ(copy[0], 99);

    Vector<int> assigned;
    assigned.push_back(-1);
    assigned = original;
    assigned[1] = 77;

    ASSERT_EQ(assigned.size(), 2u);
    EXPECT_EQ(assigned[0], 10);
    EXPECT_EQ(original[1], 20);
}

TEST(VectorTest, Move) {
    Vector<int> original;
    original.push_back(10);

    int* address = &original[0];

    Vector<int> moved(std::move(original));

    EXPECT_TRUE(original.empty());
    ASSERT_EQ(moved.size(), 1u);
    EXPECT_EQ(&moved[0], address);
    EXPECT_EQ(moved[0], 10);

    Vector<int> assigned;
    assigned.push_back(99);
    assigned = std::move(moved);

    EXPECT_TRUE(moved.empty());
    ASSERT_EQ(assigned.size(), 1u);
    EXPECT_EQ(&assigned[0], address);
    EXPECT_EQ(assigned[0], 10);

    original.push_back(20);
    EXPECT_EQ(original.front(), 20);
}

TEST(VectorTest, PushOwnElement) {
    Vector<int> v;
    v.push_back(10);

    ASSERT_EQ(v.size(), v.capacity());

    v.push_back(v[0]);

    ASSERT_EQ(v.size(), 2u);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 10);
}

TEST(VectorTest, Iterator) {
    Vector<int> v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    auto it = v.begin();

    EXPECT_EQ(*it++, 10);
    EXPECT_EQ(*it, 20);
    EXPECT_EQ(*++it, 30);

    EXPECT_EQ(*it--, 30);
    EXPECT_EQ(*--it, 10);

    it += 2;
    EXPECT_EQ(*it, 30);

    it -= 1;
    EXPECT_EQ(*it, 20);
    EXPECT_EQ(it[1], 30);
    EXPECT_EQ(*(it - 1), 10);
    EXPECT_EQ(*(1 + it), 30);

    EXPECT_EQ(v.end() - v.begin(), 3);
    EXPECT_TRUE(v.begin() < v.end());

    *it = 50;
    EXPECT_EQ(v[1], 50);
}

TEST(VectorTest, Algorithms) {
    Vector<int> v;
    v.push_back(30);
    v.push_back(10);
    v.push_back(20);

    std::sort(v.begin(), v.end());

    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);

    auto it = std::find(v.begin(), v.end(), 20);

    ASSERT_TRUE(it != v.end());
    EXPECT_EQ(*it, 20);

    EXPECT_TRUE(std::find(v.begin(), v.end(), 99) == v.end());

    std::for_each(v.begin(), v.end(), [](int& x) {
        x *= 2;
    });

    EXPECT_EQ(v[0], 20);
    EXPECT_EQ(v[1], 40);
    EXPECT_EQ(v[2], 60);
}

TEST(VectorTest, DoubleValues) {
    Vector<double> v;
    v.push_back(2.5);
    v.push_back(1.5);

    std::sort(v.begin(), v.end());

    EXPECT_DOUBLE_EQ(v.front(), 1.5);
    EXPECT_DOUBLE_EQ(v.back(), 2.5);
}