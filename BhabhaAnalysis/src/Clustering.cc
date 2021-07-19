#include "Clustering.hh"
#include "ClusterHits.hh"

#include <iostream>
#include <algorithm>

Cluster::Cluster(){
  nhits=0;
  mostUpstreamChannel=100;
  mostDownstreamChannel=-1;
  averagetime=0;
}

int Cluster::InsertHit(ClusterHits* hit, int ihit){
  if(nhits>MAXHIT-1){
    std::cout<<"Too many hits, exiting"<<std::endl;
    return -1;
  }
  hitIndex[nhits]=ihit;
  if(hit->GetChannelId()<mostUpstreamChannel) mostUpstreamChannel=hit->GetChannelId();
  if(hit->GetChannelId()>mostDownstreamChannel) mostDownstreamChannel=hit->GetChannelId();
  averagetime = (nhits*averagetime+hit->GetTime())/(nhits+1);
  nhits++;
}

int Cluster::AddCluster(Cluster* newcluster){

  int NMax=newcluster->GetNHits()+nhits;
  //  std::cout<<nhits<<" "<<newcluster->GetNHits()<<" "<<NMax<<std::endl;
  int goodreturn=1;
  if(newcluster->GetNHits()+nhits>MAXHIT){
    std::cout<<"MAXHIT too small, too many hits"<<std::endl;
    NMax=MAXHIT;
    goodreturn=-1;
  }
  for(int ii=0;ii<NMax;ii++){
    hitIndex[nhits+ii]=newcluster->GetHitIndex()[ii];
  }
  if(newcluster->GetMostUpstreamChannel()<mostUpstreamChannel) mostUpstreamChannel=newcluster->GetMostUpstreamChannel();
  if(newcluster->GetMostDownstreamChannel()>mostDownstreamChannel) mostDownstreamChannel=newcluster->GetMostDownstreamChannel();
  averagetime=(nhits*averagetime+newcluster->GetNHits()*newcluster->GetAverageTime())/(nhits+newcluster->GetNHits());
  nhits=NMax;

  return goodreturn;
}


ClusterStructure::ClusterStructure(){
  HitVec.clear();
  ClusVec.clear();
  HitIndexVec.clear();
}

void ClusterStructure::Clear(){
  ClusVec.clear();
  HitVec.clear();//legit???
  HitIndexVec.clear();
  }

void ClusterStructure::Clusterise(){
  for (int ii=0; ii <HitVec.size(); ++ii ) {
    bool UsedHit = 0;
    for (int jj = 0; jj <ClusVec.size(); ++jj ) {
      if(HitVec.at(ii)->GetTime()-ClusVec.at(jj)->GetAverageTime()<4&&
	 (HitVec.at(ii)->GetChannelId()-ClusVec.at(jj)->GetMostUpstreamChannel()==-1//changed 6/7/21
	  ||HitVec.at(ii)->GetChannelId()-ClusVec.at(jj)->GetMostDownstreamChannel()==1)){	//4->parameter
	ClusVec.at(jj)->InsertHit(HitVec.at(ii), HitIndexVec.at(ii));
	UsedHit=1;
	//	std::cout<<"clusterising jj "<<jj<<" size "<<ClusVec.at(jj)->GetNHits()<<std::endl;
	break;
      }	
    }//end cluster loop
    if(!UsedHit) {
      Cluster* NewCluster = new Cluster();
      NewCluster->InsertHit(HitVec.at(ii), HitIndexVec.at(ii));
      ClusVec.push_back(NewCluster);
    }
  }
}


