#include "../../Domain/src/DomainElementFactory.hh"
#include <set>
#include "ripper.hh"

#include "ripper.h"
#include "mdb.h"

// named constants
double infinity(std::numeric_limits<double>::infinity());
double neg_infinity(-std::numeric_limits<double>::infinity());

namespace ripper {
/* Solutions set. */
    concept_t *hyp;
/* Occurrences set. */
    vec_t *data;
/* Classes */
    atom_t class_a;
    atom_t class_b;

    void class_init() {
        /* Class definitions. */
        class_a.nom = intern("a");
        class_a.nom->index = 0;
        class_a.nom->kind = CLASS;

        class_b.nom = intern("b");
        class_b.nom->index = 0;
        class_b.nom->kind = CLASS;

        if (!Classes) Classes = new_vec(atom_t);
        else
            clear_vec(atom_t, Classes);

        ext_vec(atom_t, Classes, &class_a);
        ext_vec(atom_t, Classes, &class_b);
    }

    void ripper_init(const bool irep) {
        /* Ripper parameters. */
        Simplify = (irep)?TRUE:FALSE;
        Force_independent_rules = TRUE;
        Uniq_rule_class = TRUE;
        Class_ordering = UNORDERED;

        Trace_level = 0;
    }

    void names_init(int n) {
        if (!Names) Names = new_vec(attr_def_t);
        else
            clear_vec(attr_def_t, Names);

        for (int i = 0; i < n; i++) {
            std::stringstream sn;
            sn << "t" << i;
            attr_def_t adef;
            adef.name = intern(sn.str().c_str());
            adef.values = new_vec(symbol_t *);
            adef.value_index = NULL;
            adef.isbag = FALSE;
            adef.suppressed = FALSE;
            adef.name->kind = ATTRIBUTE;
            adef.name->index = i;
            adef.kind = CONTINUOUS;

            ext_vec(attr_def_t, Names, &adef);
        }

        set_names_defined();
    }

    void example_init(
            const atom_t &classe,
            const std::vector<std::vector<std::vector<double>>> &a,
            const unsigned int& offset,
            double weight
    ) {
        aval_t av;
        example_t ex;

        av.kind = CONTINUOUS;
        ex.wt = (ex_count_t) weight;
        ex.lab = classe;

        for(unsigned int i = 0; i < a.size(); i++){
            ex.sid = i+offset;
            for (std::vector<double> e : a[i]) {
                ex.inst = _new_vec(sizeof(aval_t), vmax(Names));

                for (double v : e) {
                    av.u.num = v;
                    ext_vec(aval_t, ex.inst, &av);
                }

                ext_vec(example_t, data, &ex);
            }
        }
    }

    void data_init(
            const std::vector<std::vector<std::vector<double>>> &a,
            const std::vector<std::vector<std::vector<double>>> &b,
            double gmin,
            bool useSID = true
    ) {
        data = new_vec(example_t);

        example_init(class_a, a, 0, 1);
        example_init(class_b, b, (const unsigned int &) a.size(), gmin);

        if (useSID) {
            if (!SID) SID = new_vec(BOOL);
            else
                clear_vec(BOOL, SID);

            int nb_SID = (int) (a.size() + b.size());
            BOOL t = TRUE;

            for (int i = 0; i < nb_SID; i++)
                ext_vec(BOOL, SID, &t);
        }
    }

    void singleInit(
            int n,
            const std::vector<std::vector<std::vector<double>>> &a,
            const std::vector<std::vector<std::vector<double>>> &b,
            double gmin,
            bool irep,
            bool useSID
    ) {
        class_init();
        ripper_init(irep);
        names_init(n);
        data_init(a, b, gmin, !irep && useSID);

        ld_grammar((char *) "unfound", 1, data);
    }

