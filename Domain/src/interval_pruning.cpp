#include <vector>

#include "interval_pruning.hh"

#include "DomainElement.hh"
#include "DomainElementFactory.hh"

void pruneRedundantIntervals(
        unsigned int significand,
        std::vector<std::vector<DomainElement>>& outputIntervals,
        const std::vector<DomainElement>& example) {
    for (int v = 0; v < DomainElementFactory::VECTOR_SIZE; ++v) {
        if (outputIntervals[significand][0][v] > example[significand][v]) {
            outputIntervals[significand][0][v] = example[significand][v];
        }
        else if (outputIntervals[significand][1][v] < example[significand][v]) {
            outputIntervals[significand][1][v] = example[significand][v];
        }
    }
}

