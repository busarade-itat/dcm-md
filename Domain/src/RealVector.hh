#ifndef REALVECTOR_HH
#define REALVECTOR_HH

#include <vector>

class RealVector: public std::vector<double> {
public:
    RealVector();

    RealVector operator+ (const RealVector& that) const;
    RealVector operator- (const RealVector& that) const;
    // "==" operator is inherited from std::vector and will work

    bool isInsideBounds(const RealVector& lowerBound, const RealVector& upperBound) const;
protected:
    static bool isInInterval(double item, double lowerBound, double upperBound);
};

std::ostream& operator<<(std::ostream& os, const RealVector& tv);

#endif
