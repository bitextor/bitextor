/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_TEIREADER_DICTIONARIES_H
#define __HUNGLISH_TEIREADER_DICTIONARIES_H

#include <map>
#include <vector>
#include <set>
#include <iostream>

namespace Hunglish
{

// A simple tree class.
// 
template <class Atom, class Identifier>
class DicTree
{
public:
  // Gets value a bit below. Ugly C++.
  static const bool WarnOnConflict;

  DicTree() : id(0) {}
  DicTree( const Identifier& id_ ) : id(id_) {}

  ~DicTree();

  const Identifier& getIdentifier() const { return id; }
  void setIdentifier( const Identifier& id_) { id=id_; }
  DicTree<Atom, Identifier>* lookup( const Atom& word ) const;
  DicTree& add( const Atom& word, const Identifier& id );
  bool empty() const { return children.empty(); }

  void dump( std::ostream& os ) const;

private:
  typedef std::map<Atom,DicTree*> DicTreeMap;
  DicTreeMap children;
  Identifier id;
};

template <class Atom, class Identifier>
const bool DicTree<Atom,Identifier>::WarnOnConflict = false;

// This structure stores a very sparse set-system of words.
// (A dictionary of complex expressions.)
// 
// It supports the following query:
// It receives a set of words S. It gives back the sets 
// of the set system that are contained in this set S.
// 
// For it to be effective, we must be careful during the building phase:
// words in vector 'words' must be ordered by INCREASING frequency. Rare words first.

template <class Atom, class Identifier>
class SubsetLookup
{
public:

  typedef std::vector<Atom> Atoms;

  void add( const Atoms& words, const Identifier& id );

  void lookup( const Atoms& words, std::set<Identifier>& results ) const;

  void dump( std::ostream& os ) const;

private:
  DicTree<Atom,Identifier> tree;
};

// Implementation. F.ck C++ for having to put this in a header.

template <class Atom, class Identifier>
DicTree<Atom, Identifier>::~DicTree()
{
  for ( typename DicTreeMap::iterator it=children.begin(); it!=children.end(); ++it )
  {
    delete it->second;
  }
}

// Az id-t soha nem irja at nullarol nemnullara.
// Ha nemnullarol nemnullara irja at, akkor kiabal elotte.
template <class Atom, class Identifier>
DicTree<Atom, Identifier>& DicTree<Atom, Identifier>::add( const Atom& word, const Identifier& id )
{
  DicTree* v = lookup(word);
  if (!v)
  {
    v = new DicTree<Atom, Identifier>();
    v->id = id;
    children[word] = v;
  }
  else
  {
    if ( ( v->id != 0 ) && ( id != 0 ) )
    {
      if (WarnOnConflict)
        std::cerr << "warning: conflict in tree" << std::endl;
    }
    if ( id != 0 )
    {
      v->id = id;
    }
  }

  return (*v);
}

template <class Atom, class Identifier>
DicTree<Atom, Identifier>* DicTree<Atom, Identifier>::lookup( const Atom& word ) const
{
  typename DicTreeMap::const_iterator ft = children.find(word);

  if (ft==children.end())
  {
    return 0;
  }
  else
  {
    return ft->second;
  }
}

template <class Atom, class Identifier>
void DicTree<Atom, Identifier>::dump( std::ostream& os ) const
{
  if (id!=0)
  {
    os << id << " ";
  }
  os << "{" << std::endl;
  for ( typename DicTreeMap::const_iterator it=children.begin(); it!=children.end(); ++it )
  {
    os << it->first << " ";
    it->second->dump(os);
  }
  os << "}" << std::endl;
}

template <class Atom, class Identifier>
void SubsetLookup<Atom, Identifier>::add( const Atoms& words, const Identifier& id )
{
  DicTree<Atom, Identifier>* v = &tree;

  for ( typename Atoms::const_iterator it=words.begin(); it!=words.end(); ++it )
  {
    DicTree<Atom, Identifier>& newv = v->add(*it,0);
    v = &newv;
  }
  if ( v->getIdentifier() == 0 )
  {
    v->setIdentifier(id);
  }
  else
  {
    if (DicTree<Atom, Identifier>::WarnOnConflict)
      std::cerr << "warning: conflict in tree" << std::endl;
  }
}

template <class Atom, class Identifier>
void SubsetLookup<Atom, Identifier>::lookup( const Atoms& words, std::set<Identifier>& results ) const
{
  typedef std::set<const DicTree<Atom, Identifier>*> Pebbles;
  Pebbles pebbles;
  pebbles.insert(&tree);

  results.clear();

  for ( typename Atoms::const_iterator it=words.begin(); it!=words.end(); ++it )
  {
    const Atom& word = *it;

    for ( typename Pebbles::const_iterator jt=pebbles.begin(); jt!=pebbles.end(); ++jt )
    {
      const DicTree<Atom, Identifier>* subTree = (*jt)->lookup(word) ;
      
      if (!subTree)
        continue;

      const Identifier& id = subTree->getIdentifier();
      if (id!=0)
      {
        results.insert(id);
      }

      if (!subTree->empty())
      {
        pebbles.insert(subTree);
      }
    }
  }
}

template <class Atom, class Identifier>
void SubsetLookup<Atom, Identifier>::dump( std::ostream& os ) const
{
  tree.dump(os);
}

} // namespace Hunglish


#endif // #define __HUNGLISH_TEIREADER_DICTIONARIES_H
