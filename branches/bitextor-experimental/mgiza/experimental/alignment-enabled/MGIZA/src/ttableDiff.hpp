/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * newgiza
 * Copyright (C) Qin Gao 2007 <qing@cs.cmu.edu>
 * 
 * newgiza is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * newgiza is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with newgiza.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */


#ifndef _TTABLEDIFF_HPP_
#define _TTABLEDIFF_HPP_
#include "TTables.h"
#include <sstream>
#include <string>
#include "types.h"

using namespace std;
/*!
This class is meant to create a difference file in order to make
GIZA paralell.
*/
template <class COUNT,class PROB>
class CTTableDiff{
private:
		INT32 noEnglishWords;  // total number of unique source words
		INT32 noFrenchWords;   // total number of unique target words
		/*!
		Store only the counting*/	
		hash_map<wordPairIds, COUNT, hashpair, equal_to<wordPairIds> > ef;

public:
		INT32 SaveToFile(const char* filename){
			ofstream ofs(filename);
			if(!ofs.is_open()){
				return -1;
			}else{
				typename hash_map<wordPairIds, COUNT, hashpair, equal_to<wordPairIds> >::iterator it;
				for( it = ef.begin() ; it != ef.end(); it++){
					ofs << it->first.first << " " << it->first.second << " "
						<< it->second << std::endl;
				}
			}
			return SUCCESS;
		}
	
		INT32 LoadFromFile(const char* filename){
			ef.clear();
			ifstream ifs(filename);
			if(!ifs.is_open()){
				return -1;
			}
			string sline;
			while(!ifs.eof()){
				sline = "";
				std::getline(ifs,sline);
				if(sline.length()){
					//cout << sline << endl;
					stringstream ss(sline.c_str());
					WordIndex we=-1,wf=-1;
					COUNT ct=-1 ;
					ss >> we >> wf >> ct;
					if(we==-1||wf==-1||ct==-1)
						continue;
					ef[wordPairIds(we,wf)] = ct;
				}
			}
			return SUCCESS;
		}
	
		COUNT * GetPtr(WordIndex e, WordIndex f){
			// look up this pair and return its position
			typename hash_map<wordPairIds, COUNT, hashpair, equal_to<wordPairIds> >::iterator i = ef.find(wordPairIds(e, f)); 
			if(i != ef.end())  // if it exists, return a pointer to it.
				return(&((*i).second));
			else return(0) ; // else return NULL pointer
		}
	
		void incCount(WordIndex e, WordIndex f, COUNT inc) 
		// increments the count of the given word pair. if the pair does not exist, 
		// it creates it with the given value.
		{
			if( inc )
				ef[wordPairIds(e, f)] += inc ;
		}
	
		INT32 AugmentTTable(tmodel<COUNT,PROB>& ttable){
			typename hash_map<wordPairIds, COUNT, hashpair, 
				equal_to<wordPairIds> >::iterator it;
			for( it = ef.begin() ; it != ef.end(); it++){
				ttable.incCount(it->first.first,it->first.second,it->second);
			}
			return SUCCESS;
		}
	
protected:

};

#endif // _TTABLEDIFF_HPP_
