#ifndef DCM_MASTER_DOMAINELEMENTFACTORY_TEST_HH
#define DCM_MASTER_DOMAINELEMENTFACTORY_TEST_HH

#include <catch2/catch.hpp>
#include "../../Domain/src/DomainElementFactory.hh"

TEST_CASE("zero Test") {
    DomainElementFactory::VECTOR_SIZE = 1;

    // it constructs a RealVector with all zeroes correctly
    // it reacts to VECTOR_SIZE changes properly
    auto tv = DomainElementFactory::zero();

    REQUIRE(tv.size() == 1);
    REQUIRE(tv[0] == 0);
}

TEST_CASE("positiveInfinity Test") {
    double test_infinity(std::numeric_limits<double>::infinity());

    DomainElementFactory::VECTOR_SIZE = 2;

    // it constructs a RealVector with all infinity values correctly
    // it reacts to VECTOR_SIZE changes properly
    auto tv = DomainElementFactory::positiveInfinity();

    REQUIRE(tv.size() == 2);
    REQUIRE(tv[0] == test_infinity);
    REQUIRE(tv[1] == test_infinity);
}

TEST_CASE("negativeInfinity Test") {
    double test_neg_infinity(-std::numeric_limits<double>::infinity());

    DomainElementFactory::VECTOR_SIZE = 3;

    // it constructs a RealVector with all negative infinity values correctly
    // it reacts to VECTOR_SIZE changes properly
    auto tv = DomainElementFactory::negativeInfinity();

    REQUIRE(tv.size() == 3);
    REQUIRE(tv[0] == test_neg_infinity);
    REQUIRE(tv[1] == test_neg_infinity);
    REQUIRE(tv[2] == test_neg_infinity);
}

#endif
