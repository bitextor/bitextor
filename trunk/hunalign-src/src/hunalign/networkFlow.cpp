/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#pragma warning ( disable : 4786 )

#include "networkFlow.h"

#include <portableHash.h>
#include <cassert>
#include <iostream>

#include "timer.h"

namespace Hunglish
{

DiGraph::Nodes emptySet;

const DiGraph::Nodes& DiGraph::forwardNeighbours ( int a ) const
{
  ToNodes::const_iterator ft = forward.find(a);
  if (ft==forward.end())
  {
    return emptySet;
  }
  else
  {
    return ft->second;
  }
}

const DiGraph::Nodes& DiGraph::backwardNeighbours ( int a ) const
{
  ToNodes::const_iterator ft = backward.find(a);
  if (ft==backward.end())
  {
    return emptySet;
  }
  else
  {
    return ft->second;
  }
}

bool DiGraph::isEdge( int a, int b ) const
{
  const DiGraph::Nodes& nei = forwardNeighbours (a);
  return ( nei.find(b) != nei.end() );
}


void DiGraph::addEdge( int a, int b )
{
  forward [a].insert(b);
  backward[b].insert(a);
}

void DiGraph::clear()
{
  forward.clear();
  backward.clear();
}

void NetworkWithFlow::addEdge( int a, int b, double v )
{
  assert(v>0);

  capacity[Edge(a,b)] = v;
  flow    [Edge(a,b)] = 0;

  ((DiGraph*)this)->addEdge(a,b); // ???
}

DiGraph::Nodes::const_iterator NetworkWithFlow::nextFwd( int x, DiGraph::Nodes::const_iterator it, DiGraph::Nodes::const_iterator end, double& excess )
{
  excess=0;
  Nodes::const_iterator jt=it;
  for ( ; jt!=end ; ++jt )
  {
    excess = capacity[Edge(x,*jt)] - flow[Edge(x,*jt)] ;
    if (excess>0)
    {
      break;
    }
  }
  return jt;
}

DiGraph::Nodes::const_iterator NetworkWithFlow::nextBwd( int x, DiGraph::Nodes::const_iterator it, DiGraph::Nodes::const_iterator end, double& excess )
{
  excess=0;
  Nodes::const_iterator jt=it;
  for ( ; jt!=end ; ++jt )
  {
    excess = flow[Edge(*jt,x)];
    if (excess>0)
    {
      break;
    }
  }
  return jt;
}

double NetworkWithFlow::dfs( int s, int t, std::vector<int>& path, bool justWithForwards )
{
  // FIGYELEM: Nem a legrovidebb utak kozul valasztjuk ki a legnagyobb
  // kapacitasut, hanem az osszes s-t ut kozul, tehat ez nem is Edmonds-Karp,
  // hanem Ford-Fulkerson.

  path.clear();
  typedef EXTNAMESPACE::hash_map<int,int> ReMap;
  typedef EXTNAMESPACE::hash_map<int,double> NodeValueMap;
  ReMap reMap;
  NodeValueMap nodeValueMap;

  Nodes alive;
  alive.insert(s);
  reMap[s] = -1;
  nodeValueMap[s] = 1e10; // TODO

  bool foundTarget(false);
  while (!alive.empty())
  {
    Nodes nextAlive;
    for ( Nodes::const_iterator it=alive.begin(); it!=alive.end(); ++it )
    {
      const int& oldNode = *it;
      double oldNodeValue = nodeValueMap[oldNode];

      {
        const Nodes& nei = forwardNeighbours(oldNode);
        Nodes::const_iterator endIt = nei.end();
        Nodes::const_iterator currIt = nei.begin();
        for ( ; ; )
        {
          double excess(0);
          currIt = nextFwd( oldNode, currIt, endIt, excess );
          if (currIt==endIt)
          {
            break;
          }
          int newNode = *currIt;
          ++currIt;

          double newNodeValue = (excess<oldNodeValue) ? excess : oldNodeValue ;

          NodeValueMap::iterator rt = nodeValueMap.find(newNode);
          if (rt==nodeValueMap.end())
          {
            reMap[newNode] = oldNode;
            nodeValueMap[newNode] = newNodeValue;
            nextAlive.insert(newNode);
          }
          else
          {
            // Ugyanezen a melysegen mar egyszer elertuk ot.
            // Nezzuk meg, hogy a mostani utvonalon nagyobb 
            // kapacitast tudunk-e atvinni. Ha igen, valtsunk erre.
            double oldNodeValue = rt->second;
            if (newNodeValue>oldNodeValue)
            {
              rt->second = newNodeValue;
              reMap[newNode] = oldNode;
            }
          }

          if (newNode==t)
          {
            foundTarget = true;
            // Ha megvan a t, akkor oldNode tobbi szomszedja mar erdektelen
            // a szamunkra:
            break;
          }
        }
      }

      // Ezt igazibol akkor kell eloszor igazra allitani, 
      // amikor pusztan fwd-kkel mar nem erheto el t.
      if (!justWithForwards)
      {
        const Nodes& nei = backwardNeighbours(oldNode);
        Nodes::const_iterator endIt = nei.end();
        Nodes::const_iterator currIt = nei.begin();
        for ( ; ; )
        {
          double excess(0);
          currIt = nextBwd( oldNode, currIt, endIt, excess );
          if (currIt==endIt)
          {
            break;
          }
          int newNode = *currIt;
          ++currIt;

          double newNodeValue = (excess<oldNodeValue) ? excess : oldNodeValue ;

          NodeValueMap::iterator rt = nodeValueMap.find(newNode);
          if (rt==nodeValueMap.end())
          {
            assert(newNodeValue>0);
            reMap[newNode] = oldNode;
            nodeValueMap[newNode] = newNodeValue;
            nextAlive.insert(newNode);
          }
          else
          {
            // Ugyanezen a melysegen mar egyszer elertuk ot.
            // Nezzuk meg, hogy a mostani utvonalon nagyobb 
            // kapacitast tudunk-e atvinni. Ha igen, valtsunk erre.
            double oldNodeValue = rt->second;
            if (newNodeValue>oldNodeValue)
            {
              rt->second = newNodeValue;
              reMap[newNode] = oldNode;
            }
          }

          if (newNode==t)
          {
            foundTarget = true;
            // Ha megvan a t, akkor oldNode tobbi szomszedja mar erdektelen
            // a szamunkra:
            break;
          }
        }
      }

      // Az uj valtozatban akkor is vegigdaralunk a szinten, ha megtalaltuk
      // a t csucsot. Hatha mashonnan jobban meg tudjuk kozeliteni.
      /* if (foundTarget) break; */
    }
    if (foundTarget)
    {
      break;
    }
    // De kover masolas.
    alive = nextAlive;
  }

  if (!foundTarget)
  {
    return 0;
  }

  int curr = t;
  while (curr!=s)
  {
    double val = nodeValueMap[curr];
    path.push_back(curr);
    curr = reMap[curr];
  }
  path.push_back(curr);

  std::reverse(path.begin(),path.end());

  assert(nodeValueMap[t]>0);
  return nodeValueMap[t];
}

double NetworkWithFlow::evaluateAugmentation( const std::vector<int>& path )
{
  double improve=1e10;

  int i;
  for ( i=0; i+1<path.size(); ++i )
  {
    int a = path[i];
    int b = path[i+1];

    double freeCap(0);
    if (isEdge(a,b))
    {
      freeCap = capacity[Edge(a,b)] - flow[Edge(a,b)] ;
    }

    if (isEdge(b,a))
    {
      // Ha vannak ellentetes iranyu (a,b) (b,a) elek, es mindketto augmentalo, akkor csak a nagyobb kapacitasut hasznalom.
      // Ez jelentektelen mertekben lassit.
      if ( flow[Edge(b,a)] > freeCap )
        freeCap = flow[Edge(b,a)] ;
    }

    if (freeCap==0)
    {
      throw "internal error";
    }

    if (freeCap<improve)
    {
      improve = freeCap;
    }
  }
  return improve;
}

void NetworkWithFlow::augment( const std::vector<int>& path, const double& improve )
{
  bool backwardsUsed(false);
  for ( int i=0; i+1<path.size(); ++i )
  {
    int a = path[i];
    int b = path[i+1];

    if ( isEdge(a,b) && ( capacity[Edge(a,b)] >= flow[Edge(a,b)]+improve ) )
    {
      flow[Edge(a,b)] += improve;
    }
    else if (isEdge(b,a) && ( flow[Edge(b,a)] >= improve ) )
    {
      flow[Edge(b,a)] -= improve;
      backwardsUsed = true;
    }
    else
    {
      throw "internal error";
    }
  }
  if (backwardsUsed)
  {
    std::cerr << "Backwards ";
  }
  else
  {
    std::cerr << "Forwards  ";
  }
  std::cerr << path.size()-1 << " ";
}

void NetworkWithFlow::edmondsKarp( int s, int t )
{
  // FIGYELEM: Nem a legrovidebb utak kozul valasztjuk ki a legnagyobb
  // kapacitasut, hanem az osszes s-t ut kozul, tehat ez nem is Edmonds-Karp,
  // hanem Ford-Fulkerson.

  Ticker ticker;
  std::cerr << "Starting Edmonds-Karp..." << std::endl;

  // Az elso fazisban mindig csak elore-elek menten novelunk,
  // minden korben moho modon az elore-utak
  // kozul a legnagyobb szabad kapacitasuval.
  // Igy a dfs ciklus belsejeben nem kell megnezni a hatra-eleket,
  // ami ott egy ketszeres gyorsitast jelent.
  while (true)
  {
    std::vector<int> path;

    double excess = dfs( s, t, path, true/*justWithForwards*/ );
    if (excess>0)
    {
      /*
      double recheckExcess = evaluateAugmentation(path);
      assert( recheckExcess >= excess-0.01 );
      assert( recheckExcess <= excess+0.01 );
      */

      augment(path,excess);
    }
    else
    {
      break;
    }
    dumpFlow( std::cerr, s );
  }

  std::cerr << "Greedy Phase Done in " << ticker.next() << " ms." << std::endl;

  // A masodik fazisban mar a hatra-eleket is figyeljuk.
  // Most is az OSSZES augmentalo utak kozul
  // a legnagyobb szabad kapacitasuval javitunk.
  while (true)
  {
    std::vector<int> path;

    double excess = dfs( s, t, path, false/*justWithForwards*/ );
    if (excess>0)
    {
      /*
      double recheckExcess = evaluateAugmentation(path);
      assert( recheckExcess >= excess-0.01 );
      assert( recheckExcess <= excess+0.01 );
      */

      augment(path,excess);
    }
    else
    {
      break;
    }
    dumpFlow( std::cerr, s );
  }

  std::cerr << "Second Phase Done in " << ticker.next() << " ms." << std::endl;
}

void NetworkWithFlow::dumpFlow( std::ostream& os, int s ) const
{
  double value(0);
  for ( Valuation::const_iterator it=flow.begin(); it!=flow.end(); ++it )
  {
    // os << it->first.first << "," << it->first.second << "\t" << it->second << std::endl;
    if (it->first.first == s)
    {
      value += it->second ;
    }
  }
  os << "Value: " << value << std::endl;
}


void main_edmondsKarpTest()
{
  NetworkWithFlow nw;

  nw.addEdge( 1, 2, 5 );
  nw.addEdge( 1, 3, 6 );
  nw.addEdge( 2, 4, 5 );
  nw.addEdge( 2, 5, 6 );
  nw.addEdge( 3, 4, 5 );
  nw.addEdge( 3, 5, 6 );
  nw.addEdge( 4, 6, 6 );
  nw.addEdge( 5, 6, 5 );

  int s=1;
  int t=6;
  nw.edmondsKarp(s,t);
}

} // namespace Hunglish
