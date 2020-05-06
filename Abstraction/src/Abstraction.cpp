#include "Abstraction.hh"

void
Abstraction::addPunct (
	std::vector<DomainElement>& vec_s,
	const int&,
	const DomainElement& start
) {
	vec_s.push_back(start);
}

void
Abstraction::addPunct (
	std::vector<Event>& vec_s,
	const int& e1,
	const DomainElement& start
) {
	vec_s.push_back(Event(e1,start,start));
}