#ifndef _VERTICAL_EVENT_BASE__
#define _VERTICAL_EVENT_BASE__

#include <vector>
#include <algorithm>
#include <iostream>
#include <omp.h>
#include <ctime>
#include <map>
#include <sstream>
#include <cassert>
#include <cmath>
#include "Base.hh"


template <class T>
class VerticalEventBase : public Base {

	public :
        DomainElement _cwm;

	protected :
	
		unsigned int _size;
		std::vector<std::vector<std::vector<T>>> _base;
		std::vector<bool> _conclude;
		
		int _nb_se;
		
		bool
		occur (
			const Chronicle& c,
			const int& seq,
			const int& cwm
		) const {
			std::vector<T> match(c.size());
			return occur (c, seq, match, 0, cwm);
		}
				
		bool
		occur (
			const Chronicle& c,
			const int& seq,
			const std::vector<T>& match,
			const int& e,
			const int& cwm
		) const;
		
		unsigned int
		size() const {return _size;}

		int
		getNbEvent() const{return _base.size();}
		const DomainElement&
		getTime(const DomainElement& e) const {return e;}

		const DomainElement&
		getTime(const Event& e) const {return e.getTime();}

		const DomainElement&
		getOTime(const DomainElement& e) const {return e;}

		const DomainElement&
		getOTime(const Event& e) const {return e.getOtherTime();}
		
		bool
		getItv (
			const int& i,
			const int& j
		) const;		
		
		void
		prune (const int& fmin) {
			int nb_e = _base.size();
			int support;
			int f;
			
			for (unsigned int i = 0; i < (unsigned) nb_e; i++) {
				f = 0;
				support = _base[i].size();
				
				for (unsigned int j = 0; j < (unsigned) support; j++)
					if (_base[i][j].size() > 0) f++;
					
				if (f < fmin) _base[i].clear();
			}
		}
		
		std::vector<Event>
		getTimeline (const unsigned int& s) const {
			std::vector<Event> timeline;
			
			for (unsigned int i = 0; i < _base.size(); i++)
				if (_base[i].size() > s)
					timeline.insert(timeline.end(),_base[i][s].begin(),_base[i][s].end());
			
			std::sort (timeline.begin(), timeline.end());
			return timeline;
		}
		
		bool
		testInclude (
			const std::vector<int>& sub,
			const std::vector<int>& set
		) const {return testInclude (sub,set,0,0);}
		
		bool
		testInclude (
			const std::vector<int>& sub,
			const std::vector<int>& set,
			int sub_flag,
			int set_flag
			) const {
			if (sub_flag >= sub.size()) return true;
			if (sub.size() - sub_flag > set.size() - set_flag) return false;
			if (sub[sub_flag] == set[set_flag]) return testInclude(sub,set,sub_flag+1,set_flag+1);
			return testInclude(sub,set,sub_flag,set_flag+1);
		}
		
		void
		closeAdd (
			std::list<std::vector<int>>& set,
			const std::list<Event>& add
		) const {
			std::vector<int> episode;
			for (Event e : add)	episode.push_back(e.getElement());
			std::sort(episode.begin(),episode.end());
		
			std::vector<std::list<std::vector<int>>::iterator> del;
			bool closed = false;
			bool n_closed = false;
			
			for (std::list<std::vector<int>>::iterator it = set.begin(); it != set.end(); it++) {
				if (episode.size() <= it->size() && !closed) {
					if (testInclude(episode, *it)) {
						n_closed = true;
						break;
					}
				}
				else if (episode.size() > it->size()) {
					if (testInclude(*it, episode)) {
						closed = true;
						del.push_back(it);
					}
				}
			}
			
			if (!n_closed) {
				set.push_back(episode);
				for (std::list<std::vector<int>>::iterator it : del) set.erase(it);
			}
		}
		
	public :
	
		VerticalEventBase(const std::vector<std::vector<std::vector<T>>>& base);
		VerticalEventBase(
			const std::vector<std::vector<std::vector<T>>>& base,
			const int& fmin
		) : VerticalEventBase(base)
		{prune(fmin);}
		
