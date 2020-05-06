#ifndef __BASE_HH_
#define __BASE_HH_

#include <vector>
#include <list>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include "EMSet.hh"
#include "Chronicle.hh"
#include "Event.hh"
#include "MSetOcc.hh"

class Base {
	public :
		int classe = 0;
	
	protected :

		void
		getWindow (
                const Chronicle& c,
                const int& e,
                const std::vector<DomainElement>& match,
                const DomainElement& cwm,
                DomainElement& start,
                DomainElement& end,
                int&,
                std::vector<DomainElement>& stop
		) const;

		void
		getWindow (
                const Chronicle& c,
                const int& e,
                const std::vector<Event>& match,
                const DomainElement& cwm,
                DomainElement& start,
                DomainElement& end,
                int& pos,
                std::vector<DomainElement>& stop
		) const;

		virtual bool
		occur (
			const Chronicle& c,
			const int& seq,
			const int& cwm
		) const = 0;

	public :
		
		virtual
		std::vector<std::pair<DomainElement,int>>
		getOccurences(
			const int& i,
			const int& j
		) = 0;
		
		virtual
		std::vector<std::pair<DomainElement,int>>
		getIntOccurences(const int& i) = 0;
		
		bool
		frequent (
			Chronicle& c,
			const unsigned int& fmin,
			std::list<Chronicle>& frequents,
			bool& add,
			const bool& freq,
			const int& cwm
		) const;
		
		virtual unsigned int
		size() const = 0;
		
		virtual void
		initOccurences (
			const std::vector<bool>&,
			const int&,
			const int&,
			const bool&
		) {}
		
		virtual void
		clearOccurences () {}
		
		virtual
		unsigned int
		getSubBase (std::vector<bool>&, const int&, const int&) const {
			return 0;
		}

		virtual
		MSetOcc
		compileOccurrences(
				const std::vector<bool>&,
				const std::vector<int>&,
				const std::vector<int>&,
				const unsigned int&,
				std::vector<std::vector<std::vector<DomainElement>>>*,
				const bool,
				const bool
		) const {return MSetOcc({},{});}
	
		virtual	
		MSetOcc
		compileOccurrences(
			const std::vector<bool>&,
			const std::vector<int>&,
			const std::vector<int>&,
			const unsigned int&
		) const {return MSetOcc({},{});}

		virtual int getFreq(const EMSet&) const {return -1;}

		virtual int getMaxOcc() const {return 0;}
		virtual std::string generateLCMInputFile(int) const {return "";}

		virtual std::vector<EMSet>
		extractFrequentMSet (
			unsigned int,
			unsigned int,
			unsigned int,
			bool
		){return std::vector<EMSet>();}

		virtual
		~Base(){}
};

#endif
