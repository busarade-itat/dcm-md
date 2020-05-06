#ifndef __TC_ITERATOR_HH_
#define __TC_ITERATOR_HH_

#include "TC.hh"

class TCIterator {

	protected :
	
		std::shared_ptr<TC> _constraint;
		bool _punct;
		
	public :
	
		TCIterator() : _constraint(nullptr), _punct(true) {}
	
		TCIterator (
			const std::shared_ptr<TC>& tc,
			const bool& punct
		) : _constraint(tc), _punct(punct)
		{}
		
		TC* operator->() const {return _constraint.operator->();}
		TC& operator*() const {return (*_constraint);}
		TCIterator& operator=(const std::shared_ptr<TC>& tc) {
			_constraint = tc;
			return *this;
		}
		
		bool operator==(const TCIterator& tci) const {
			return _constraint == tci._constraint;
		}
		
		bool isNull() const {
			return _constraint == nullptr;
		}
		
		const bool& isPunct() const {
			return _punct;
		}
};

#endif