    SingleRule makeRule(int c, rule_t *r, unsigned int frequency, unsigned int ofrequency) {
        gsym_t *gsym;

        std::vector<int> tcs;
        std::vector<std::pair<double*, double*>> values;

        for (int k = 0; k < vmax(r->antec); k++) {
            gsym = vref(gsym_t, r->antec, k);
            if (!gsym->nonterm) {
                int i = gsym->attr_index;
                if (tcs.size() == 0 || tcs.back() != i) {
                    tcs.push_back(i);
                    values.push_back(std::pair<double*, double*>(NULL, NULL));
                }

                double ruleValue = gsym->value.num;

                if (gsym->op == OPLE) {
                    if (values.back().second == NULL) {
                        values.back().second = new double(ruleValue);
                    }
                    else if (*values.back().second > ruleValue) {
                        *values.back().second = ruleValue;
                    }
                }
                else if (gsym->op == OPGE) {
                    if (values.back().first == NULL) {
                        values.back().first = new double(ruleValue);
                    }
                    else if (*values.back().first < ruleValue) {
                        *values.back().first = ruleValue;
                    }
                }
            }
        }

        return SingleRule(c, tcs, values, frequency, ofrequency);
    }

    std::vector<SingleRule> singleRun(bool print, int fmin, double gmin) {
        Min_coverage = fmin;
        FP_cost = gmin;

        if (print) std::cerr << "[INFO] Model generation" << std::endl;
        hyp = model(data);

        if (print) {
            std::cerr << "[INFO] Print of " << vmax(hyp->rules) << " extracted rules" << std::endl;
            fprint_concept(stderr, hyp);
            std::cerr << std::endl;
        }

        std::vector<SingleRule> res;
        int c = 0;

        for (int i = 0; i < vmax(hyp->rules); i++) {
            rule_t *r = vref(rule_t, hyp->rules, i);
            c = -1;
            if (vmax(r->antec) > 0) {
                if (r->conseq->name[0] == 'a') {
                    c = 0;
                }
                else if (r->conseq->name[0] == 'b') {
                    c = 1;
                }
                if (c >= 0) {
                    res.push_back(makeRule(c, r, r->nposx, r->nnegx));
                }
            }
        }

        return res;
    }

    std::vector<std::vector<std::vector<double>>>
    splitDatasetPart(const std::vector<std::vector<std::vector<DomainElement>>>& multiData, int elementIndex) {
        std::vector<std::vector<std::vector<double>>> out;

        for (auto level1Item : multiData) {
            std::vector<std::vector<double>> l1Out;

            for (auto level2Item : level1Item) {
                std::vector<double> l2Out;

                for (DomainElement& tv: level2Item) {
                    l2Out.push_back(tv.at(elementIndex));
                }

                l1Out.push_back(l2Out);
            }

            out.push_back(l1Out);
        }

        return out;
    }

    std::vector<MultiRule> run(
            int n,
            const std::vector<std::vector<std::vector<DomainElement>>>& positiveData,
            const std::vector<std::vector<std::vector<DomainElement>>>& negativeData,
            int fmin,
            double gmin,
            bool irep,
            bool print,
            bool useSID
    ) {
        // map:
        // {tcs} => {run index -> SingleRule}
        std::map<
            std::vector<int>,
            std::map<int, SingleRule>
        > singleRunResults;

        std::vector<std::vector<std::vector<double>>> positiveSingleData;
        std::vector<std::vector<std::vector<double>>> negativeSingleData;

        for (int i = 0; i < DomainElementFactory::VECTOR_SIZE; ++i) {
            // positiveSingleData = splitDatasetPart(positiveData, i);
            //negativeSingleData = splitDatasetPart(negativeData, i);

            // A->B = index 0
            // B->C = index 1
            // A->C = index 2

            // pos
            // SID=1
            std::vector<std::vector<double>> sid1;

            // ex1
            sid1.push_back(std::vector<double>{1, -4.6, 6.3});
            // ex2
            sid1.push_back(std::vector<double>{0, 9.6, 0});

            positiveSingleData.push_back(sid1);

            // SID=2
            std::vector<std::vector<double>> sid2;

            sid2.push_back(std::vector<double>{1.6, 4, 3.9});

            positiveSingleData.push_back(sid2);

            // neg
            // SID=3
            std::vector<std::vector<double>> sid3;

            sid3.push_back(std::vector<double>{-3.6, 0, 0});

            negativeSingleData.push_back(sid3);

            singleInit(n, positiveSingleData, negativeSingleData, 1, irep, false);
            std::vector<SingleRule> result = singleRun(print, fmin, gmin);

            for (auto& sr : result) {
                auto coveredExamples = checkExamples(sr, positiveSingleData, negativeSingleData);

                sr.setCorrectExamples(coveredExamples.first);
                sr.setIncorrectExamples(coveredExamples.second);
            }

            indexRuleset(singleRunResults, result, i);
        }

        return mergeRulesets(singleRunResults);
    }

