#include "TC.hh"

using namespace std;

std::ostream&
operator<< (std::ostream& out, TC& tc) {
	out << string(tc);
	return out;
}

TC::TC (const TC& tc) : _pleft(tc._pleft), _pright(tc._pright), _left(tc._left), _right(tc._right) {
	if (tc._start != nullptr) _start = new DomainElement(*(tc._start));
	else _start = nullptr;
	
	if (tc._end != nullptr) _end = new DomainElement(*(tc._end));
	else _end = nullptr;			
}

TC::operator std::string() const {
	std::stringstream res;
	res << "[";
	if (isStart()) res << (*_start);
	else res << "-inf";
	res << ",";
	if (isEnd()) res << (*_end);
	else res << "+inf";
	res << "]";
	return res.str();
} 

bool
operator<(
	const std::shared_ptr<TC>& tc1,
	const std::shared_ptr<TC>& tc2
) {
	if ((tc1->getEnd() - tc1->getStart()) > (tc2->getEnd() - tc2->getStart()))
		return true;
	else if ((tc1->getEnd() - tc1->getStart()) < (tc2->getEnd() - tc2->getStart()))
		return false;
	else return tc1->getStart() < tc2->getStart(); 
}

bool
operator==(
	const std::shared_ptr<TC>& tc1,
	const std::shared_ptr<TC>& tc2
) {
	return ((tc1->getEnd() == tc2->getEnd()) && (tc1->getStart() == tc2->getStart()));
}
