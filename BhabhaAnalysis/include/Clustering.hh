#ifndef Clustering_h
#define Clustering_h 1
#define MAXHIT 100

#include "ClusterHits.hh"
#include <iostream>

class Cluster{
public:
  Cluster();
  ~Cluster(){};

  int InsertHit(ClusterHits hit, int ihit);
  int* GetHitIndex(){return hitIndex;}
  int GetMostUpstreamChannel(){return mostUpstreamChannel;}
  int GetMostDownstreamChannel(){return mostDownstreamChannel;}
  double GetAverageTime(){return averagetime;}
  int GetNHits(){return nhits;}
  int AddCluster(Cluster* cluster);
  
private:
  int hitIndex[MAXHIT];
  int mostUpstreamChannel;
  int mostDownstreamChannel;
  double averagetime;
  int nhits;
  
};

class ClusterStructure{
  
public:
  ClusterStructure();
  ~ClusterStructure(){};

  void Clear();
  
  void AddHit(ClusterHits MyHit, int hitindex){    
    HitIndexVec.push_back(hitindex);
    HitVec.push_back(MyHit);
  }

  void HitSort();

  void Clusterise();

  void MergeClusters();

  std::vector<Cluster*> GetClusters(){return ClusVec;}
  std::vector<ClusterHits> GetHitVec(){return HitVec;}
  
private:
  std::vector<ClusterHits> HitVec;
  std::vector<Cluster*> ClusVec;
  std::vector<int> HitIndexVec;
};

#endif
