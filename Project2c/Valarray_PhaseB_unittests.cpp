/*
 * valarray_PhaseB_unittests.cpp
 * EPL - Spring 2015
 */

#include <chrono>
#include <complex>
#include <cstdint>
#include <future>
#include <iostream>
#include <stdexcept>

#include "InstanceCounter.h"
#include "Valarray.h"

#include "gtest/gtest.h"

using std::cout;
using std::endl;
using std::string;
using std::complex;

int InstanceCounter::counter = 0;

using namespace epl;

template <typename X, typename Y>
bool match(X x, Y y) {
    double d = x - y;
    if (d < 0) { d = -d; }
    return d < 1.0e-9; // not really machine epsilon, but close enough
}
/*********************************************************************/
// Phase B Tests
/*********************************************************************/

#if defined(PHASE_B1_0) | defined(PHASE_B)
TEST(PhaseB1, Sqrt) {
    valarray<int> v1(10);
    valarray<int> v4 = 4 + v1;

    valarray<float> v5 = v4.sqrt();
    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(2.0, v5[i]);
    }
}
#endif

#if defined(PHASE_B1_1) | defined(PHASE_B)
TEST(PhaseB1, Apply) {
    valarray<int> v1(10);
    valarray<int> v4 = 4 + v1;

    valarray<int> v7 = v4.apply(std::negate<int>());

    EXPECT_EQ(10, v7.size());

    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(-4, v7[i]);
    }
}
#endif

#if defined(PHASE_B1_2) | defined(PHASE_B)
TEST(PhaseB1, Accumulate) {
    valarray<int> v1{1, 2, 3, 4, 5};
    int sum = v1.accumulate(std::plus<int>());
    int product = v1.accumulate(std::multiplies<int>());
    EXPECT_EQ(15, sum);
    EXPECT_EQ(120, product);
}
#endif

#if defined(PHASE_B1_2) | defined(PHASE_B)
TEST(PhaseB1, ApplyPromote) {
    valarray<int> v1(10);
    valarray<int> v4 = 4 + v1;

    valarray<double> v7 = v4.apply(std::negate<double>());

    EXPECT_EQ(10, v7.size());

    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(-4.0, v7[i]);
    }

    valarray<double> v10(10);
    valarray<double> v5 = 5 + v10;

    valarray<complex<double>> v6 = v5.apply(std::negate<complex<double>>());

    EXPECT_EQ(10, v6.size());

    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(complex<double>(-5.0, 0), v6[i]);
    }
}
#endif

#if defined(PHASE_B1_2) | defined(PHASE_B)
TEST(PhaseB1, AccumulatePromote) {
    valarray<int> v1{1, 2, 3, 4, 5};
    double sum = v1.accumulate(std::plus<double>());
    double product = v1.accumulate(std::multiplies<double>());
    EXPECT_EQ(15.0, sum);
    EXPECT_EQ(120.0, product);

    complex<double> c_sum = v1.accumulate(std::plus<complex<double>>());
    complex<double> c_prod = v1.accumulate(std::multiplies<complex<double>>());

    EXPECT_EQ(complex<double>(15.0, 0), c_sum);
    EXPECT_EQ(complex<double>(120.0, 0), c_prod);
}
#endif

#if defined(PHASE_B1_3) | defined(PHASE_B)
TEST(PhaseB1, Lazy) {
    // lazy evaluation test
    valarray <double> v1, v2, v3, v4;
    for (int i = 0; i<10; ++i) {
        v1.push_back(1.0);
        v2.push_back(1.0);
        v3.push_back(1.0);
        v4.push_back(1.0);
    }
    int cnt = InstanceCounter::counter;
    v1 + v2 - (v3 * v4);
    EXPECT_EQ(cnt, InstanceCounter::counter);

    valarray<double> ans(10);
    ans = v1 + v2 - (v3*v4);
    EXPECT_TRUE(match(ans[3], (v1[3] + v2[3] - (v3[3] * v4[3]))));
}
#endif

template<typename A, typename B, typename C=int>
C operator+(A& a, B& b) { return int(7); }

#if defined(PHASE_B1_4) | defined(PHASE_B)
TEST(PhaseB1, Fun) {
    valarray<int> v1;
    unsigned int x = 0;
    int c = v1 + "hello world";

    EXPECT_EQ(c, 7);
}
#endif

#if defined(PHASE_B1_4) | defined(PHASE_B)
TEST(PhaseB1, Printing) {
    valarray<int> v1(5);
    valarray<double> v2 = 4 + v1;

    std::cout << v1 << " + 4 = " << v2 << '\n';
    std::cout << v1 << " + 5 = " << v1 + 5 << '\n';

    EXPECT_EQ(5, (v1 + 10).size());
}
#endif

#if defined(PHASE_B1_4) | defined(PHASE_B)
TEST(PhaseB1, VexprIter) {
    valarray<int> v1(5);

    for (auto y : (4 + v1)) {
        EXPECT_EQ(4, y);
    }
}
#endif

#if defined(PHASE_B1_4) | defined(PHASE_B)
TEST(PhaseB1, VexprAccumulate) {
    valarray<int> v1{1, 2, 3, 4, 5};
    double sum = (v1 + 4).sum();
    double product = (v1 + 4).accumulate(std::multiplies<double>());
    EXPECT_EQ(35.0, sum);
    EXPECT_EQ(15120.0, product);

}
#endif


#if defined(PHASE_B1_4) | defined(PHASE_B)
TEST(PhaseB1, VexprApply) {
    valarray<int> v1(10);
    auto v4 = 4 + v1;

    valarray<double> v7 = v4.apply(std::negate<double>());

    EXPECT_EQ(10, v7.size());

    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(-4.0, v7[i]);
    }

    valarray<double> v10(10);
    auto v5 = 5 + v10;

    auto v6 = v5.apply(std::negate<complex<double>>());

    EXPECT_EQ(10, v6.size());

    for (uint64_t i = 0; i<10; i++) {
        EXPECT_EQ(complex<double>(-5.0, 0), v6[i]);
    }
}
#endif
