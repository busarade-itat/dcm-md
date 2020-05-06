#include "CDA.hh"
#include "../../Domain/src/interval_pruning.hh"

using namespace ripper;

void
CDA::testSub(
		MSetOcc& s,
		const std::vector<std::vector<std::vector<DomainElement>>>& positiveData,
		const std::vector<std::vector<std::vector<DomainElement>>>& negativeData,
		const MultiRule& r,
		int f,
        double g,
        double ratio
) const {
	std::vector<std::vector<DomainElement>> bornes;
	std::vector<int> tcs = r.getConstraints();
	std::vector<std::pair<DomainElement*, DomainElement*>> values = r.getValues();

	unsigned int vf = 0;
	std::vector<unsigned int> tid_list_pos;

	for (unsigned int i = 0; i < positiveData.size(); i++) {
        bool match = false;

        for (const auto &instance : positiveData[i]) {
            bool val = true;

            for (unsigned int x = 0; x < tcs.size() && val; x++) {
                if (!instance[tcs[x]].isInsideBounds(*(values[x].first), *(values[x].second))) {
                    val = false;
                }
            }
            if (val) {
                match = true;
                for (unsigned int k = 0; k < instance.size(); k++) {
                    if (bornes.size() <= k) {
                        bornes.push_back({instance[k], instance[k]});
                    }
                    else {
                        pruneRedundantIntervals(k, bornes, instance);
                    }
                }
            }
        }

        if (match) {
			vf++;
			if (this->extract_with_tid) tid_list_pos.push_back(i);
		}
    }

	if (vf >= (unsigned) f) {
        unsigned int vfn = 0;
		std::vector<unsigned int> tid_list_neg;
		for (unsigned int i = 0; i < negativeData.size(); i++) {
            bool match = false;

            for (unsigned int j = 0; j < negativeData[i].size() && !match; j++) {
                bool val = true;

                for (unsigned int x = 0; x < tcs.size() && val; x++) {
                    DomainElement* lowerBound = values[x].first;
                    DomainElement* upperBound = values[x].second;
                    DomainElement occurence = negativeData[i][j][tcs[x]];

                    if (!occurence.isInsideBounds(*lowerBound, *upperBound)) {
                        val = false;
                    }
                }

                if (val) {
                    match = true;
                }
            }

            if (match) {
				vfn++;
				if (this->extract_with_tid) tid_list_neg.push_back(i);
			}
        }

        if (vfn == 0 || double(vf)/(double(vfn)*ratio) >= g) {
            std::vector<std::pair<bool, bool>> disc(bornes.size(), std::pair<bool, bool>(false, false));
            for (unsigned int x = 0; x < tcs.size(); x++) {
                if (*(values[x].first) != DomainElementFactory::negativeInfinity()) {
                    disc[tcs[x]].first = true;
                }
                if (*(values[x].second) != DomainElementFactory::positiveInfinity()) {
                    disc[tcs[x]].second = true;
                }
            }

            std::vector<int> elements = s.getElements();
            std::vector<TCIterator> tcis;
            for (auto &borne : bornes) {
                TC *tc = new TC(borne[0], borne[1]);
                tcis.emplace_back(std::shared_ptr<TC>(tc), true);
            }

            if (txt) {
				if (!this->extract_with_tid)
					std::cout << Chronicle(elements, tcis, vf, vfn).txt(inv_events, &disc) << std::endl;
				else
					std::cout << Chronicle(elements, tcis, vf, vfn, tid_list_pos, tid_list_neg).txt(inv_events, &disc) << std::endl;
			}
            else
			{
				if (!this->extract_with_tid)
					std::cout << Chronicle(elements, tcis, vf, vfn).json(inv_events, &disc) << std::endl;
				else
					std::cout << Chronicle(elements, tcis, vf, vfn, tid_list_pos, tid_list_neg).json(inv_events, &disc) << std::endl;
			}
        }
	}
}

