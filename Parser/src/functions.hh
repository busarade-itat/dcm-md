#ifndef __FUNCTIONS_HH_
#define __FUNCTIONS_HH_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <string>
#include <memory>

#include <regex>

#include "CDA.hh"

void
LineParser(
		std::ifstream& fichier,
		std::map<int,int>& patients,
		std::map<std::string, int>& events,
		std::vector<std::string>& code_events,
		std::vector<std::vector<std::vector<DomainElement>>>& pathway_events,
        const unsigned int& vector_size
);

void
print (
	const std::vector<std::shared_ptr<TC>>& base,
	const int& nb_e
);

void
print (
	std::shared_ptr<TC> tc,
	const int& indent,
	const bool& left
);

#endif
