#ifndef __Chronicle_HH_
#define __Chronicle_HH_

#include "TCIterator.hh"
#include <vector>
#include <string>
#include <map>
#include <iostream>

class Chronicle {
	private:
	static int nextId;

	protected :
	
		std::vector<int> _elements;
		std::vector<TCIterator> _constraints;
		
		unsigned int _flag;
			
	public :

		friend std::ostream& operator<< (std::ostream& out, Chronicle& c);

		int id;
		int frequency;
		int ofrequency;
		std::vector<unsigned int> tid_list_pos;
		std::vector<unsigned int> tid_list_neg;
  

		Chronicle(){}

		Chronicle(
				const std::vector<int>& e,
				const std::vector <TCIterator>& c,
				const int f,
				const int of,
				const std::vector<unsigned int>& tid_list_pos,
				const std::vector<unsigned int>& tid_list_neg
		):
				_elements(e),
				_constraints(c),
				_flag(0),
				id(nextId++),
				frequency(f),
				ofrequency(of),
				tid_list_pos(tid_list_pos),
				tid_list_neg(tid_list_neg)
		{}

		Chronicle(
				const std::vector<int>& e,
				const std::vector<TCIterator>& c,
				const int f,
				const int of
		) :
				_elements(e),
				_constraints(c),
				_flag(0),
				id(nextId++),
				frequency(f),
				ofrequency(of)
		{}

		Chronicle(
				const std::vector<int>& e,
				const std::vector<TCIterator>& c,
				const int& f
		) :
				_elements(e),
				_constraints(c),
				_flag(0),
				id(nextId++),
				frequency(f)
		{}

		Chronicle(
			const std::vector<int>& e,
			const std::vector<TCIterator>& c
		) :
			_elements(e),
			_constraints(c),
			_flag(0),
			id(nextId++),
			frequency(0)
		{}
		
		bool
		extend(
			int e,
			const std::vector<TCIterator>& c
		) {
			_elements.push_back(e);
			_constraints.insert(_constraints.end(), c.begin(), c.end());
			return true;
		}
		
		void
		specConstraint(
			const int& i,
			const int& dir
		) {
			(dir == RIGHT) ? _constraints[i] = _constraints[i]->_right : _constraints[i] = _constraints[i]->_left;
			_flag = (unsigned int) i;
		}
		
		const int&
		getElement(const int& i) const {
			return _elements[i];
		}
		
		const TCIterator&
		getConstraint(const int& i) const {
			return _constraints[i];
		}
		
		const unsigned int&
		getFlag() const {
			return _flag;
		}
		
		bool
		canSpecConstraint(
			const int& i,
			const int& dir
		) const {
			return (dir == RIGHT) ? _constraints[i]->_right != nullptr : _constraints[i]->_left != nullptr && _constraints[i]->_left->_pleft == nullptr;
		}

		const std::vector<TCIterator>& getConstraints() const {return _constraints;};
		
		unsigned int
		size() const {
			return (unsigned int) _elements.size();
		}

		static int
		indice (
			const int& i,
			const int& j
		);
		
		int
		str_indice(
			const int& i,
			const int& j
		) const;
		
		std::string
		str (std::vector<std::string> events) const;

		std::string
		json (
			const std::vector<std::string>& events,
			std::vector<std::pair<bool,bool>>* disc
		) const;

		std::string
		txt (
			const std::vector<std::string>& events,
			std::vector<std::pair<bool,bool>>* disc
		) const;

		std::string
		json (const std::vector<std::string>& events) const;
		

		operator std::string() const;
};

#endif
