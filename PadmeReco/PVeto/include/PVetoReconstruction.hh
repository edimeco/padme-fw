// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2016-03-23
//
// --------------------------------------------------------------
#ifndef PVetoReconstruction_H
#define PVetoReconstruction_H

#include "PadmeVReconstruction.hh"
#include "TRandom2.h"


class PVetoReconstruction : public PadmeVReconstruction
{

public:
  
  PVetoReconstruction(TFile*, TString);
  ~PVetoReconstruction();
  // virtual void ProcessEvent(TMCVEvent*,TMCEvent*);
  virtual void HistoInit();
  virtual void AnalyzeEvent(TRawEvent* evt);
  void ConvertMCDigitsToRecoHits(TMCVEvent* tEvent,TMCEvent* tMCEvent);
  void BuildHits(TRawEvent* rawEv); //Get the CH_ID MR 
  virtual void ProcessEvent(TRawEvent* rawEv); //Get the CH_ID MR 
  //void Init(PadmeVReconstruction* MainReco);
  bool TriggerToBeSkipped();

private:

  TRandom2* random;
  Double_t fSigmaNoiseForMC;
  Double_t fPVetoDigiTimeWindow;

};
#endif
