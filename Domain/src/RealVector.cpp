#include <iostream>

#include "RealVector.hh"

RealVector RealVector::operator+ (const RealVector& that) const {
    if (size() != that.size()) {
        throw std::domain_error("RealVector to compare has differing size.");
    }
    else {
        RealVector out;

        for (unsigned long i = 0; i < size(); ++i) {
            out.push_back(at(i) + that.at(i));
        }

        return out;
    }
}

RealVector RealVector::operator- (const RealVector& that) const {
    if (size() != that.size()) {
        throw std::domain_error("RealVector to compare has differing size.");
    }
    else {
        RealVector out;

        for (unsigned long i = 0; i < size(); ++i) {
            out.push_back(at(i) - that.at(i));
        }

        return out;
    }
}

bool RealVector::isInsideBounds(const RealVector &lowerBound, const RealVector &upperBound) const {
    if (size() != lowerBound.size() || size() != upperBound.size()) {
        throw std::domain_error("TimeVectors to compare have differing size.");
    }
    else {
        for (unsigned long i = 0; i < size(); ++i) {
            if (!RealVector::isInInterval(at(i), lowerBound.at(i), upperBound.at(i))) {
                return false;
            }
        }

        return true;
    }
}

bool RealVector::isInInterval(double item, double lowerBound, double upperBound) {
    return item >= lowerBound && item <= upperBound;
}

RealVector::RealVector() {
    // blank intentionally
}

std::ostream& operator<<(std::ostream& os, const RealVector& tv) {
    os << "<";

    for (unsigned long i = 0; i < tv.size(); ++i) {
        os << tv.at(i);

        if (i < tv.size()-1) {
            os << ",";
        }
    }

    os << ">";

    return os;
}
