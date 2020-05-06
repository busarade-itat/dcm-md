#ifndef __TC_HH_
#define __TC_HH_

#include <cstdlib>
#include <string>
#include <sstream>
#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>
#include "../../Domain/src/DomainElement.hh"

const int LEFT = 0;
const int RIGHT = 1;

class TC {

	public :
		std::shared_ptr<TC> _pleft;
		std::shared_ptr<TC> _pright;
		
		std::shared_ptr<TC> _left;
		std::shared_ptr<TC> _right;

	protected :
        DomainElement* _start;
        DomainElement* _end;
	
	public :
		
		friend std::ostream& operator<< (std::ostream& out, TC& tc);
		
		TC (const TC& tc);
	
		TC (const DomainElement& s, const DomainElement& e) :
                _pleft(nullptr), _pright(nullptr), _left(nullptr), _right(nullptr),
                _start(new DomainElement(s)), _end(new DomainElement(e)) {}

		TC (DomainElement* s, const DomainElement& e) :
		    _pleft(nullptr), _pright(nullptr), _left(nullptr), _right(nullptr),
		    _start(s), _end(new DomainElement(e)) {}

		TC (const DomainElement& s, DomainElement* e) :
                _pleft(nullptr), _pright(nullptr), _left(nullptr), _right(nullptr),
                _start(new DomainElement(s)), _end(e) {}

		TC (DomainElement* s, DomainElement* e) :
		    _pleft(nullptr), _pright(nullptr), _left(nullptr), _right(nullptr), _start(s), _end(e) {}

		~TC() {
			delete _start;
			delete _end;
		}
		
		bool isStart() const {return _start != nullptr;}
		bool isEnd() const {return _end != nullptr;}
		const DomainElement& getStart() const {return *_start;}
		const DomainElement& getEnd() const {return *_end;}

		operator std::string() const; 
};

#endif
