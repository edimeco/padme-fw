#ifndef Clustering_h
#define Clustering_h 1
#define MAXHIT 100

#include "TMCVHit.hh"

class Cluster{
public:
  Cluster();
  ~Cluster(){};

  int InsertHit(TMCVHit* hit, int ihit);
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

  //  void Initialise();
  
  void AddHit(TMCVHit* MyHit, int hitindex){    
    HitVec.push_back(MyHit);
    HitIndexVec.push_back(hitindex);
  }

  void HitSort();

  void Clusterise();

  void MergeClusters();

  std::vector<Cluster*> GetClusters(){return ClusVec;}
  
  
private:
  std::vector<TMCVHit*> HitVec;
  std::vector<Cluster*> ClusVec;
  std::vector<int> HitIndexVec;
};

#endif
