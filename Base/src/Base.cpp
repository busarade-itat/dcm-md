#include "Base.hh"
#include "../../Domain/src/DomainElement.hh"

bool
Base::frequent (
	Chronicle& c,
	const unsigned int& fmin,
	std::list<Chronicle>&,
	bool&,
	const bool& freq,
	const int& cwm
) const {
	unsigned int oc = 0;
	int s = size();
	
	for (int i = 0; i < s; i++) {
		if (occur(c, i, cwm)) {
			oc++;
			if (!freq && oc == fmin)
				return true;
		}
	}
	
	if (oc >= fmin) {
		if (freq)
			c.frequency = oc;
		return true;
	}
	
	return false;
}

void
Base::getWindow (
		const Chronicle& c,
		const int& e,
		const std::vector<DomainElement>& match,
		const DomainElement& cwm,
		DomainElement& start,
		DomainElement& end,
		int&,
		std::vector<DomainElement>& stop
) const {

    DomainElement time;

	for (int x = 0; x < e; x++) {

		time = match[x];
		if (cwm > DomainElementFactory::zero()) {
			if (time - cwm > start)
				start = time - cwm;
			if (time + cwm < end)
				end = time + cwm;
		}

		if (c.getElement(x) == c.getElement(e))
			stop.push_back(time);

		const TCIterator p_constraint = c.getConstraint(c.indice(x,e));

		if (!p_constraint.isNull()) {

            DomainElement constraint_s = start;
            DomainElement constraint_e = end;
			if (p_constraint->isStart()) constraint_s = p_constraint->getStart() + time;
			if (p_constraint->isEnd()) constraint_e = p_constraint->getEnd() + time;

			if (constraint_s > start) start = constraint_s;
			if (constraint_e < end) end = constraint_e;
		}
	}

	std::sort(stop.begin(),stop.end());
}

void
Base::getWindow (
		const Chronicle& c,
		const int& e,
		const std::vector<Event>& match,
		const DomainElement& cwm,
		DomainElement& start,
		DomainElement& end,
		int& pos,
		std::vector<DomainElement>& stop
) const {

    DomainElement time;

	for (int x = 0; x < e; x++) {

		Event el_x = match[x];

		time = el_x.getTime();
		if (cwm > DomainElementFactory::zero()) {
			if (time - cwm > start)
				start = time - cwm;
			if (time + cwm < end)
				end = time + cwm;
		}

		if (el_x.getElement() == c.getElement(e))
			stop.push_back(time);

		const TCIterator p_constraint = c.getConstraint(c.indice(x,e));

		if (!p_constraint.isNull()) {

			DomainElement constraint_s = start;
            DomainElement constraint_e = end;

			if (p_constraint->isStart()) constraint_s = p_constraint->getStart() + time;
			if (p_constraint->isEnd()) constraint_e = p_constraint->getEnd() + time;

			if (constraint_s > start) start = constraint_s;
			if (constraint_e < end) end = constraint_e;

			if (!p_constraint.isPunct())
				pos = x;
			else if (el_x.getElement()+1 == c.getElement(e) && el_x.getOtherTime() > el_x.getTime())
				stop.push_back(el_x.getOtherTime());
		}
	}

	std::sort(stop.begin(),stop.end());
}