		std::vector<std::pair<DomainElement,int>>
		getOccurences(
			const int& i,
			const int& j
		);
		
		std::vector<std::pair<DomainElement,int>>
		getIntOccurences(
			const int& i
		);
		
		void
		initOccurences (
			const std::vector<bool>& conclusion,
			const int& nb_se,
			const DomainElement& cwm,
			const bool&
		) {
			_conclude = conclusion;
			_cwm = cwm;
			_nb_se = nb_se;
		}
		
		void
		clearOccurences () {
			_conclude.clear();
		}
		
		std::vector<int>
		getClosedEpisode (const std::vector<bool>& sub) const {
			std::vector<int> closed (_base.size(),0);
			
			int t;
			int card = _base.size();
			int support;
			int occ;
			
			for (int i = 0; i < card; i++) {
				t = -1;
				support = _base[i].size();
				for (int j = 0; j < support; j++) {
					if (sub[j]) {
						occ = _base[i][j].size();
						if (occ == 0) {
							t = -1;
							break;
						}
						else if (occ < t || t == -1) t = occ;	
					}
				}
				if (t > 0) closed[i] = t;
			}
			
			return closed;
		}
		
		unsigned int
		getSubBase (std::vector<bool>& sub, const int& e, const int& n) const {
			int res = 0;
			int support = _base[e].size();
			
			for (int i = 0; i < support; i++)
				if (sub[i]) {
				 if (_base[e][i].size() < (unsigned) n) {
                     sub[i] = false;
                 }
				 else res++;
				}
				
			return res;
		}
		
		void
		checkFreq (std::vector<bool>& freq, const int& fmin) const {
			int nb_e = freq.size();
			int f;
			int support;
			
			for (unsigned int i = 0; i < nb_e; i++) {
				if (freq[i]) {
					f = 0;
					support = _base[i].size();
					
					for (unsigned int j = 0; j < support; j++) {
						if (_base[i][j].size() > 0) {
							f++;
							if (f >= fmin) break;
						}
					}
					
					if (f < fmin) freq[i] = false;
				}
			}
		}

        MSetOcc
        compileOccurrences(
                const std::vector<bool>& sub,
                const std::vector<int>& cepisode,
                const std::vector<int>& episode,
                const unsigned int& nb_se
        ) const {
            return compileOccurrences(sub, cepisode, episode, nb_se, NULL, false, false);
        }

        MSetOcc
        compileOccurrences(
                const std::vector<bool>& sub,
                const std::vector<int>& cepisode,
                const std::vector<int>& episode,
                const unsigned int& nb_se,
                std::vector<std::vector<std::vector<int>>>* data
        ) const {
            return compileOccurrences(sub,cepisode,episode,nb_se,data,false);
        }

		MSetOcc
		compileOccurrences(
			const std::vector<bool>& sub,
			const std::vector<int>& cepisode,
			const std::vector<int>& episode,
			const unsigned int& nb_se,
			std::vector<std::vector<std::vector<DomainElement>>>* data,
			const bool use_episode,
            const bool p
		) const {
			std::vector<std::vector<std::pair<DomainElement,unsigned int>>> occurrences ((episode.size() * (episode.size() - 1)) / 2);

			std::vector<DomainElement> t;
			if (data != NULL) *data = std::vector<std::vector<std::vector<DomainElement>>>(_size);

			for (unsigned int i = 0; i < _size; i++) {
				if (sub[i]) {
					std::vector<std::vector<DomainElement>> t_res = findAllOccurs(cepisode, episode, nb_se, i, std::vector<int>(), 0, 0, p);
					for (unsigned int a = 0; a < t_res.size(); a++) {
                        for (unsigned int b = 0; b < t_res[a].size(); b++) {
                            DomainElement duration = t_res[a][b];
                            if (data != NULL) {
                                t.push_back(duration);
                                if (p) std::cerr << "[INFO] " << duration << " ";
                            }
                            occurrences[b].push_back(std::pair<DomainElement, unsigned int>(duration, i));
                        }
                        if (data != NULL) {
							if (p) std::cerr << "[INFO] ;" << std::endl;
							(*data)[i].push_back(t);
							t.clear();
						}
                    }
				}
			}
			
			for (unsigned int i = 0; i < occurrences.size(); i++) {
				std::vector<std::pair<DomainElement,unsigned int>>::iterator it = std::unique(
					occurrences[i].begin(),
					occurrences[i].end(),
					[](const std::pair<DomainElement,int>& p1, const std::pair<DomainElement,int>& p2){
						return p1.first == p2.first && p1.second == p2.second;
					}
				);

				occurrences[i].resize(std::distance(occurrences[i].begin(),it));
			}
			
			return MSetOcc(episode,occurrences);
		}
		
