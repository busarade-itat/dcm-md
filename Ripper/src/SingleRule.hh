#ifndef SINGLE_RULE_HH
#define SINGLE_RULE_HH

#include <set>

#include "GeneralRule.hh"

// pair semantics:
// first - SID
// second - example index for given SID (there can be more examples per one SID)
using ExampleId = std::pair<int, int>;

class SingleRule : public GeneralRule<double> {
    using GeneralRule::GeneralRule;

    public:
        std::set<ExampleId> getCorrectExamples() const { return correctExamples; }
        std::set<ExampleId> getIncorrectExamples() const { return incorrectExamples; }
        void setCorrectExamples(const std::set<ExampleId>& examples) { correctExamples = examples; }
        void setIncorrectExamples(const std::set<ExampleId>& examples) { incorrectExamples = examples; }

    protected:
        std::set<ExampleId> correctExamples;
        std::set<ExampleId> incorrectExamples;
};

#endif
