#ifndef __CDA_HH_
#define __CDA_HH_

#include <iostream>
#include <cstdlib>

#include <algorithm>
#include <list>
#include <set>
#include <map>
#include <vector>

#include <cmath>

#include "Chronicle.hh"
#include "TCIterator.hh"
#include "Event.hh"
#include "Base.hh"
#include "MSetOcc.hh"
#include "EMSet.hh"

#include "ripper.hh"

#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

/**
 * Chronicle Discovery Algorithm class
 */
class CDA {
		
	public :

		bool stop; //!< Initialized to false, switch stop to true will break the main run(...) loop.
		bool next;
		int nb_tc;
		bool extract_with_tid;

		std::vector<int> stats;
		
		/*
			Is used to generated json chronicle during extraction if not empty.
		 */
		std::vector<std::string> inv_events;
		bool txt= false;
		
	protected :
		
		/**
		 * Run the CDA algorithm on a specific events' set. /!\ Old function !
		 * @param chron The chronicle to specify.
		 * @param trace The sequences set.
		 * @param fmin The minimal frequency threshold.
		 * @param frequents The set of frequents chronicles to return.
		 * @param p True if chronicle added to the frequents set are printed during the execution.
		 * @param close True if frequents is a frequent closed chronicles set, false if it's a frequent minimal chronicles' set.
		 * @param freq True to force the frequency count of each chronicle when close is false.
		 */
		void
		run (
			const Chronicle& chron,
			const Base& trace,
			unsigned int fmin,
			std::list<Chronicle>& frequents,
			bool p,
			int cwm,
			bool close,
			bool freq
		);

		/**
		 * Bind each discriminant multiset with his occurrences and print it if necessary.
		 * If the occurrences are not printed or the multiset not used later, the computation of all occurrences can be
		 * removed disable to improve the computation speed.
		 * @param set The discriminant multisets.
		 * @param trace The base from which we should extract the occurrences.
		 * @param nb_se The number of events in the base.
		 * @param res The vector in which the multisets are put if not printed.
		 */
		void
		emergeMSetOcc (
			const std::vector<EMSet>& set,
			const Base& trace,
			unsigned int nb_se,
			std::vector<MSetOcc>& res
		);

		/**
		 * Return the subset of discriminant multisets in a given set of multisets.
		 * @param set1 The given set of multisets.
		 * @param trace The negative base to compute the negative support.
		 * @param bset A vector to store the non discriminant multiset.
		 * @param gmin The minimal growth rate.
		 * @param mincs The minimal size threshold.
		 * @param maxcs The maximal size threshold (0 for infinity).
		 * @return std::vector<EMSet> the vector containing the discriminant multisets.
		 */
		std::vector<EMSet>
		emergeMSet (
			std::vector<EMSet>& set1,
			const Base& trace,
			std::vector<EMSet>* bset,
			const double& gmin,
            unsigned int mincs,
            unsigned int maxcs
		);
		
		/**
		 * Add a chronicle in a minimal (according to Cram's definition) chronicles' set.
		 * @param s The set.
		 * @param c The chronicle to add.
		 * @param close True if the set is a frequent closed chronicles set, False for minimal chronicle.
		 */
		void
		minAdd (
			std::list<Chronicle>& s,
			const Chronicle& c,
			bool
		);

		/**
		 * Run the CDA algorithm on the specialized children (same elements) of a chronicle (to avoid the use of a children' set).
		 * @param c The chronicle.
		 * @param trace The sequences' base.
		 * @param fmin The minimum frequency threshold.
		 * @param frequents A set of frequents chronicles.
		 * @param p True if chronicle added to the frequents set are printed during the execution.
		 * @param close True if frequents is a frequent closed chronicles' set, false if it's a frequent minimal chronicles' set.
		 * @param freq True to force the frequency count of each chronicle when close is false.
		 */
		void
		runSpecChild(
			const Chronicle& c,
			const Base& trace,
			unsigned int fmin,
			std::list<Chronicle>& frequents,
			bool p,
			int cwm,
			bool close,
			bool freq
		);
		
		/**
		 * Generate extension of a chronicle with element's adding.
		 * @param c The chronicle to extend.
		 * @param base The temporal constraints' base used to add the element.
		 * @param nb_e The number of element represented in the base.
		 * @return The set of extended chronicles.
		 */
		std::vector<Chronicle>
		generateExtendChild(
			const Chronicle& c,
			const std::vector<std::shared_ptr<TC>>& base_pct,
			const std::vector<std::shared_ptr<TC>>& base_itv,
			unsigned int nb_e,
			unsigned int nb_se,
			bool dif,
			bool itv_only
		);
		
