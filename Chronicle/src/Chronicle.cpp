#include "../../Domain/src/DomainElementFactory.hh"
#include "Chronicle.hh"

using namespace std;

int Chronicle::nextId = 0;

ostream& operator<< (ostream& out, Chronicle& c) {
	out << string(c);
	return out;
}

int
Chronicle::str_indice (
	const int& i,
	const int& j
) const {
	return (int) (i * _elements.size() - ((i + 1) * i) / 2 + j - i - 1);
}

string
Chronicle::str (vector<string> events) const {
	int nb_e = (int) _elements.size();
	ostringstream res;
	
	res << "E : {";
	for (int i = 0; i < nb_e; i++) {
		
		if (i > 0) res << ", ";
		res << events[_elements[i]];
	}
	res << "}" << endl;
	
	for (int i = 0; i < nb_e; i++) {
		for (int j = i+1; j < nb_e; j++) {
			if (i > 0 || j > 1) res << ", ";
			res << (*_constraints[str_indice(i,j)]);
		}
	}
	res << "}";
	
	return res.str();
}

std::string
Chronicle::json(const std::vector<std::string> &events) const {
	return json(events, nullptr);
}

std::string
Chronicle::txt (
		const std::vector<std::string>& events,
		std::vector<std::pair<bool,bool>>* disc
) const {
	std::stringstream os;

	int nb_e = size();
	os << "C: {[";
	for (int i = 0; i < nb_e; i++) {
		int e = getElement(i);
		os << "\""<<events[e]<<"\"";
		if (i+1 < nb_e) os << ", ";
	}
	os << "]}\n";

	for (int i = 0; i < nb_e; i++) {
		for (int j = i+1; j < nb_e; j++) {
			const TCIterator tc = getConstraint(str_indice(i,j));
			os << i << ", " << j << ": (";

			if (disc != nullptr) {
				if ((*disc)[str_indice(i, j)].first) {
				    os << tc->getStart();
				}
				else {
				    os << DomainElementFactory::negativeInfinity();
				}

				os << ", ";

				if ((*disc)[str_indice(i, j)].second) {
				    os << tc->getEnd();
				}
				else {
                    os << DomainElementFactory::positiveInfinity();
				}
			}
			else
				os << tc->getStart() << ", " << tc->getEnd();
			os << ")\n";
		}
	}

	if (ofrequency == -1)
		os << "f: " << frequency << std::endl;
	else
		os << "f: " << frequency << "/" << ofrequency << std::endl;

	if (!tid_list_pos.empty()) {
		os << "positives: [" << tid_list_pos[0];
		for (unsigned i = 1; i < tid_list_pos.size(); i++) os << ", " << tid_list_pos[i];
		os << "]" << std::endl;
	}
	if (!tid_list_neg.empty()) {
		os << "negatives: [" << tid_list_neg[0];
		for (unsigned i = 1; i < tid_list_neg.size(); i++) os << ", " << tid_list_neg[i];
		os << "]" << std::endl;
	}

	return os.str();
}

std::string
Chronicle::json (
		const std::vector<std::string>& events,
		std::vector<std::pair<bool,bool>>* disc
) const {
	
	std::stringstream os;
	
	int nb_e = size();
	os << R"({ "id" : ")" << id << "\", ";
	os << "\"nodes\": [";
	for (int i = 0; i < nb_e; i++) {
		int e = getElement(i);
		os << R"({"id": ")" << i << R"(", "label": ")" << events[e] << "\"}";
		if (i+1 < nb_e) os << ",";
	}
	
	os << "],";
	os << "\"edges\": [";
	
	for (int i = 0; i < nb_e; i++) {
		for (int j = i+1; j < nb_e; j++) {
			const TCIterator tc = getConstraint(str_indice(i,j));
				os << R"({"source": ")" << i << R"(", "target": ")" << j << "\", ";
				os << R"("inf": ")" << tc->getStart() << R"(", "sup": ")" << tc->getEnd() << "\", ";
			if (disc != nullptr)
				os << R"("inf_dis": ")" << (*disc)[str_indice(i,j)].first << R"(", "sup_dis": ")" << (*disc)[str_indice(i,j)].second << "}";
	    	
			if (i+2 < nb_e || j+1 < nb_e) os << ",";
		}
	}
	os << "],";
	if (!tid_list_pos.empty()) {
		os << R"("positives": [")" << tid_list_pos[0] << "\"";
		for (unsigned i = 1; i < tid_list_pos.size(); i++) os << ", \"" << tid_list_pos[i] << "\"";
		os << "]," << std::endl;
	}
	if (!tid_list_neg.empty()) {
		os << R"("negatives": [")" << tid_list_neg[0] << "\"";
		for (unsigned i = 1; i < tid_list_neg.size(); i++) os << ", \"" << tid_list_neg[i] << "\"";
		os << "]," << std::endl;
	}
	if (ofrequency == -1)
		os << R"("frequency": ")" << frequency << "\"}" << std::endl;
	else
		os << R"("frequency": ")" << frequency << "/" << ofrequency << "\"}" << std::endl;
	return os.str();
}

Chronicle::operator std::string () const {
	int nb_e = (int) _elements.size();
	ostringstream res;
	
	res << "E : {";
	for (int i = 0; i < nb_e; i++) {
		
		if (i > 0) res << ", ";
		res << _elements[i];
	}
	res << "}" << endl;
	
	res << "Constraints : {";
	for (int i = 0; i < nb_e; i++) {
		for (int j = i+1; j < nb_e; j++) {
			if (i > 0 || j > 1) res << ", ";
			res << (*_constraints[str_indice(i,j)]);
		}
	}
	res << "}";
	
	return res.str();
}

int
Chronicle::indice (
		const int& i,
		const int& j
) {
	return j*(j-1)/2 +i;
}

