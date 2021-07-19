#ifndef Clustering_h
#define Clustering_h 1
#define MAXHIT 100

#include "ClusterHits.hh"
#include <iostream>

class Cluster{
public:
  Cluster();
  ~Cluster(){};

  int InsertHit(ClusterHits* hit, int ihit);
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
  
  void AddHit(ClusterHits* MyHit, int hitindex){    
    //    std::cout<<"adding hits, time "<<MyHit->GetTime()<<" hitvec size "<<HitVec.size()<<" HitIndexVec size "<<HitIndexVec.size()<<std::endl;
    HitIndexVec.push_back(hitindex);
    //std::cout<<"size "<<HitVec.size()<<" "<<HitIndexVec.size()<<std::endl;
    HitVec.push_back(MyHit);
    //std::cout<<"hit pushed"<<std::endl;
  }

  void HitSort();

  void Clusterise();

  void MergeClusters();

  std::vector<Cluster*> GetClusters(){return ClusVec;}
  
  
private:
  std::vector<ClusterHits*> HitVec;
  std::vector<Cluster*> ClusVec;
  std::vector<int> HitIndexVec;
};

#endif
