#ifndef DCM_MASTER_TEMPORALVECTOR_TEST_HH
#define DCM_MASTER_TEMPORALVECTOR_TEST_HH

#include <catch2/catch.hpp>
#include <SingleRule.hh>

#include <set>
#include <ripper.hh>
#include "../../Domain/src/DomainElementFactory.hh"

double ripper_test_infinity(std::numeric_limits<double>::infinity());
double ripper_test_neg_infinity(-std::numeric_limits<double>::infinity());

bool singleRuleEquals(SingleRule& sr1, SingleRule& sr2) {
    bool haveSameValues = true;

    for (int i = 0; i < sr1.getValues().size(); ++i) {
        haveSameValues = haveSameValues && (
            (sr1.getValues().at(i).first != NULL && sr2.getValues().at(i).first != NULL &&
            *(sr1.getValues().at(i).first) == *(sr2.getValues().at(i).first)) ||
            (sr1.getValues().at(i).first == NULL && sr2.getValues().at(i).first == NULL)
        );

        haveSameValues = haveSameValues && (
            (sr1.getValues().at(i).second != NULL && sr2.getValues().at(i).second != NULL &&
            *(sr1.getValues().at(i).second) == *(sr2.getValues().at(i).second)) ||
            (sr1.getValues().at(i).second == NULL && sr2.getValues().at(i).second == NULL)
        );

        if (haveSameValues == false) {
            break;
        }
    }

    bool haveSameClass = sr1.getClass() == sr2.getClass();
    bool haveSameConstraints = sr1.getConstraints() == sr2.getConstraints();
    bool haveSameCorrect = sr1.getCorrectExamples() == sr2.getCorrectExamples();
    bool haveSameIncorrect = sr1.getIncorrectExamples() == sr2.getIncorrectExamples();
    bool haveSameFrequency = sr1.getFrequency() == sr2.getFrequency();
    bool haveSameOFrequency = sr1.getOFrequency() == sr2.getOFrequency();

    return (
        haveSameClass &&
        haveSameConstraints &&
        haveSameCorrect &&
        haveSameIncorrect &&
        haveSameFrequency &&
        haveSameOFrequency &&
        haveSameValues
    );
}

