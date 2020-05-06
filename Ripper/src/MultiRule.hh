#ifndef MULTI_RULE_HH
#define MULTI_RULE_HH

#include <vector>
#include <map>

#include "../../Domain/src/DomainElement.hh"
#include "GeneralRule.hh"

class MultiRule : public GeneralRule<DomainElement> {
    using GeneralRule::GeneralRule;
};

#endif
