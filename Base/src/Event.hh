#ifndef __EVENT_HH_
#define __EVENT_HH_

#include <iostream>
#include <cmath>
#include "../../Domain/src/DomainElement.hh"
#include "../../Domain/src/DomainElementFactory.hh"

class Event {
	
	protected :
		int _element;
        DomainElement _time;
        DomainElement _end_time;
		
	public :
		Event() :
		    _element(0),
		    _time(DomainElementFactory::zero()),
		    _end_time(DomainElementFactory::zero()) {}
	
		Event(const int& e, const DomainElement& t) : _element(e), _time(t), _end_time(t) {}
		Event(const int& e, const DomainElement& t, const DomainElement& et) : _element(e), _time(t), _end_time(et) {}
		const int& getElement() const {return _element;}
		const DomainElement& getTime() const {return _time;}
		const DomainElement& getOtherTime() const {return _end_time;}

		bool operator< (const Event& e) const {
			return _time < e._time;
		}
};

#endif
