#include "functions.hh"

void
LineParser(
		std::ifstream& fichier,
		std::map<int,int>& patients,
		std::map<std::string, int>& events,
		std::vector<std::string>& code_events,
		std::vector<std::vector<std::vector<DomainElement>>>& pathway_events,
        const unsigned int& vector_size
) {
	std::string line;
	getline(fichier,line);

	int sid = 0;
	std::string label;
    DomainElement dates;

	int indice;
	int e;

	std::map<int, int>::iterator it_p;
	std::map<std::string, int>::iterator it_e;

	while (!fichier.eof()) {
		std::istringstream str_line(line);

		while (str_line.good()) {
		    double dateBuffer;
            dates.clear();

			str_line >> label;

			for (int i = 0; i < vector_size; ++i) {
                str_line >> dateBuffer;

                dates.push_back(dateBuffer);
            }

			if (str_line) {
				if ((it_e = events.find(label)) != events.end())
					e = it_e->second;
				else {
					e = (int) events.size();
					events[label] = e;
					code_events.push_back(label);
				}

				while (pathway_events.size() <= (unsigned) e)
					pathway_events.push_back(std::vector<std::vector<DomainElement>>(patients.size(), std::vector<DomainElement>()));

				if ((it_p = patients.find(sid)) != patients.end())
					indice = it_p->second;
				else {
					indice = (int) patients.size();
					patients[sid] = indice;
				}

				while (pathway_events[e].size() <= (unsigned) indice)
					pathway_events[e].push_back(std::vector<DomainElement>());

                pathway_events[e][indice].push_back(dates);
			}
		}

		getline(fichier, line);
		sid++;
	}

	for (unsigned int i = 0; i < pathway_events.size(); i++) {
		while (pathway_events[i].size() < patients.size())
			pathway_events[i].push_back(std::vector<DomainElement>());
	}
}

void
print (const std::vector<std::shared_ptr<TC>>& base, const int& nb_e) {

	for (int i = 0; i < nb_e; i++) {
		for (int j = i; j < nb_e; j++) {
		
			std::cout << "G_{"<<i<<","<<j<<"} :" << std::endl;
			print(base[CDA::indice(i,j,nb_e)],0,true);
			
			if (i != j || i+1 < nb_e) std::cout << std::endl;
		}
	}
}

void
print (const std::shared_ptr<TC> tc, const int& indent, const bool& left) {

	if (tc != NULL) {
	
	if (indent > 0) std::cout << " " << (std::string((unsigned long) (2 * indent), '-')) << " ";
	else std::cout << " ";

		int i = indent+1;
		std::cout << (*tc) << std::endl;
		if (left) print(tc->_left, i, true);
		print(tc->_right, i, (tc->_left == NULL || tc->_left->_right == NULL));
	}	
}