    std::pair<std::set<ExampleId>, std::set<ExampleId>> checkExamples(
            const SingleRule& rule,
            const std::vector<std::vector<std::vector<double>>>& positiveExamples,
            const std::vector<std::vector<std::vector<double>>>& negativeExamples) {
        auto out = std::pair<std::set<ExampleId>, std::set<ExampleId>>(
            std::set<ExampleId>{}, // .first = correct example set
            std::set<ExampleId>{} // .second = incorrect example set
        );

        checkExampleSet(out, 0, rule, positiveExamples);
        checkExampleSet(out, 1, rule, negativeExamples);

        if (out.first.size() != rule.getFrequency() || out.second.size() != rule.getOFrequency()) {
            throw std::logic_error("Computed correct/incorrect examples do not match RIPPERk nposx and nnegx.");
        }
        else {
            return out;
        }
    }

    void checkExampleSet(
            std::pair<std::set<ExampleId>, std::set<ExampleId>>& out,
            int datasetClass,
            const SingleRule& rule,
            const std::vector<std::vector<std::vector<double>>>& dataset) {
        for (int i = 0; i < dataset.size(); ++i) { // "i" represents SID over here
            for (int j = 0; j < dataset[i].size(); ++j) { // "j" represents index of example of given SID
                // checkResults signifies whether the rule fulfills this dataset row
                bool checkResult = checkExample(rule, dataset[i][j]);

                if (checkResult) {
                    if (datasetClass == 0) { // fulfills for pos dataset = correctly covered example
                        out.first.insert(ExampleId(i, j));
                    }
                    else if (datasetClass == 1) { // fulfills for neg dataset = INcorrectly covered example
                        out.second.insert(ExampleId(i, j));
                    }
                }
            }
        }
    }

    bool checkExample(const SingleRule& rule, const std::vector<double>& datasetRow) {
        auto tcs = rule.getConstraints();
        auto values = rule.getValues();

        if (tcs.size() != values.size()) {
            throw std::logic_error("tcs and values have different size.");
        }
        else {
            // for empty tcs it should be true because the rule
            // does not lay any further conditions on the example
            bool result = true;

            for (int i = 0; i < tcs.size(); ++i) {
                int exampleItemIndex = tcs[i];
                double lowerBound = values[i].first == NULL ? neg_infinity : *(values[i].first);
                double upperBound = values[i].second == NULL ? infinity : *(values[i].second);

                result = result && (
                    datasetRow[exampleItemIndex] >= lowerBound &&
                    datasetRow[exampleItemIndex] <= upperBound
                );
            }

            return result;
        }
    }

    std::vector<MultiRule> mergeRulesets(const std::map<std::vector<int>, std::map<int, SingleRule>>& singleRunResults) {
        std::vector<MultiRule> out;

        for (auto& srr : singleRunResults) {
            out.push_back(mergeRules(srr));
        }

        return out;
    }

