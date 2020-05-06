#ifndef __RIPPER_HH__

#include <sstream>
#include <vector>
#include <iostream>

#include "MultiRule.hh"
#include "SingleRule.hh"

namespace ripper{
    enum BoundType { LOWER_BOUND, UPPER_BOUND };

    void singleInit(
        int n,
        const std::vector<std::vector<std::vector<double>>>& a,
        const std::vector<std::vector<std::vector<double>>>& b,
        double gmin,
        bool irep,
        bool useSID = true
    );

    std::vector<MultiRule> run(
        int n,
        const std::vector<std::vector<std::vector<DomainElement>>>& a,
        const std::vector<std::vector<std::vector<DomainElement>>>& b,
        int fmin,
        double gmin,
        bool irep,
        bool print,
        bool useSID = true
    );

    std::vector<SingleRule> singleRun(bool print, int fmin, double gmin);

    void indexRuleset(
        std::map<std::vector<int>, std::map<int, SingleRule>>& singleRunResults,
        const std::vector<SingleRule>& result,
        int i
    );

    MultiRule mergeRules(const std::pair<std::vector<int>, std::map<int, SingleRule>> &srr);

    std::pair<std::set<ExampleId>, std::set<ExampleId>> checkExamples(
            const SingleRule& rule,
            const std::vector<std::vector<std::vector<double>>>& positiveExamples,
            const std::vector<std::vector<std::vector<double>>>& negativeExamples
    );

    void checkExampleSet(
            std::pair<std::set<ExampleId>, std::set<ExampleId>>& out,
            int datasetClass,
            const SingleRule& rule,
            const std::vector<std::vector<std::vector<double>>>& dataset
    );

    bool checkExample(const SingleRule& rule, const std::vector<double>& datasetRow);

    void mergeValues(std::vector<std::pair<DomainElement*, DomainElement*>> &resultValues,
                     int runIndex, std::vector<std::pair<double*, double*>> &inputValues);

    std::vector<MultiRule> mergeRulesets(const std::map<std::vector<int>,
            std::map<int, SingleRule>>& singleRunResults);

    double coalesceValue(double* ruleValue, BoundType boundType);
}

#endif 