bool
CDA::run (
	Base& trace1,
	Base& trace2,
	unsigned int fmin,
	double gmin,
	unsigned int nb_e,
    unsigned int mincs,
    unsigned int maxcs,
	bool print,
	bool closed,
	bool use_episode,
	bool irep
) {
	std::vector<EMSet> bset;

	if (print) {
		std::cerr << "[INFO] Example number : " << trace1.size() + trace2.size() << std::endl;
		std::cerr << "[INFO] Positive examples : " << trace1.size() << std::endl;
		std::cerr << "[INFO] Negative examples : " << trace2.size() << std::endl;
		std::cerr << "[INFO] Ratio : " << ((double) trace1.size() / (double) trace2.size()) << std::endl;
	}

	{
        if (print) std::cerr << "[INFO] Multisets mining" << std::endl;
		std::vector<EMSet> set1 = trace1.extractFrequentMSet(fmin, mincs, maxcs, closed);
        if (print) std::cerr << "[INFO] Multisets number: " << set1.size() << std::endl;

		std::vector<MSetOcc> disc_msets;
		emergeMSetOcc(emergeMSet(set1,trace2,&bset,gmin,mincs,maxcs), trace1, nb_e, disc_msets);
	}

    if (print) std::cerr << "[INFO] Non emergeant multisets number: " << bset.size() << std::endl << std::endl;

	std::vector<EMSet>::iterator it;

	for (it = bset.begin(); it != bset.end(); it++){
		pid_t pid = fork();

		if (pid == 0) {
			EMSet t = *(it);
			std::vector<int> episode = t.getShortMSet();

			if (episode.size() >= 2) {
				int n = (int) (episode.size() * (episode.size() - 1)) / 2;

				std::vector<bool> ssub1((unsigned long) trace1.size(), true);
				std::vector<bool> ssub2((unsigned long) trace2.size(), true);

				for (unsigned int i = 0; i < t.getMSet().size(); i++) {
					if (t.getMSet()[i] > 0) {
						trace1.getSubBase(ssub1, i, t.getMSet()[i]);
						trace2.getSubBase(ssub2, i, t.getMSet()[i]);
					}
				}

				std::vector<std::vector<std::vector<DomainElement>>> data_a;
				std::vector<std::vector<std::vector<DomainElement>>> data_b;

				MSetOcc s1((std::vector<int>()), std::vector<std::vector<std::pair<DomainElement, unsigned int>>>()); // = trace1.compileOccurrences(ssub1, t.getMSet(), episode, nb_e, &data_a, use_episode, false);
				trace2.compileOccurrences(ssub2, t.getMSet(), episode, nb_e, &data_b, use_episode, false);

				std::vector<MultiRule> rules = ripper::run(
				    n,
				    data_a,
				    data_b,
				    fmin,
				    gmin,
				    irep,
				    print,
				    false
                );

				for (auto &rule : rules) {
                    if (rule.getClass() == 0) {
                        testSub(s1, data_a, data_b, rule, fmin, gmin, 1);
                    }
                }
			}
            return false;
		}
		else if (pid > 0) {
            waitpid(pid, nullptr,0);
        }
		else {
			std::cerr << "Fork error" << std::endl;
			return true;
		}
	}
	return true;
}

void
CDA::emergeMSetOcc (
	const std::vector<EMSet>& set,
	const Base& trace,
	unsigned int,
	std::vector<MSetOcc>& res
) {
	for (const auto &i : set) {
		// We do not compute emerging multiset occurrences for memory space and computational time
		// MSetOcc pat = trace.compileOccurrences(set[i].getSub(), set[i].getMSet(), set[i].getShortMSet(), nb_se);
		MSetOcc pat = MSetOcc(i.getShortMSet(), std::vector<std::vector<std::pair<DomainElement,unsigned int>>>());
		pat.classe = (unsigned int) trace.classe;
        pat.frequency = i.getFreq();
		pat.ofrequency = i.getOFreq();

		if (!inv_events.empty()) {
			if (txt)
				std::cout << pat.txt(inv_events,true) << std::endl;
			else
				std::cout << pat.json(inv_events,true) << std::endl;
		}
        else res.push_back(pat);
	}
}

