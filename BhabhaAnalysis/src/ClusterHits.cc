#include "ClusterHits.hh"

#include <iostream>
#include "cstdlib"
#include "math.h"

ClusterHits::ClusterHits()
{

  // Initialize to invalid channel
  fChannelId = -1;
  fEnergy    = 0.;
  fTime      = 1.E9;
}

void ClusterHits::Clear(){
  // Initialize to invalid channel
  fChannelId = -1;
  fEnergy    = 0.;
  fTime      = 1.E9;
  fPosition  = TVector3(-1.E9,-1.E9,-1.E9);
}

ClusterHits::ClusterHits(const ClusterHits& h){
  // Initialize to invalid channel
  fChannelId = h.fChannelId;
  fPosition  = h.fPosition;
  fEnergy    = h.fEnergy;
  fTime      = h.fTime;                                                                             }                                       
ClusterHits::~ClusterHits()
{;}

void ClusterHits::Print()
{
  std::cout << "Hit - ChannelId = " << fChannelId
            << " HitPosition = (" << fPosition.X() << "," << fPosition.Y() << "," << fPosition.Z() << ")"
            << " Energy = " << fEnergy
            << " Time = " << fTime << std::endl;
}
