#ifndef PadmeAnalysisEvent_h
#define PadmeAnalysisEvent_h 1

#include "TRecoEvent.hh"
#include "TTargetRecoEvent.hh"
#include "TTargetRecoBeam.hh"
#include "TECalRecoEvent.hh"
#include "TECalMLRecoEvent.hh"
#include "TPVetoRecoEvent.hh"
#include "TEVetoRecoEvent.hh"
#include "TSACRecoEvent.hh"
#include "THEPVetoRecoEvent.hh"
#include "TRecoVHit.hh"
#include "TMCTruthEvent.hh"
#include "TLeadGlassRecoEvent.hh"


class PadmeAnalysisEvent{
public:

  PadmeAnalysisEvent();
  ~PadmeAnalysisEvent(){;};


  TRecoEvent*                     RecoEvent            ;
  TTargetRecoEvent*               TargetRecoEvent      ;
  TEVetoRecoEvent*                EVetoRecoEvent       ;
  TPVetoRecoEvent*                PVetoRecoEvent       ;
  THEPVetoRecoEvent*              HEPVetoRecoEvent     ;
  TECalRecoEvent*                 ECalRecoEvent        ;
  TECalMLRecoEvent*               ECalMLRecoEvent      ;
  TSACRecoEvent*                  SACRecoEvent         ;
  TTargetRecoBeam*                TargetRecoBeam       ;
  TRecoVClusCollection*           SACRecoCl            ;
  TRecoVClusCollection*           ECalRecoCl           ;
  TRecoVClusCollection*           ECalMLRecoCl           ;
  TRecoVClusCollection*           PVetoRecoCl          ;
  TRecoVClusCollection*           EVetoRecoCl          ;
  TRecoVClusCollection*           HEPVetoRecoCl        ;
  TMCTruthEvent*                  MCTruthEvent         ;
  TLeadGlassRecoEvent*            LeadGlassRecoEvent   ;


};
#endif
