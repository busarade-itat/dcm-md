#include <algorithm>
#include <map>

#include "MSetOcc.hh"

using namespace std;

std::string
MSetOcc::txt (const std::vector<std::string>& events, bool empty) const {
	ostringstream os;
	std::vector<std::pair<bool,bool>>* disc = nullptr;
	if (empty) disc = new std::vector<std::pair<bool,bool>>((_elements.size()*(_elements.size()-1))/2,std::pair<bool,bool>(false,false));
	if (_prefs.empty()) {
		std::vector<TCIterator> tcs;
		for (const auto &_occurrence : _occurrences) {
			TC *tc = new TC(_occurrence[0].first,
							_occurrence[_occurrence.size() - 1].first);
			tcs.emplace_back(std::shared_ptr<TC>(tc), true);
		}
		if (empty) tcs = std::vector<TCIterator>(disc->size());
		os << Chronicle(_elements, tcs, frequency, ofrequency).txt(events,disc);
	}

	for (auto pref : _prefs) {
		std::vector<TCIterator> tcs;
		for (unsigned i = 0; i < _occurrences.size(); i++) {
			unsigned begin = (pref.second)[i].first;
			unsigned end = (pref.second)[i].second;
			TC *tc = new TC(_occurrences[i][begin].first,
							_occurrences[i][end].first);

			tcs.emplace_back(std::shared_ptr<TC>(tc), true);
		}
		os << Chronicle(_elements, tcs, frequency, ofrequency).txt(events,disc);
	}

	delete disc;
	return os.str();
}

std::string
MSetOcc::json (const std::vector<std::string>& events, bool empty) const {

	if (empty) {
		std::stringstream os;
	
		int nb_e = (int) _elements.size();
		os << "{\"nodes\": [";
		for (int i = 0; i < nb_e; i++) {
			int e = _elements[i];
			os << R"({"id": ")" << i << R"(", "label": ")" << events[e] << "\"}";
			if (i+1 < nb_e) os << ",";
		}
	
		os << "],";
		os << "\"occurrences\": [";
	
		for (unsigned int i = 0; i < _occurrences.size(); i++) {
			os << "[";
			for (unsigned int j = 0; j < _occurrences[i].size(); j++) {
				os << "[\"" << _occurrences[i][j].first << "\",\"" << _occurrences[i][j].second << "\"]";
				if (j < (_occurrences[i].size()-1)) os << ",";
			}
			os << "]";
			if (i < _occurrences.size()-1) os << ",";
		}
		os << "], \"prefs\": {";
		bool start = true;
		for (std::pair<std::string, std::vector<std::pair<unsigned int, unsigned int>>> pref : _prefs) {
			if (start) start = false;
			else os << ",";
			os << "\"" << pref.first << "\": [";
				for (unsigned int i = 0; i < pref.second.size(); i++){
					os << "[\"" << pref.second[i].first << "\",\"" << pref.second[i].second << "\"]";
					if (i < pref.second.size()-1) os << ","; 
				}
			os << "]";
		}
		os << R"(}, "frequency": ")" << frequency << "\"}" << std::endl;
	
		return os.str();
	}

	ostringstream os;

	if (_prefs.empty()) {
		std::vector<TCIterator> tcs;
		for (const auto &_occurrence : _occurrences) {
			TC *tc = new TC(_occurrence[0].first,
							_occurrence[_occurrence.size() - 1].first);
			tcs.emplace_back(std::shared_ptr<TC>(tc), true);

		}
		os << Chronicle(_elements, tcs, frequency, ofrequency).json(events);
	}

	for (auto pref : _prefs) {
		std::vector<TCIterator> tcs;
		for (unsigned i = 0; i < _occurrences.size(); i++) {

			unsigned begin = (pref.second)[i].first;
			unsigned end = (pref.second)[i].second;

			TC *tc = new TC(_occurrences[i][begin].first,
							_occurrences[i][end].first);
			tcs.emplace_back(std::shared_ptr<TC>(tc), true);

		}

		os << Chronicle(_elements, tcs, frequency, ofrequency).json(events);
	}
	return os.str();
}