		void
		compile(
			std::vector<std::vector<int>>& res,
			std::vector<std::vector<int>>& bornes
		) const {
			for (std::vector<int> b : bornes) {
				if (res.size() == 0) for (int x : b) res.push_back({x,x});
				else {
					for (unsigned int i = 0; i < b.size(); i++) {
						if (b[i] < res[i][0]) res[i][0] = b[i];
						else if (b[i] > res[i][1]) res[i][1] = b[i];
					}
				}
			}
		}

		int
		getFreq (const Chronicle& c) const {
			int f = 0;
			for (unsigned int i = 0; i < _size; i++)
				if (occur(c,i,-1)) f++;
			return f;
		}

        int
        getFreq(const EMSet& e) const {
            std::vector<int> cepisode = e.getMSet();
            bool present;
            int res = 0;

            for (unsigned int i = 0; i < _size; i++) {
                present = true;
                for (unsigned int j = 0; j < cepisode.size(); j++) {
                    if (_base[j][i].size() < (unsigned) cepisode[j]) {
                        present = false;
                        break;
                    }
                }
                if (present) res++;
            }
            return res;
        }

        std::vector<std::vector<int>>
        findAllOccurs (
                const std::vector<int>& cepisode,
                const std::vector<int>& episode,
                const unsigned int& nb_se,
                const int& seq,
                const std::vector<int>& match,
                const int& e,
                const int& c
        ) const {
            return findAllOccurs(cepisode,episode,nb_se,seq,match,e,c,false);
        }

		std::vector<std::vector<DomainElement>>
		findAllOccurs (
			const std::vector<int>& cepisode,
			const std::vector<int>& episode,
			const unsigned int& nb_se,
			const int& seq,
			const std::vector<int>& match,
			const int& e,
			const int& c,
			const bool& p
		) const {
			std::vector<std::vector<DomainElement>> res;
			if ((unsigned) e < episode.size()){
				int elem = episode[e];
				
				if ((unsigned) elem < nb_se || (unsigned) elem%2 == nb_se%2) {
					int n_c = 1;
					if (c < cepisode[elem]) n_c += c;
					int pos = (e > 0 && episode[e-1] == elem)?match[e-1]+1:0;
					int total = _base[elem][seq].size()-pos-cepisode[elem]+n_c;
					if (n_c == cepisode[elem]) n_c = 0;

					for (unsigned int i = 0; i < (unsigned) total; i++) {
						std::vector<int> n_match(match);
						n_match.push_back(pos+i);
						std::vector<std::vector<DomainElement>> t_res = findAllOccurs(cepisode, episode, nb_se, seq, n_match, e + 1, n_c, p);
                        res.insert(res.end(),t_res.begin(),t_res.end());
					}
				}
				else {
					std::vector<int> n_match(match);
					int deb = elem-1;
                    DomainElement time = _base[deb][seq][match[e - (cepisode[elem - 1])]].getOtherTime();
					for (unsigned int i = 0; i < _base[elem][seq].size(); i++) {
						if (_base[elem][seq][i].getTime() == time) {
							n_match.push_back(i);
							break;
						}
					}
					
					std::vector<std::vector<DomainElement>> t_res = findAllOccurs(cepisode, episode, nb_se, seq, n_match, e + 1, 0, p);
					res.insert(res.end(),t_res.begin(),t_res.end());
				}
			}
			else {
				std::vector<DomainElement> t_res;

				if (match.size() == episode.size()) {
                    for (unsigned int i = 0; i < match.size(); i++) {
                        for (unsigned int j = i + 1; j < match.size(); j++) {
                            t_res.push_back(getTime(_base[episode[j]][seq][match[j]]) -
                                            getTime(_base[episode[i]][seq][match[i]]));
                        }
                    }
                    res.push_back(t_res);
                }
			}

			return res;
		}

