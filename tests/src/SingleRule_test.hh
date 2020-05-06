#ifndef SINGLERULE_TEST_HH
#define SINGLERULE_TEST_HH

#include <catch2/catch.hpp>
#include <SingleRule.hh>

TEST_CASE("SingleRule Test") {
    auto sr = SingleRule(
        0,
        std::vector<int>{},
        std::vector<std::pair<double*,double*>>{},
        1,
        2
    );

    auto correctExamples = std::set<ExampleId>{ExampleId(1, 1)};
    auto incorrectExamples = std::set<ExampleId>{ExampleId(2, 2)};

    // all the setters mutate object properties properly
    sr.setCorrectExamples(correctExamples);
    sr.setIncorrectExamples(correctExamples);

    // all the getters access object properties properly
    REQUIRE(sr.getCorrectExamples() == correctExamples);
    REQUIRE(sr.getIncorrectExamples() == correctExamples);
}

#endif
