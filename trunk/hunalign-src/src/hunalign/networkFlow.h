/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/

#ifndef __HUNGLISH_ALIGNMENT_NETWORKFLOW_H
#define __HUNGLISH_ALIGNMENT_NETWORKFLOW_H

#include <set>
#include <map>
#include <vector>
#include <iostream>

namespace Hunglish
{

// It cannot represent graphs with isolated vertices.
// But you don't really need them, do you?
// I could add an addNode class, and throw an exception
// when forwardNeighbours-ing a non-node.
class DiGraph
{
public:
  void addEdge( int a, int b );
  bool isEdge ( int a, int b ) const;
  void clear();

public:
  typedef std::set<int> Nodes;

  const Nodes& forwardNeighbours ( int a ) const;
  const Nodes& backwardNeighbours( int a ) const;

private:
  typedef std::map< int, Nodes > ToNodes;

  ToNodes forward;
  ToNodes backward;
};


class NetworkWithFlow : public DiGraph
{
public:
  typedef std::pair<int,int> Edge;
  typedef std::map<Edge,double> Valuation;

public:
  void addEdge( int a, int b, double v );
  void edmondsKarp( int s, int t );
  const Valuation& getFlow() const
  {
    return flow;
  }
  const Valuation& getCapacity() const
  {
    return capacity;
  }
  void dumpFlow( std::ostream& os, int s ) const;

private:

  double dfs( int s, int t, std::vector<int>& path, bool justWithForwards );
  void augment( const std::vector<int>& path, const double& excess );
  double evaluateAugmentation( const std::vector<int>& path );
  DiGraph::Nodes::const_iterator nextFwd
    ( int x, DiGraph::Nodes::const_iterator it, DiGraph::Nodes::const_iterator end, double& excess );
  DiGraph::Nodes::const_iterator nextBwd
    ( int x, DiGraph::Nodes::const_iterator it, DiGraph::Nodes::const_iterator end, double& excess );

private:
  Valuation capacity;
  Valuation flow;
};

} // namespace Hunglish

#endif // #define __HUNGLISH_ALIGNMENT_NETWORKFLOW_H
