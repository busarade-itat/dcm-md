#ifndef DOMAINELEMENTFACTORY_HH
#define DOMAINELEMENTFACTORY_HH

#include "DomainElement.hh"

class DomainElementFactory {
public:
    static unsigned int VECTOR_SIZE;

    static DomainElement zero();
    static DomainElement positiveInfinity();
    static DomainElement negativeInfinity();

protected:
    static DomainElement numberFill(double number);
};


#endif
