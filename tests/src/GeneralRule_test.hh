#ifndef GENERAL_RULE_TEST_HH
#define GENERAL_RULE_TEST_HH

#include <catch2/catch.hpp>
#include <GeneralRule.hh>

TEST_CASE("GeneralRule<T> Test") {
    int klazz = 1;
    std::vector<int> tcs{1, 2, 3};
    std::vector<std::pair<int*,int*>> values =
        std::vector<std::pair<int*,int*>> {
            std::pair<int*,int*>(NULL, new int(4)),
            std::pair<int*,int*>(new int(6), NULL)
        };
    unsigned int frequency = 10;
    unsigned int ofrequency = 20;

    auto gr = GeneralRule<int>(
        klazz,
        tcs,
        values,
        frequency,
        ofrequency
    );

    // it constructs an instance of GeneralRule<int> correctly
    REQUIRE(gr.getClass() == klazz);
    REQUIRE(gr.getConstraints() == tcs);

    // all the getters access object properties properly
    REQUIRE(gr.getValues().size() == values.size());
    REQUIRE(gr.getValues()[0].first == NULL);
    REQUIRE(*(gr.getValues()[0].second) == 4);
    REQUIRE(*(gr.getValues()[1].first) == 6);
    REQUIRE(gr.getValues()[1].second == NULL);
    REQUIRE(gr.getFrequency() == frequency);
    REQUIRE(gr.getOFrequency() == ofrequency);
}

#endif