		int
		getMaxOcc() const {
			unsigned int max = 0;
			for (unsigned int i=0; i < size(); i++) {
				for (unsigned int j = 0; j < _base.size(); j++) {
					if (_base[j][i].size() > max)
						max = _base[j][i].size();
				}
			}
			return max;
		}

		std::string
		getNZero(int n) const {
			std::stringstream tmp;
			for (int i = 0; i < n; i++)
				tmp << "0";
			return tmp.str();
		}

		std::vector<EMSet>
		extractFrequentMSet (
			unsigned int fmin,
			unsigned int mincs,
			unsigned int maxcs,
			bool closed
		){
			std::vector<int> mset(getNbEvent(),0);
			std::vector<bool> sub(size(),true);
			if (closed)
				mset = getClosedEpisode(sub);
			std::vector<EMSet> freqMSet;
			extractFrequentMSet(EMSet(mset, sub, size()), 0, 0, fmin, mincs, maxcs, closed, freqMSet);
			return freqMSet;
		}

		void
		extractFrequentMSet (
			const EMSet& pattern,
			unsigned int size,
			unsigned int last,
			unsigned int fmin,
			unsigned int mincs,
			unsigned int maxcs,
			bool closed,
			std::vector<EMSet>& freqMSet
		){
			if (pattern.getFreq() >= fmin && (maxcs == 0 || size <= maxcs)) {
				if (size >= mincs) freqMSet.push_back(pattern);
				if (maxcs == 0 || size < maxcs) {
					for (unsigned int i = last; i < _base.size(); i++) {
						if (_base[i].size() > 0) {
							unsigned int max_f = pattern.getFreq();
							std::vector<bool> nsub = pattern.getSub();
							for (unsigned int j = 0; j < nsub.size() && max_f >= fmin; j++) {
								if (nsub[j]) {
									if (_base[i][j].size() < (unsigned) pattern.getMSet()[i] + 1) {
										nsub[j] = false;
										max_f--;
									}
								}
							}

							if (max_f >= fmin) {
								std::vector<int> nmset = pattern.getMSet();
								nmset[i]++;
								bool redundant = false;

								if (closed) {
									std::vector<int> cnmset = getClosedEpisode(nsub);
									for (unsigned x = 0; x < i; x++) {
										if (cnmset[x] > nmset[x]) {
											redundant = true;
											break;
										}
									}
									nmset = cnmset;
								}

								if (!redundant)
									extractFrequentMSet(EMSet(nmset, nsub, max_f), size+1, i, fmin, mincs, maxcs, closed, freqMSet);
							}
						}
					}
				}
			}
		}
};

template <typename T>
VerticalEventBase<T>::VerticalEventBase(const std::vector<std::vector<std::vector<T>>>& base) : 
_base(base) {
	if (_base.size() > 0)
		_size = _base[0].size();
	else
		_size = 0;
}