std::vector<EMSet>
CDA::emergeMSet (
	std::vector<EMSet>& set1,
    const Base& trace,
	std::vector<EMSet>* bset,
	const double& gmin,
    unsigned int mincs,
    unsigned int maxcs
) {
	std::vector<EMSet> res;
    double min = gmin;
	for (auto &i : set1) {
        std::vector<int> episode = i.getShortMSet();
        if (episode.size() >= mincs && (episode.size() <= maxcs || maxcs == 0)) {
			if (trace.getFreq(i) * min <= i.getFreq()) {
				i.setOFreq((unsigned int) trace.getFreq(i));
                res.push_back(i);
            }
            else if (bset != nullptr)
				bset->push_back(i);
        }
	}
	
	return res;
}

std::vector<Chronicle> 
CDA::run (
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
) {

	std::list<Chronicle> extended;
	std::list<Chronicle> frequents;
		
	{
		std::vector<Chronicle> enfants;
		std::list<Chronicle> extends;
		bool add;

		if (p) {
			std::cerr << "[INFO] Constraint graphs number : " << base_pct.size() << std::endl;
			std::cerr << "[INFO] Elements number : " << nb_e << std::endl;
			std::cerr << "[INFO] Simple elements number : " << nb_se << std::endl;
			std::cerr << "[INFO] Intervals graphs number : " << base_itv.size() << std::endl << std::endl;
		}

		if (init != nullptr) extends.push_back(*init);
		else {
			unsigned int nb_itv = (nb_e - nb_se) / 2;
			for (unsigned int i = 0; i < nb_e; i++) {
				unsigned int j = i;
				if (dif) j++;
			
				for (; j < nb_e; j++) {
					if (base_pct[indice(i,j,nb_e)] != nullptr) {
						if (!itv_only || i < nb_se || j != i+1 || (i-nb_se)%2 == 1) 
							extends.push_back(Chronicle({(int) i, (int) j},{TCIterator (base_pct[indice(i,j,nb_e)],true)}));
						
					}
				}
			}
			
			if (base_itv.size() == nb_itv) {
				for (unsigned int i = 0; i < nb_itv; i++) {
					if (base_itv[i] != nullptr)
						extends.push_back(Chronicle({(int) (nb_se+i*2), (int) (nb_se+i*2+1)},{TCIterator(base_itv[i],false)}));
				}
			}
		}
		
		while (!stop && !extends.empty()){
			Chronicle c (extends.back());
			extends.pop_back();
			add = true;	
            if (trace.frequent(c, fmin, frequents, add, (close || freq), cwm)) {
				if (maxcs < 2 || c.size() <= maxcs) {
					if (c.size() >= mincs) {
						if (add)
							minAdd(frequents, c, close);

						extended.push_back(c);
					}
					enfants = generateExtendChild(c,base_pct,base_itv,nb_e,nb_se,dif,itv_only);
					extends.insert(extends.end(), enfants.begin(), enfants.end());
				}
			}
		}
	}
	
	for (const Chronicle &c : extended) {
		if (stop) break;
		run(c,trace,fmin,frequents,p,cwm,close,(close || freq));
	}
	
	return std::vector<Chronicle>(frequents.begin(), frequents.end());
}

void
CDA::run (
	const Chronicle& chron,
	const Base& trace,
	unsigned int fmin,
	std::list<Chronicle>& frequents,
	bool p,
	int cwm,
	bool close,
	bool freq
) {
	Chronicle c (chron);
	bool add = true;
	
	if (trace.frequent(c, fmin, frequents, add, freq, cwm)) {

		for (unsigned long i = stats.size(); i < c.size(); i++)
			stats.push_back(0);		
		stats[c.size()-1]++;

		if (add) {
			minAdd(frequents, c, close);
		}

		runSpecChild(c, trace, fmin, frequents, p, cwm, close, freq);
	}
}

