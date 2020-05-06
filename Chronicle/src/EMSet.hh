#ifndef __EMSET_HH_
#define __EMSET_HH_

#include <vector>
#include <iostream>

class EMSet {

	protected:
		
		std::vector<int> _mset;
		std::vector<bool> _sub;
		unsigned int _f;
		unsigned int _of;
		
	public:
	
		EMSet(
			const std::vector<int>& mset,
			const std::vector<bool>& sub,
			const unsigned int& f
		) : _mset(mset), _sub(sub), _f(f), _of(0) {}

		EMSet(
			const std::vector<int>& mset,
			const std::vector<bool>& sub,
			const unsigned int& f,
			const unsigned int& of
		) : _mset(mset), _sub(sub), _f(f), _of(of) {}
		
		std::vector<int>
		getShortMSet() const {
			std::vector<int> res;		
			for (unsigned int i = 0; i < _mset.size(); i++) {
				for (int j = 0; j < _mset[i]; j++)
					res.push_back(i);
			}
			return res;
		}
		
		bool
		operator<(const EMSet& e) const {
			if (_mset.size() == e._mset.size()) {
				for (unsigned int i = 0; i < _mset.size(); i++) {
					if (_mset[i] < e._mset[i]) return true;
					else if (_mset[i] > e._mset[i]) return false;
				}
			}
			return false;
		}

		bool
		operator==(const EMSet& e) const {
			if (_mset.size() != e._mset.size()) return false;
			for (unsigned int i = 0; i < _mset.size(); i++) if (_mset[i] != e._mset[i]) return false;
			return true;
		}

		const std::vector<int>&
		getMSet() const {return _mset;}
		
		const std::vector<bool>&
		getSub() const {return _sub;}
		
		unsigned int
		getFreq() const {return _f;}

		unsigned int
		getOFreq() const {return _of;}

		void
		setOFreq(unsigned int i){_of=i;}
};

#endif