void ClusterStructure::MergeClusters(){
   Int_t noCompact = 0;
  while(noCompact==0) {
    //    std::cout<<"ClusVec.size() "<<ClusVec.size()<<std::endl;
    noCompact = 1;
    int ii = 0;
    while(ii+1< ClusVec.size()){
      //std::cout<<"ii "<<ii<<" size "<<ClusVec.size()<<" "<<typeid(ClusVec.at(ii)).name()<<std::endl;
      //qualche volta anche se ClusVec.size()!=0, ClusVec.at(ii)->GetNhits() fa segmentation violation. Non lo fa sempre e non riesco a capire quando/perche'
      //std::cout<<"ii "<<ii<<" Nhits "<<ClusVec.at(ii)->GetNHits()<<std::endl;
      int iUp = ClusVec.at(ii)->GetMostUpstreamChannel();
      int iDown = ClusVec.at(ii)->GetMostDownstreamChannel();
      double iAvgT = ClusVec.at(ii)->GetAverageTime();
      int jj = ii+1; 
      while(jj < ClusVec.size()){
	int jUp = ClusVec.at(jj)->GetMostUpstreamChannel();
	int jDown = ClusVec.at(jj)->GetMostDownstreamChannel();
	double jAvgT = ClusVec.at(jj)->GetAverageTime();
	//	std::cout<<"jj "<<jj<<" jUp "<<jUp<<" jDown "<<jDown<<" jAvgT "<<jAvgT<<std::endl;
	if(std::abs(iAvgT-jAvgT)<4&&(std::abs(iUp-jDown==1)||std::abs(jUp-iDown==1))){
	  int goodorbad = ClusVec.at(ii)->AddCluster(ClusVec.at(jj));//adds new hits to existing cluster & updates mostUpstreamChannel, mostDownstreamChannel and averagetime.
	  //std::cout<<"gonna erase "<<jj<<std::endl;
	  //if(jj==1) std::cout<<"TAKE NOTICE OF ME!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
	  if(jj<ClusVec.size()-1) ClusVec.erase(ClusVec.begin()+jj);//cluster jj has already been used so get rid of it, and move all other clusters down in the vector
	  if(jj==ClusVec.size()-1) ClusVec.pop_back();//
	  noCompact = 0;
	  for(int kk = 0;kk<ClusVec.size();kk++){
	    //    std::cout<<kk<<" upstream "<<ClusVec.at(kk)->GetMostUpstreamChannel()<<" downstream "<<ClusVec.at(kk)->GetMostDownstreamChannel()<<" avgT "<<ClusVec.at(kk)->GetAverageTime()<<std::endl;

	  }
	  //	  std::cout<<goodorbad<<std::endl;
	}//end if(clusterise?)
	else	++jj;
      }
      ++ii;
    }
  }
  /*  Int_t noCompact = 0;
  while(noCompact==0) {
    std::cout<<"ClusVec.size() "<<ClusVec.size()<<std::endl;
    noCompact = 1;
    int indexii=0;
    for(std::vector<Cluster*>::iterator ii = ClusVec.begin(); ii+1!= ClusVec.end();){
      indexii++;
      std::cout<<"ii "<<indexii<<" size "<<ClusVec.size()<<" nocompact "<<noCompact<<std::endl;//typeid(ii).name()<<std::endl;
      //qualche volta anche se ClusVec.size()!=0, ClusVec.at(ClusVec.begin()+ii)->GetNhits() fa segmentation violation. Non lo fa sempre e non riesco a capire quando/perche'
      std::cout<<"ii "<<indexii<<" Nhits "<<(*ii)->GetNHits()<<std::endl;
      int iUp = (*ii)->GetMostUpstreamChannel();
      int iDown = (*ii)->GetMostDownstreamChannel();
      double iAvgT = (*ii)->GetAverageTime();
      int indexjj=0;
      for(std::vector<Cluster*>::iterator jj = ii+1; jj!=ClusVec.end();){
	indexjj++;
	int jUp = (*jj)->GetMostUpstreamChannel();
	int jDown = (*jj)->GetMostDownstreamChannel();
	double jAvgT = (*jj)->GetAverageTime();
	std::cout<<"jj "<<indexjj<<" jUp "<<jUp<<" jDown "<<jDown<<" jAvgT "<<jAvgT<<" size "<<ClusVec.size()<<std::endl;
	if(std::abs(iAvgT-jAvgT)<4&&(iUp-jDown==1||jUp-iDown==1)){
	  int goodorbad = (*ii)->AddCluster(*jj);//adds new hits to existing cluster & updates mostUpstreamChannel, mostDownstreamChannel and averagetime.
	  std::cout<<"gonna erase ii"<<indexii<<" jj "<<indexjj<<std::endl;
	  //	  if(jj==1) std::cout<<"IT HAPPENS HERE!!!!!!!!!"<<std::endl;
	  jj=ClusVec.erase(jj);//cluster jj has already been used so get rid of it, and move all other clusters down in the vector
	  noCompact = 0;
	  std::cout<<goodorbad<<" size "<<ClusVec.size()<<" noCompact "<<noCompact<<std::endl;
	}//end if(clusterise?)
	else{
	++jj;
	std::cout<<"else size "<<ClusVec.size()<<" noCompact "<<noCompact<<std::endl;	
	}
      }//end jj loop
      ii++;
      std::cout<<"ii increase "<<ClusVec.size()<<" noCompact "<<noCompact<<std::endl;	
    }
  }*/
}

void ClusterStructure::HitSort(){

  std::vector<ClusterHits*> HitVecCopy(HitVec);
    
  std::vector<int> index(HitVec.size(), 0);
  
  for (int i = 0 ; i != index.size() ; i++) {
    index[i] = i;
  }
  
  sort(index.begin(), index.end(),
       [&](const int& a, const int& b) {
	 return (HitVec[a]->GetTime() < HitVec[b]->GetTime());
	 }
       );
  
  for (int ii = 0 ; ii != index.size() ; ++ii) {
    HitVec[ii]=(HitVecCopy[index[ii]]);
    HitIndexVec[ii]=index[ii];
    //    std::cout<<"ii "<<ii<<" Time[ii] "<<HitVec[ii]->GetTime()<<std::endl;
    if(ii>0&&HitVec[ii]->GetTime()-HitVec[ii-1]->GetTime()<0) {
      std::cout<<"----------------YOU'RE IN A MESS MY FRIEND---------------"<<std::endl;
      return;
    }
  }

}