void
CDA::runSpecChild(
	const Chronicle& c,
	const Base& trace,
	unsigned int fmin,
	std::list<Chronicle>& frequents,
	bool p,
	int cwm,
	bool close,
	bool freq
) {

	int size = c.size()*(c.size()-1)/2;

	for (int i = c.getFlag(); i < size; i++) {
		
		if (stop || next) break;
		
		if (c.canSpecConstraint(i, RIGHT)) {
		
			Chronicle nc = c;
			nc.specConstraint(i, RIGHT);
			run(nc, trace, fmin, frequents, p, cwm, close, freq);
		}
		
		if (c.canSpecConstraint(i, LEFT)) {
		
			Chronicle nc = c;
			nc.specConstraint(i, LEFT);
			run(nc, trace, fmin, frequents, p, cwm, close, freq);
		}
	}
}

void
CDA::CCDC(
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
) {
	
	std::vector<std::pair<DomainElement,int>> Ak;
	std::vector<std::pair<DomainElement,int>>::iterator it;
	
	trace->initOccurences(conclusion,nb_se,cwm,dif);
	
	for (unsigned int i = 0; i < nb_e; i++) {
	
		unsigned int j = i;
		if (dif) {
			j++;
			base_pct.push_back(nullptr);
		}
		
		if (i >= nb_se && (i - nb_se)%2 == 0) {
			Ak = trace->getIntOccurences(i);
			
			std::sort(
				Ak.begin(),
				Ak.end(),
				[](const std::pair<DomainElement,int>& p1, const std::pair<DomainElement,int>& p2){
					if (p1.first < p2.first) return true;
					if (p1.first == p2.first) return p1.second < p2.second;
					return false;
				}
			);
			
			it = std::unique(
				Ak.begin(),
				Ak.end(),
				[](const std::pair<DomainElement,int>& p1, const std::pair<DomainElement,int>& p2){
					return p1.first == p2.first && p1.second == p2.second;
				}
			);
			
			Ak.resize((unsigned long) std::distance(Ak.begin(),it));
			
			if (Ak.size() >= fmin)
				base_itv.push_back(G(Ak, fmin, trace->size()));
			else
				base_itv.push_back(nullptr);
				
			if (itv_only && dif) {
				base_pct.push_back(nullptr);
				j++;
			}	
		}
		
		for (; j < nb_e; j++) {
			Ak = trace->getOccurences(i,j);
			
			std::sort(
				Ak.begin(),
				Ak.end(),
				[](const std::pair<DomainElement,int>& p1, const std::pair<DomainElement,int>& p2){
					if (p1.first < p2.first) return true;
					if (p1.first == p2.first) return p1.second < p2.second;
					return false;
				}
			);
			
			it = std::unique(
				Ak.begin(),
				Ak.end(),
				[](const std::pair<DomainElement,int>& p1, const std::pair<DomainElement,int>& p2){
					return p1.first == p2.first && p1.second == p2.second;
				}
			);
			
			Ak.resize((unsigned long) std::distance(Ak.begin(),it));
			
			if (Ak.size() >= fmin)
				base_pct.push_back(G(Ak, fmin, trace->size()));
			else
				base_pct.push_back(nullptr);
		}
	}
	
	trace->clearOccurences();
}

bool
CDA::OccFreq (
	const std::vector<std::pair<DomainElement,int>>& Ak,
	unsigned int fmin,
	unsigned int nb_seq,
	int l,
	int k
) {
	std::vector<bool> sett((unsigned long) nb_seq,false);
	unsigned int sett_size = 0;

	for (int m = 0; m <= k; m++) {
		if (!sett[Ak[l+m].second]) {
			sett[Ak[l+m].second] = true;
			sett_size++;
		
			if (sett_size >= fmin) return true;
		}
	}
	
	return false;
}

std::shared_ptr<TC>
CDA::G(
	const std::vector<std::pair<DomainElement,int>>& Ak,
	unsigned int fmin,
	unsigned int nb_seq
) {
	if (OccFreq(Ak,fmin,nb_seq,0,(int) Ak.size()-1)) {
	
		std::shared_ptr<TC> res (new TC(Ak[0].first,Ak[Ak.size()-1].first));
		nb_tc++;
		
		if (Ak.size()-1 > fmin) {
			res->_left = G(res,false,Ak,fmin,nb_seq,0,(int) Ak.size()-2);
			res->_right = G(res,true,Ak,fmin,nb_seq,1,(int) Ak.size()-2);
		}
		
		return res;
	}
	
	return nullptr;
}

