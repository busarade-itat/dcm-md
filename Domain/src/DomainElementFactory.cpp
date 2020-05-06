#include "DomainElementFactory.hh"

// named constants
double rv_infinity(std::numeric_limits<double>::infinity());
double rv_neg_infinity(-std::numeric_limits<double>::infinity());

unsigned int DomainElementFactory::VECTOR_SIZE = 1;

DomainElement DomainElementFactory::numberFill(double number) {
    RealVector out;

    for (unsigned long i = 0; i < DomainElementFactory::VECTOR_SIZE; ++i) {
        out.push_back(number);
    }

    return out;
}

DomainElement DomainElementFactory::negativeInfinity() {
    return DomainElementFactory::numberFill(rv_neg_infinity);
}

DomainElement DomainElementFactory::positiveInfinity() {
    return DomainElementFactory::numberFill(rv_infinity);
}

DomainElement DomainElementFactory::zero() {
    return DomainElementFactory::numberFill(0);
}