		bool
		OccFreq (
			const std::vector<std::pair<DomainElement,int>>& Ak,
			unsigned int fmin,
			unsigned int nb_seq,
			int l,
			int k
		);

		std::shared_ptr<TC>
		G(
            const std::vector<std::pair<DomainElement,int>>& Ak,
            unsigned int fmin,
            unsigned int nb_seq
        );

		std::shared_ptr<TC>
		G(
			const std::shared_ptr<TC>& father,
			bool left,
			const std::vector<std::pair<DomainElement,int>>& Ak,
			unsigned int fmin,
			unsigned int nb_seq,
			int l,
			int k
		);


		/**
		 * Test if a rule constraining a multiset is really discriminant for g and, if true, print the corresponding
		 * discriminant chronicle.
		 * @param s The object corresponding to the non discriminant multiset.
		 * @param positiveData The positive dataset.
		 * @param negativeData The negative dataset.
		 * @param r The rule.
		 * @param f The minimal support threshold.
		 * @param g The minimal growth rate threshold.
		 * @param ratio The ratio to use if the two datasets have different sizes.
		 */
		void
		testSub(
                MSetOcc& s,
                const std::vector<std::vector<std::vector<DomainElement>>>& positiveData,
                const std::vector<std::vector<std::vector<DomainElement>>>& negativeData,
                const MultiRule& r,
                int f,
                double g,
                double ratio
		) const;

	public :
		CDA():stop(false),next(false),nb_tc(0),extract_with_tid(false){}

		/**
		 * Run the CDA algorithm (frequent chronicles).
		 * @param trace The base of sequences to mine.
		 * @param base The sets of temporal constraints' graph.
		 * @param init The initial chronicle to extend, NULL if there is no chronicle to extend.
		 * @param fmin The minimal frequency threshold for the chronicles.
		 * @param nb_e The elements' number.
		 * @param p True if the extracted chronicles must be printed while the execution.
		 * @param close True if the extracted chronicles are closed chronicles, false if they are minimal.
		 * @param freq True to force the frequency count of each chronicle when close is false.
		 * @return The extracted chronicles.
		 */
		std::vector<Chronicle>
		run (
			const Base& trace,
			const std::vector<std::shared_ptr<TC>>& base_pct,
			const std::vector<std::shared_ptr<TC>>& base_itv,
			const Chronicle* const init,
			unsigned int fmin,
			unsigned int nb_e,
			unsigned int nb_se,
			bool p,
			int cwm,
			unsigned int mincs,
			unsigned int maxcs,
			bool close,
			bool dif,
			bool itv_only,
			bool freq
		);

		/**
		 * Run the DCM algorithm (discriminant chronicles).
		 * @param trace1 positive sequences base.
		 * @param trace2 negative sequences base.
		 * @param fmin minimal support threshold.
		 * @param gmin minimal growth threshold.
		 * @param nb_e number of events.
		 * @param mincs minimal size threshold (number of events).
		 * @param maxcs maximal size threshold.
		 * @param print use verbose or not.
		 * @param closed use closed itemsets.
		 * @param irep use irep (ordered rules).
		 * @return bool True if the it is the end of the parent execution else false.
		 */
		bool
		run (
			Base& trace1,
			Base& trace2,
			const unsigned int fmin,
			const double gmin,
			const unsigned int nb_e,
            const unsigned int mincs,
            const unsigned int maxcs,
			const bool print,
			const bool closed,
			const bool use_episode,
			const bool irep
		);

		/**
		 * Generate the temporal constraints' bases.
		 * @param trace The base of sequences to mine.
		 * @param nb_e The elements' number.
		 * @param nb_se The non-interval elements' number.
		 * @param fmin The minimal frequency threshold for the chronicles.
		 * @param cwm The maximal window threshold.
		 * @param base_pct The inter-event temporal constraints' base to update.
		 * @param base_itv The intra-event temporal constraints' base to update /!\ Not used for the moment /!\.
		 */
		void
		CCDC(
			Base* trace,
			std::vector<bool> conclusion,
			unsigned int nb_e,
			unsigned int nb_se,
			unsigned int fmin,
			int cwm,
			bool dif,
			bool itv_only,
			std::vector<std::shared_ptr<TC>>& base_pct,
			std::vector<std::shared_ptr<TC>>& base_itv
		);

		/**
		 * Calculate the index of a temporal constraints' graph in a base.
		 * @param i The first event of the graph' constraints.
		 * @param j The second event.
		 * @param nb_e The total events' number.
		 * @return The index.
		 */
		static int
		indice (
			const int& i,
			const int& j,
			const int& nb_e
		){return j + i * (nb_e-1) - (i*(i-1))/2;}
};

#endif