TEST_CASE("ripper test") {
    /**
     * DurationTypeIDs:
     * A _ A->B: 0
     * A _ B->C: 1
     */
    int class_pos = 0; // +

    auto sr1 = SingleRule(
        class_pos,
        std::vector<int>{0},
        std::vector<std::pair<double*,double*>>{
            std::pair<double*,double*>(
                new double(-2.4),
                new double(1.3)
            )
        },
        4,
        0
    );
    sr1.setCorrectExamples(
        std::set<std::pair<int, int>>{
            std::pair<int, int>(1, 0),
            std::pair<int, int>(2, 0),
            std::pair<int, int>(3, 0),
            std::pair<int, int>(4, 0)
        }
    );
    sr1.setIncorrectExamples(std::set<std::pair<int, int>>());

    auto sr2 = SingleRule(
        class_pos,
        std::vector<int>{0, 1},
        std::vector<std::pair<double*,double*>>{
            std::pair<double*,double*>(
                NULL,
                new double(6)
            ),
            std::pair<double*,double*>(
                new double(0),
                new double(1)
            )
        },
        2,
        2
    );
    sr2.setCorrectExamples(
        std::set<std::pair<int, int>>{
            std::pair<int, int>(1, 0),
            std::pair<int, int>(2, 0)
        }
    );
    sr2.setIncorrectExamples(
        std::set<std::pair<int, int>>{
            std::pair<int, int>(3, 0),
            std::pair<int, int>(4, 0)
        }
    );

    auto sr3 = SingleRule(
        class_pos,
        std::vector<int>{0},
        std::vector<std::pair<double*,double*>>{
            std::pair<double*,double*>(
                new double(0),
                NULL
            )
        },
        3,
        1
    );
    sr3.setCorrectExamples(
        std::set<std::pair<int, int>>{
            std::pair<int, int>(1, 0),
            std::pair<int, int>(2, 0),
            std::pair<int, int>(3, 0)
        }
    );
    sr3.setIncorrectExamples(
        std::set<std::pair<int, int>>{
            std::pair<int, int>(4, 0)
        }
    );

    std::vector<SingleRule> resultRun1;
    std::vector<SingleRule> resultRun2;
    std::map<std::vector<int>, std::map<int, SingleRule>> resultMap;

    resultRun1.push_back(sr1);
    resultRun1.push_back(sr2);
    resultRun2.push_back(sr3);

    ripper::indexRuleset(resultMap, resultRun1, 0);
    ripper::indexRuleset(resultMap, resultRun2, 1);

    SECTION("indexRuleset works according to the specification") {
        REQUIRE(resultMap.size() == 2);

        // It stores the rules correctly for the significand set {A_A->B}
        REQUIRE(resultMap.find(std::vector<int>{0})->second.size() == 2);
        REQUIRE(singleRuleEquals(resultMap.find(std::vector<int>{0})->second.find(0)->second, sr1));
        REQUIRE(singleRuleEquals(resultMap.find(std::vector<int>{0})->second.find(1)->second, sr3));

        // It stores the rules correctly for the significand set {A_A->B, A_B->C}
        REQUIRE(resultMap.find(std::vector<int>{0, 1})->second.size() == 1);
        REQUIRE(singleRuleEquals(resultMap.find(std::vector<int>{0, 1})->second.find(0)->second, sr2));

        // It does not store given rule into the rule map if no rule is present for given run
        REQUIRE(
            resultMap.find(std::vector<int>{0, 1})->second.find(1) ==
            resultMap.find(std::vector<int>{0, 1})->second.end()
        );
    }

    SECTION("mergeRulesets works according to the specification") {
        DomainElementFactory::VECTOR_SIZE = 2;
        auto outMultiRules = ripper::mergeRulesets(resultMap);

        // It creates a correct number of resulting rules
        REQUIRE(outMultiRules.size() == 2);

        // It assigns the pos label to all resulting rules
        // It assigns a distinct significand set for each resulting rule
        // It merges rule values correctly
        // It merges rule values correctly for some run rules missing
        // It computes the count of correctly/incorrectly covered examples for all resulting rules correctly

        REQUIRE(outMultiRules.at(0).getClass() == 0);
        REQUIRE(outMultiRules.at(0).getConstraints() == std::vector<int>{0});
        REQUIRE(outMultiRules.at(0).getValues().size() == 1);
        REQUIRE(outMultiRules.at(0).getValues().at(0).first->at(0) == -2.4);
        REQUIRE(outMultiRules.at(0).getValues().at(0).first->at(1) == 0);
        REQUIRE(outMultiRules.at(0).getValues().at(0).second->at(0) == 1.3);
        REQUIRE(outMultiRules.at(0).getValues().at(0).second->at(1) == ripper_test_infinity);
        REQUIRE(outMultiRules.at(0).getFrequency() == 3);
        REQUIRE(outMultiRules.at(0).getOFrequency() == 0);

        REQUIRE(outMultiRules.at(1).getClass() == 0);
        REQUIRE(outMultiRules.at(1).getConstraints() == std::vector<int>{0, 1});
        REQUIRE(outMultiRules.at(1).getValues().size() == 2);
        REQUIRE(outMultiRules.at(1).getValues().at(0).first->at(0) == ripper_test_neg_infinity);
        REQUIRE(outMultiRules.at(1).getValues().at(0).first->at(1) == ripper_test_neg_infinity);
        REQUIRE(outMultiRules.at(1).getValues().at(0).second->at(0) == 6);
        REQUIRE(outMultiRules.at(1).getValues().at(0).second->at(1) == ripper_test_infinity);
        REQUIRE(outMultiRules.at(1).getValues().at(1).first->at(0) == 0);
        REQUIRE(outMultiRules.at(1).getValues().at(1).first->at(1) == ripper_test_neg_infinity);
        REQUIRE(outMultiRules.at(1).getValues().at(1).second->at(0) == 1);
        REQUIRE(outMultiRules.at(1).getValues().at(1).second->at(1) == ripper_test_infinity);
        REQUIRE(outMultiRules.at(1).getFrequency() == 2);
        REQUIRE(outMultiRules.at(1).getOFrequency() == 2);
    }
}

#endif