template <typename T>
bool
VerticalEventBase<T>::occur (
	const Chronicle& c,
	const int& seq,
	const std::vector<T>& match,
	const int& e,
	const int& cwm
) const {
	int elem = c.getElement(e);
	int size = _base[elem][seq].size();
	
	if (size > 0) {
		DomainElement start = getTime(_base[elem][seq][0]);
        DomainElement end = getTime(_base[elem][seq][size - 1]);

		std::vector<DomainElement> stop;
	
		int pos = -1;

		std::vector<DomainElement>::iterator it_stop = stop.begin();
	
		if (pos == -1) {
			for (T i : _base[elem][seq]) {	
				if (getTime(i) >= start) {
					if (getTime(i) > end) break;

					while (it_stop != stop.end() && (*it_stop) < getTime(i))
						it_stop++;
					
					if (it_stop == stop.end() || (*it_stop) > getTime(i)) {	
						if ((unsigned) e+1 == match.size()) return true;
				
						std::vector<T> n_match = match;
						n_match[e] = i;

						if (occur (c, seq, n_match, e+1,cwm)) return true;
					}
				}
			}
		}
		else {
			DomainElement i = getOTime(match[pos]);
			if (i >= start) {
				if (i <= end) {		
				
					while (it_stop != stop.end() && (*it_stop) < i)
						it_stop++;
				
					if (it_stop == stop.end() || (*it_stop) > i) {	
						if ((unsigned) e+1 == match.size()) return true;
							
						std::vector<T> n_match = match;
						n_match[e] = Event(elem,i,i);

						if (occur (c, seq, n_match, e+1,cwm)) return true;
					}
				}
			}
		}
	}
	
	return false;
}

template <typename T>
std::vector<std::pair<DomainElement,int>>
VerticalEventBase<T>::getOccurences(
	const int& i,
	const int& j
) {
	bool itv = getItv(i,j);

    DomainElement a;
    DomainElement x;
    DomainElement xo;
    DomainElement y;
	
	std::vector<std::pair<DomainElement,int>> Ak;
	
	for (unsigned int t = 0; t < _base[i].size(); t++) {
	
		for (unsigned int k = 0; k < _base[i][t].size(); k++) {
			x = getTime(_base[i][t][k]);
			xo = getOTime(_base[i][t][k]);
			
			for (unsigned int l = 0; l < _base[j][t].size(); l++) {
				y = getTime(_base[j][t][l]);
				
				if (i != j || x != y) {
					if (i != j-1 || (!itv || xo != y)) {
						a = y - x;

						if (!(_conclude[i] && !_conclude[j]) || a <= DomainElementFactory::zero()) {
							if (!(!_conclude[i] && _conclude[j]) || a >= DomainElementFactory::zero()) {
								if (_cwm == DomainElementFactory::zero() || (a <= _cwm)) {
									if (i == j) Ak.push_back(std::pair<DomainElement,int>(a, t));
									else Ak.push_back(std::pair<DomainElement,int>(a, t));
								}
								else if (a > _cwm) break; 
							}
						}
					}
				}
			}
		}
	}
	
	return Ak;
}

template <>
bool
VerticalEventBase<int>::getItv(
	const int&,
	const int&
) const {
	return false;
}

template <>
bool
VerticalEventBase<Event>::getItv(
	const int& i,
	const int& j
) const {
	return j == i+1 && i >= _nb_se && (i - _nb_se)%2 == 0;
}

template <>
std::vector<std::pair<DomainElement,int>>
VerticalEventBase<int>::getIntOccurences(
	const int&
) {
	return std::vector<std::pair<DomainElement,int>>();
}

template <>
std::vector<std::pair<DomainElement,int>>
VerticalEventBase<Event>::getIntOccurences(
	const int& i
) {
    DomainElement a;
	std::vector<std::pair<DomainElement,int>> Ak;
	
	for (unsigned int t = 0; t < _base[i].size(); t++) {
		for (unsigned int k = 0; k < _base[i][t].size(); k++) {
			a = getOTime(_base[i][t][k]) - getTime(_base[i][t][k]);
		
			if (_cwm == DomainElementFactory::zero() || (a <= _cwm))
				Ak.push_back(std::pair<DomainElement,int>(a, t));
		}
	}
	
	return Ak;
}

#endif
