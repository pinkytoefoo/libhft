#pragma once

#include <hft/containers/unique_ptr.hpp>
#include <gtest/gtest.h>

// tests modified from
// https://quantdev.blog/posts/implementing_unique_ptr/#basic-functionalities-to-expect-out-of-stdunique_ptrt
TEST(UniquePtrTests, CreateAndAccessTest)
{
    int* raw_ptr = new int(42);
    hft::unique_ptr<int> p1(raw_ptr);
    
    EXPECT_EQ(*p1 == 42,true);
    EXPECT_EQ(p1.get(), raw_ptr);

    hft::unique_ptr<int> p2 = new int(17);
    EXPECT_EQ(*p2 == 17,true);
    EXPECT_EQ(p2.get() != nullptr, true);
}

/* Move constructor - Transfer of ownership */
TEST(UniquePtrTests, MoveConstructorTestAndMakeUnique)
{
    hft::unique_ptr p1 = hft::make_unique<int>(17);
    hft::unique_ptr p2 {std::move(p1)};

    EXPECT_EQ(p1.get(), nullptr);
    EXPECT_EQ(*p2, 17);
    EXPECT_EQ(p2!=nullptr, true);
}

/* Move assignment */
TEST(UniquePtrTests, MoveAssignmentTest)
{
    hft::unique_ptr<int> p1(new int(42));
    p1 = hft::unique_ptr<int>(new int(17));

    EXPECT_EQ(p1!=nullptr, true);
    EXPECT_EQ(*p1 == 17, true);
}

TEST(UniquePtrTests, NullptrAssign)
{
    hft::unique_ptr<int> p(new int(42));
    p = nullptr;
    EXPECT_EQ(p, nullptr);
}

TEST(UniquePtrTests, CopiesDisabled)
{
    hft::unique_ptr<int> p = hft::make_unique<int>(42);

    p = nullptr;
    EXPECT_EQ(p, nullptr);

    EXPECT_FALSE(std::is_copy_constructible_v<hft::unique_ptr<int>>);
    EXPECT_FALSE(std::is_copy_assignable_v<hft::unique_ptr<int>>);
    // EXPECT_THROW(copy_p());
}

// Modifiers
/* release() : Returns the pointer to resource and releases ownership*/
TEST(UniquePtrTests, ReleaseTest){
    hft::unique_ptr<double> ptr(new double(3.14));
    double* rawPtr = ptr.release();

    EXPECT_EQ(ptr == nullptr, true);
    EXPECT_EQ(rawPtr != nullptr, true);
    EXPECT_EQ(*rawPtr == 3.14, true);

    delete rawPtr;
}

/* reset() :  replaces the managed object */
TEST(UniquePtrTests, ResetUniquePtr) {
    hft::unique_ptr<int> ptr(new int(10));
    ptr.reset(new int(20));
    EXPECT_EQ(ptr != nullptr, true);
    EXPECT_EQ(*ptr == 20, true);

    // Self-reset test
    // ptr.reset(ptr.get());
}

/* swap() : swap the managed objects */
TEST(UniquePtrTests, SwapTest){
    int* first = new int(42);
    int* second = new int(17);

    hft::unique_ptr<int> p1(first);
    hft::unique_ptr<int> p2(second);

    swap(p1, p2);

    EXPECT_EQ(p2.get() == first && p1.get() == second, true);
    EXPECT_EQ(((*p1) == 17) && ((*p2) == 42), true);
}

// Observers
/* get() : Returns a pointer to the managed object or nullptr*/
TEST(UniquePtrTests, GetTest){
    double* resource = new double(0.50);
    hft::unique_ptr p(resource);

    EXPECT_EQ(p.get() == resource, true);
    EXPECT_EQ(*(p.get()) == 0.50, true);
}

/* operator bool() : Checks whether *this owns an object*/
TEST(UniquePtrTests, OperatorBoolTest){
    int* resource = new int(28);
    hft::unique_ptr<int> p1;
    hft::unique_ptr<int> p2(resource);

    EXPECT_EQ(p1 != p2, true);
    EXPECT_EQ(p2 != nullptr, true);
}

// Pointer-like functions
TEST(UniquePtrTests, IndirectionOperatorTest) {
    /* indirection operator* to dereference pointer to managed object,
       member access operator -> to call member function*/
    struct X {
        int _n;

        X() = default;
        X(int n) : _n{n} {}
        ~X() = default;
        int foo() { return _n; }
    };

    hft::unique_ptr<X> ptr(new X(10));
    EXPECT_EQ((*ptr)._n == 10, true);
    EXPECT_EQ(ptr->foo() == 10, true);
}

// checks at compile time, but still nice as runtime check
TEST(UniquePtrTests, Constexpr) {
    constexpr auto lambda = []() {
        auto a = hft::make_unique<int>(300);
        return *a + 1;
    };
    constexpr auto res = lambda();
    EXPECT_EQ(res, 301);
}

// TEST(UniquePtrTests, PointerToArrayOfTConstructionAndAccess){
//     /* Constructing unique_ptr<T[]> and access */
//     hft::unique_ptr<int[]> p;
//     EXPECT_EQ(p == nullptr, true);
//     {
//         p = new int[5]{1, 2, 3, 4, 5};
//         EXPECT_EQ(p !=nullptr, true);
//         EXPECT_EQ(*p == 1, true);
//         EXPECT_EQ(p[2] == 3, true);
//         p.release();
//         EXPECT_EQ(p ==nullptr, true);
//     }
// }
