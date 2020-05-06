#ifndef __ABSTRACTION_HH_
#define __ABSTRACTION_HH_

#include <string>
#include <vector>
#include <map>

#include <algorithm>

#include "Event.hh"

/**
 * This class or at least its should not exist in this version of code.
 * Its last role is to generate the pathways for DCM.
 */
class Abstraction {

	private :

	static void
	addPunct (
		std::vector<DomainElement>& vec_s,
		const int&,
		const DomainElement& start
	);

	static void
	addPunct (
		std::vector<Event>& vec_s,
		const int& e1,
		const DomainElement& start
	);
	
	public :
	
		Abstraction() = delete;

		template <typename T>
		static void
		generatePathways(
				unsigned int event_size,
				std::vector<std::vector<std::vector<T>>>& pathway,
				std::vector<std::vector<std::vector<DomainElement>>>& pathway_events
		);
};

template <typename T>
void
Abstraction::generatePathways(
	unsigned int event_size,
	std::vector<std::vector<std::vector<T>>>& pathway,
	std::vector<std::vector<std::vector<DomainElement>>>& pathway_events
) {
	std::vector<int> sep;

	pathway = std::vector<std::vector<std::vector<T>>>
			(event_size,
			 std::vector<std::vector<T>>(pathway_events[0].size(),
										 std::vector<T>()));

	for (unsigned int i = 0; i < pathway_events.size(); i++) {
		for (unsigned int j = 0; j < pathway_events[i].size(); j++) {

			if (pathway_events[i][j].size() > 0) {
				sep.clear();

				sep.push_back(-1);
				sep.push_back((int) pathway_events[i][j].size() - 1);

				for (unsigned int k = 1; k < sep.size(); k++) {
					for (int l = sep[k - 1] + 1; l <= sep[k]; l++) {
						addPunct(
						        pathway[i][j],
						        i,
						        pathway_events[i][j][l]
                        );
					}
				}

			}
		}
	}
}

#endif
