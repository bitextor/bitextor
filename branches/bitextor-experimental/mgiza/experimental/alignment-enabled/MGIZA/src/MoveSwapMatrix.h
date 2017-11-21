/*

 EGYPT Toolkit for Statistical Machine Translation
 Written by Yaser Al-Onaizan, Jan Curin, Michael Jahr, Kevin Knight, John Lafferty, Dan Melamed, David Purdy, Franz Och, Noah Smith, and David Yarowsky.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 USA.

 */
/*--
 MoveSwapMatrix: Efficient representation for moving and swapping
 around in IBM3 training.
 Franz Josef Och (30/07/99)
 --*/
#ifndef moveswap2_costs_h_defined
#define moveswap2_costs_h_defined
#include "alignment.h"
#include "transpair_model3.h"
#include "myassert.h"
#include <set>
#include <map>
#include <vector>

extern short DoViterbiTraining;

template<class TRANSPAIR>
class MoveSwapMatrix: public alignment {
private:
	const TRANSPAIR&ef;
	const WordIndex l, m;
	Array2<LogProb, Vector<LogProb> > _cmove, _cswap;
	Array2<char, Vector<char> > delmove, delswap;
	Vector<int> changed;
	int changedCounter;
	const int modelnr;
	bool lazyEvaluation;
	bool centerDeleted;
	std::map<int,std::set<int> >untouch_i; // target words that should not be aligned anywhere
	std::map<int,std::set<int> > untouch_j;
public:
	void addUnTouchI(int i, int j){
		if(i>0){
			if(untouch_i.find(i)==untouch_i.end()){
				untouch_i[i] = std::set<int>();
			}
			untouch_i[i].insert(j);

		}
	}

	void addUnTouchJ(int j,int i){
		if(j>0){
			if(untouch_j.find(j)==untouch_j.end()){
				untouch_j[j] = std::set<int>();
			}
			untouch_j[j].insert(i);
		}
	}

	bool check() const {
		return 1;
	}
	const TRANSPAIR&get_ef() const {
		return ef;
	}
	bool isCenterDeleted() const {
		return centerDeleted;
	}
	bool isLazy() const {
		return lazyEvaluation;
	}
	MoveSwapMatrix(const TRANSPAIR&_ef, const alignment&_a);
	void updateJ(WordIndex j, bool, double thisValue);
	void updateI(WordIndex i, double thisValue);
	void doMove(WordIndex _i, WordIndex _j);
	void doSwap(WordIndex _j1, WordIndex _j2);
	void delCenter() {
		centerDeleted = 1;
	}
	void delMove(WordIndex x, WordIndex y) {
		delmove(x, y) = 1;
	}
	void delSwap(WordIndex x, WordIndex y) {
		massert(y>x);
		delswap(x, y) = 1;
		delswap(y, x) = 1;
	}
	bool isDelMove(WordIndex x, WordIndex y) const {
		return DoViterbiTraining || delmove(x, y);
	}
	bool isDelSwap(WordIndex x, WordIndex y) const {
		massert(y>x);
		return DoViterbiTraining || delswap(x, y);
	}
	LogProb cmove(WordIndex x, WordIndex y) const {
		massert( get_al(y)!=x );
		massert( delmove(x,y)==0 );
		if (lazyEvaluation)
			return ef.scoreOfMove(*this, x, y);
		else {
			std::map<int, std::set<int> >::const_iterator it;

			it = untouch_i.find(x);
			if(it!=untouch_i.end()){
				// Return -1 if the j jump set is not within the limit
				if(it->second.find(y) == it->second.end()) //Not in the feasible set
					return -1;
			}
			it = untouch_j.find(y);
			if(it!=untouch_j.end()){
				if(it->second.find(x) == it->second.end()) //Not in the feasible set
			return -1;
			}
			return _cmove(x, y);
		}
	}
	LogProb cswap(WordIndex x, WordIndex y) const {
		massert(x<y);
		massert(delswap(x,y)==0);
		massert(get_al(x)!=get_al(y));
		if (lazyEvaluation)
			return ef.scoreOfSwap(*this, x, y);
		else {
			massert(y>x);
			std::map<int, std::set<int> >::const_iterator it1,it2;
			it1 =untouch_j.find(y);
			it2 = untouch_j.find(x);
			int nal1 = get_al(y);
			int nal2 = get_al(x); // Need to test if nal1 is in it2's feasible set
			                      // and vice versa

			if(it1!=untouch_j.end()&&it1->second.find(nal2)==it1->second.end()){
				return -1;
			}
			if(it2!=untouch_j.end()&&it2->second.find(nal1)==it2->second.end()){
				return -1;
			}
			// Make sure we never swap these
			return _cswap(x, y);
		}
	}
	void printWrongs() const;
	bool isRight() const;
	friend ostream&operator<<(ostream&out, const MoveSwapMatrix<TRANSPAIR>&m) {
		return out << (alignment) m << "\nEF:\n" << m.ef << "\nCMOVE\n"
				<< m._cmove << "\nCSWAP\n" << m._cswap << endl;
	}
	;
};
#endif