std::shared_ptr<TC>
CDA::G(
	const std::shared_ptr<TC>& father,
	bool left,
	const std::vector<std::pair<DomainElement,int>>& Ak,
	unsigned int fmin,
	unsigned int nb_seq,
	int l,
	int k
) {
	int nl = l;
	int nk = k;

	if (left) {
		while (Ak[nl].first == father->getStart()) {
			nl++;
			nk--;
			if ((unsigned) nk < fmin) return nullptr;
		}
	} 
	else while (Ak[nl+nk].first == father->getEnd()) {
		nk--;
		if ((unsigned) nk < fmin) return nullptr;
	}

	if (OccFreq(Ak,fmin,nb_seq,nl,nk)) {
	
		std::shared_ptr<TC> res (new TC(Ak[nl].first,Ak[nk].first));
		nb_tc++;
		
		if (left) {
			res->_pleft = father;
			if (father->_left != nullptr && father->_left->_right != nullptr && father->_left->_right->getStart() == res->getStart()) {
				res->_left = father->_left->_right;
				father->_left->_right->_pright = res;
			}
		}
		else res->_pright = father;
		
		if ((unsigned) nk > fmin) {
			if (!left && res->_left == nullptr) {
				res->_left = G(res,false,Ak,fmin,nb_seq,nl,nk-1);
			}
			if (res->_right == nullptr)	
				res->_right = G(res,true,Ak,fmin,nb_seq,nl+1,nk-1);
		}
		
		return res;
	}
	return nullptr;
}

void
CDA::minAdd (
	std::list<Chronicle>& s,
	const Chronicle& c,
	bool
) {
		s.push_back(c);
}

std::vector<Chronicle>
CDA::generateExtendChild(
	const Chronicle& c,
	const std::vector<std::shared_ptr<TC>>& base_pct,
	const std::vector<std::shared_ptr<TC>>& base_itv,
	unsigned int nb_e,
	unsigned int nb_se,
	bool dif,
	bool itv_only
) {	
	std::vector<Chronicle> res;
	int csize = c.size();
	int first = c.getElement(csize -1);
	int citv = -1;
	
	if (base_itv.size() == (nb_e - nb_se)/2 && first >= (int) nb_se && (first-nb_se)%2 == 0 && base_itv[(first-nb_se)/2] != nullptr)
		citv = first+1;
	
	if (dif) first++;
	else if (c.size() >= 2 && first == c.getElement(csize -2)+1 && !(c.getConstraints().back().isPunct())) {
		first = c.getElement(csize -2);
 	}
 	
 	if (citv >= 0) {
 		std::vector<TCIterator> tcs;
		int j;
		
		for (j = 0; j < csize-1; j++) {
		
			int e = c.getElement(j);
			int ind = indice(e,citv,nb_e);
			
			if (base_pct[ind] != nullptr)
				tcs.emplace_back(base_pct[ind],true);
			else {
				j = 0;
				break;
			}
		}
		
		if (j == csize -1) {
			tcs.emplace_back(base_itv[(citv-1-nb_se)/2], false);
			Chronicle nc = c;
			
			nc.extend(citv,tcs);
			res.push_back(nc);
			
			if (itv_only)
				first = citv+1;
		}
 	}
 	
	for (int i = first; (unsigned) i < nb_e; i++) {
		std::vector<TCIterator> tcs;
		int j;
		
		for (j = 0; j < csize; j++) {
			int e = c.getElement(j);
			int ind;
			if (e < i)
				ind = indice(e,i,nb_e);
			else
				ind = indice(i,e,nb_e);
			if (base_pct[ind] != nullptr)
				tcs.emplace_back(base_pct[ind],true);
			else {
				j = 0;
				break;
			}
		}
		
		if (j == csize) {
			Chronicle nc = c;
			nc.extend(i,tcs);
			res.push_back(nc);
		}
	}
	
	return res;
}
