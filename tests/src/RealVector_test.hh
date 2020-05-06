#ifndef REALVECTOR_TEST_HH
#define REALVECTOR_TEST_HH

#include <catch2/catch.hpp>
#include "../../Domain/src/RealVector.hh"

double vector_test_infinity(std::numeric_limits<double>::infinity());

TEST_CASE("Constructor Test") {
    // it creates a blank RealVector when using the parameterless constructor
    auto tv1 = RealVector();
    REQUIRE(tv1.size() == 0);
}

TEST_CASE("Addition Test") {
    auto tv1 = RealVector();
    tv1.push_back(1);
    tv1.push_back(2);
    tv1.push_back(3);

    auto tv2 = RealVector();
    tv2.push_back(1);
    tv2.push_back(1);
    tv2.push_back(1);

    auto tv3 = tv1 + tv2;

    // it adds two RealVectors together correctly
    REQUIRE(tv3[0] == 2);
    REQUIRE(tv3[1] == 3);
    REQUIRE(tv3[2] == 4);
}

TEST_CASE("Subtraction Test") {
    auto tv1 = RealVector();
    tv1.push_back(1);
    tv1.push_back(2);
    tv1.push_back(3);

    auto tv2 = RealVector();
    tv2.push_back(1);
    tv2.push_back(1);
    tv2.push_back(1);

    auto tv3 = tv1 - tv2;

    // it subtracts two RealVectors correctly
    REQUIRE(tv3[0] == 0);
    REQUIRE(tv3[1] == 1);
    REQUIRE(tv3[2] == 2);
}

TEST_CASE("isInsideBounds Test") {
    // test for value types
    auto tvb1 = RealVector();
    tvb1.push_back(1);
    tvb1.push_back(2);
    tvb1.push_back(3);

    auto tvb2 = RealVector();
    tvb2.push_back(3);
    tvb2.push_back(4);
    tvb2.push_back(5);

    auto tvb3 = RealVector();
    tvb3.push_back(vector_test_infinity);
    tvb3.push_back(4);
    tvb3.push_back(5);

    auto tvt1 = RealVector();
    tvt1.push_back(2);
    tvt1.push_back(3);
    tvt1.push_back(4);

    auto tvt2 = RealVector();
    tvt2.push_back(1);
    tvt2.push_back(2);
    tvt2.push_back(3);

    auto tvt3 = RealVector();
    tvt3.push_back(4);
    tvt3.push_back(4);
    tvt3.push_back(4);

    auto tvt4 = RealVector();
    tvt4.push_back(4);
    tvt4.push_back(5);
    tvt4.push_back(6);

    // it performs the hyperrectangle test correctly
    REQUIRE(tvt1.isInsideBounds(tvb1, tvb2) == true);

    // it performs the hyperrectangle test correctly for points on hyperrectangle bounds
    REQUIRE(tvt2.isInsideBounds(tvb1, tvb2) == true);

    // it performs the hyperrectangle test correctly for hyperrectangle bounds containing inf/-inf
    REQUIRE(tvt2.isInsideBounds(tvb1, tvb3) == true);

    // it performs the hyperrectangle test correctly for some coordinates outside of hyperrectangle bounds
    REQUIRE(tvt3.isInsideBounds(tvb1, tvb2) == false);

    // it performs the hyperrectangle test correctly for all coordinates outside of hyperrectangle bounds
    REQUIRE(tvt4.isInsideBounds(tvb1, tvb2) == false);
}

TEST_CASE("Formatting Test") {
    double test_infinity(std::numeric_limits<double>::infinity());
    double test_neg_infinity(-std::numeric_limits<double>::infinity());

    auto tv1 = RealVector();
    tv1.push_back(1);
    tv1.push_back(2);
    tv1.push_back(3);

    auto tv2 = RealVector();
    tv2.push_back(test_infinity);
    tv2.push_back(test_infinity);

    auto tv3 = RealVector();
    tv3.push_back(test_neg_infinity);

    auto tv4 = RealVector();
    tv4.push_back(0.00005);
    tv4.push_back(1.4);
    tv4.push_back(0.0000000005);

    // it formats a RealVector containing whole numbers correctly
    std::stringstream str1;
    str1 << tv1;
    REQUIRE(str1.str() == "<1,2,3>");

    // it formats RealVectors containing +/-inf values correctly
    std::stringstream str2;
    str2 << tv2;
    REQUIRE(str2.str() == "<inf,inf>");

    std::stringstream str3;
    str3 << tv3;
    REQUIRE(str3.str() == "<-inf>");

    // it formats a RealVector containing floating point numbers correctly
    // it formats a RealVector containing very small floating
    // point numbers using scientific notation
    std::stringstream str4;
    str4 << tv4;
    REQUIRE(str4.str() == "<5e-05,1.4,5e-10>");
}

#endif
