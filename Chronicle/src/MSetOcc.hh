#ifndef __MSETOCC__
#define __MSETOCC__

#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <iostream>
#include <utility>

#include "Chronicle.hh"
#include "MultiRule.hh"

class MSetOcc {
	public:
		unsigned int frequency;
		int ofrequency;
		int classe;

	private: 
		std::vector<int> _elements;
		std::vector<std::vector<std::pair<DomainElement,unsigned int>>> _occurrences;
		std::map<std::string, std::vector<std::pair<unsigned int, unsigned int>>> _prefs;
	
	public:
		MSetOcc(
				std::vector<int> e,
				std::vector<std::vector<std::pair<DomainElement, unsigned int>>> occ
		):
		frequency(0),
		ofrequency(-1),
		classe(0),
		_elements(std::move(e)),
		_occurrences(std::move(occ))
		{}

		std::string
		txt (const std::vector<std::string>& events, bool empty) const;

		std::string
		json (
			const std::vector<std::string>& events,
			bool empty
		) const;

		const std::vector<int>&
		getElements() {
			return _elements;
		}
};

#endif
