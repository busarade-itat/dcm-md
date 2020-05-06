#ifndef GENERAL_RULE_HH
#define GENERAL_RULE_HH

#include <vector>

template <class T>
class GeneralRule {
    protected:
        std::vector<int> _tcs;
        std::vector<std::pair<T*,T*>> _values;
        unsigned int _class;
        unsigned int frequency;
        unsigned int ofrequency;

    public:
        GeneralRule (
            int c,
            std::vector<int> tcs,
            std::vector<std::pair<T*,T*>> v,
            unsigned int f,
            unsigned int of
        ): _class(c), _tcs(tcs), _values(v), frequency(f), ofrequency(of) {};

        unsigned int getClass() const {return _class;}
        unsigned int getFrequency() const {return frequency;}
        unsigned int getOFrequency() const {return ofrequency;}
        const std::vector<int>& getConstraints() const {return _tcs;}
        const std::vector<std::pair<T*,T*>>& getValues() const {return _values;}
};

#endif
