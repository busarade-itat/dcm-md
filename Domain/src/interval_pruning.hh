#ifndef INTERVAL_PRUNING_HH
#define INTERVAL_PRUNING_HH

#include "DomainElement.hh"

void pruneRedundantIntervals(
        unsigned int significand,
        std::vector<std::vector<DomainElement>>& outputIntervals,
        const std::vector<DomainElement>& example);

#endif