    MultiRule mergeRules(const std::pair<std::vector<int>,std::map<int, SingleRule>> &srr) {
        int ruleClass = 0;
        std::vector<std::pair<DomainElement*,DomainElement*>> values;
        std::vector<int> tcs = srr.first;

        std::set<ExampleId> correctlyCoveredExamples;
        std::set<ExampleId> incorrectlyCoveredExamples;

        for (int i = 0; i < tcs.size(); ++i) {
            DomainElement* lowerBound = new DomainElement();
            DomainElement* upperBound = new DomainElement();

            for (int j = 0; j < DomainElementFactory::VECTOR_SIZE; ++j) {
                lowerBound->push_back(neg_infinity);
                upperBound->push_back(infinity);
            }

            values.push_back(std::pair<DomainElement*,DomainElement*>(lowerBound, upperBound));
        }

        for (int i = 0; i < DomainElementFactory::VECTOR_SIZE; ++i) {
            auto resultItemIterator = srr.second.find(i);

            if (resultItemIterator != srr.second.end()) {
                const SingleRule& sr = (*resultItemIterator).second;
                std::vector<std::pair<double*, double*>> srValues = sr.getValues();

                mergeValues(values, i, srValues);

                if (i == 0) {
                    correctlyCoveredExamples = sr.getCorrectExamples();
                    incorrectlyCoveredExamples = sr.getIncorrectExamples();
                }
                else {
                    std::set<ExampleId> newCorrectExamples;
                    std::set<ExampleId> newIncorrectExamples;

                    std::set_intersection(
                        correctlyCoveredExamples.begin(), correctlyCoveredExamples.end(),
                        sr.getCorrectExamples().begin(), sr.getCorrectExamples().end(),
                        std::inserter(newCorrectExamples, newCorrectExamples.begin())
                    );
                    std::set_intersection(
                        incorrectlyCoveredExamples.begin(), incorrectlyCoveredExamples.end(),
                        sr.getIncorrectExamples().begin(), sr.getIncorrectExamples().end(),
                        std::inserter(newIncorrectExamples, newIncorrectExamples.begin())
                    );

                    correctlyCoveredExamples = newCorrectExamples;
                    incorrectlyCoveredExamples = newIncorrectExamples;
                }
            }
        }

        MultiRule outRule = MultiRule(
            ruleClass,
            tcs,
            values,
            correctlyCoveredExamples.size(),
            incorrectlyCoveredExamples.size()
        );

        return outRule;
    }

    void mergeValues(std::vector<std::pair<DomainElement*, DomainElement*>> &resultValues, int runIndex,
                     std::vector<std::pair<double*, double*>> &inputValues) {
        for (int j = 0; j < inputValues.size(); ++j) {
            auto srValue = inputValues.at(j);

            resultValues.at(j).first->at(runIndex) = coalesceValue(srValue.first, BoundType::LOWER_BOUND);
            resultValues.at(j).second->at(runIndex) = coalesceValue(srValue.second, BoundType::UPPER_BOUND);
        }
    }

    double coalesceValue(double* ruleValue, BoundType boundType) {
        if (boundType == BoundType::LOWER_BOUND) {
            return ruleValue == NULL ? neg_infinity : *ruleValue;
        }
        else if (boundType == BoundType::UPPER_BOUND) {
            return ruleValue == NULL ? infinity : *ruleValue;
        }
        // else nothing
    }

    void indexRuleset(
            std::map<std::vector<int>, std::map<int, SingleRule>>& singleRunResults,
            const std::vector<SingleRule>& result,
            int i
    ) {
        for (const SingleRule& sr : result) {
            std::vector<int> resultKey = sr.getConstraints();

            auto resultItemIterator = singleRunResults.find(resultKey);

            if (resultItemIterator == singleRunResults.end()) {
                std::map<int, SingleRule> resultValue;
                resultValue.emplace(i, sr);

                singleRunResults.emplace(resultKey, resultValue);
            }
            else {
                std::map<int, SingleRule>& constraintRuns = (*resultItemIterator).second;

                auto resultItemIterator2 = constraintRuns.find(i);

                if (resultItemIterator2 == constraintRuns.end()) {
                    constraintRuns.emplace(i, sr);
                }
            }
        }
    }
}
