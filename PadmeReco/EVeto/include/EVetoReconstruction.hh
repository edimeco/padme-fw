// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2016-03-23
//
// --------------------------------------------------------------
#ifndef EVetoReconstruction_H
#define EVetoReconstruction_H

#include "PadmeVReconstruction.hh"
#include "VetoClusterHits.hh"
#include "VetoClusterization.hh"
#include "TRandom2.h"


class EVetoReconstruction : public PadmeVReconstruction
{

public:
  
  EVetoReconstruction(TFile*, TString);
  ~EVetoReconstruction();
  // virtual void ProcessEvent(TMCVEvent*,TMCEvent*);
  virtual void HistoInit();
  virtual void AnalyzeEvent(TRawEvent* evt);
  void ConvertMCDigitsToRecoHits(TMCVEvent* tEvent,TMCEvent* tMCEvent);
  void BuildHits(TRawEvent* rawEv); //Get the CH_ID MR 
  void BuildClusters(TRawEvent* rawEv);
  void BuildClusters(TMCEvent* MCEv);
  void Clusterise();
  void MergeClusters();
  virtual void ProcessEvent(TRawEvent* rawEv); //Get the CH_ID MR 
  virtual void ProcessEvent(TMCVEvent* tEvent,TMCEvent* tMCEvent); //Get the CH_ID MR 
  //void Init(PadmeVReconstruction* MainReco);
  bool TriggerToBeSkipped();
  // std::vector<Cluster*> GetClusters(){return ClusVec;}

private:

  TRandom2* random;
  Double_t  fSigmaNoiseForMC;
  Double_t  fEVetoDigiTimeWindow;
  Bool_t    fMCSimBrokenChas;
  Int_t     fClusterAlgo;

  VetoClusterHits fClusterHits;
  VetoClusterStructure fClusStruc;
  // std::vector<Cluster*> ClusVec;

};
#endif
