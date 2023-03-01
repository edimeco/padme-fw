#include "Riostream.h"

#include <TString.h>

#include "PadmeVRecoConfig.hh"

#include "TRawEvent.hh"

#include "HistoSvc.hh"
#include "TrigTimeSvc.hh"
#include "RunConditionSvc.hh"
#include "RunConfigurationSvc.hh"

#include "ETagDigitizer.hh"
#include "ETagHit.hh"
#include "ETagCluster.hh"

#include "ETagReconstruction.hh"

ETagReconstruction::ETagReconstruction(TString ConfigFileName)
{

  // Attach global services
  fHistoSvc = HistoSvc::GetInstance();
  fTrigTimeSvc = TrigTimeSvc::GetInstance();
  fRunConditionSvc = RunConditionSvc::GetInstance();
  fRunConfigurationSvc = RunConfigurationSvc::GetInstance();

  // Enable ETag specific configuration service
  fETagConfig = new PadmeVRecoConfig(new utl::ConfigParser(ConfigFileName.Data()),"ETag");

  // Define verbose level
  fVerbose = fETagConfig->GetParOrDefault("Output", "Verbose", 0);

  // Create ETag digitizer
  fETagDigitizer = new ETagDigitizer(fETagConfig);

  // Check if Hits and Clusters must be written to the output file
  fWriteHits     = (Bool_t)fETagConfig->GetParOrDefault("Output", "Hits"     , 1);
  fWriteClusters = (Bool_t)fETagConfig->GetParOrDefault("Output", "Clusters" , 1);

  if (fVerbose) printf("ETagReconstruction::ETagReconstruction - ETag reconstruction system created\n");

}

ETagReconstruction::~ETagReconstruction()
{
  if (fVerbose) printf("ETagReconstruction::~ETagReconstruction - Deleting ETag reconstruction system\n");
  delete fETagDigitizer;
  delete fETagConfig;
}

void ETagReconstruction::Init()
{
  if (fVerbose>2) printf("ETagReconstruction::Init - Initializing ETag reconstruction\n");

  fETagDigitizer->Init();

}

void ETagReconstruction::ProcessEvent(TMCVEvent* tEvent,TMCEvent* tMCEvent)
{
  if (fVerbose>2) printf("ETagReconstruction::ProcessEvent - Reconstructing MC event\n");
}

void ETagReconstruction::ProcessEvent(TETagRecoEvent* tEvent,TRecoEvent* tRecoEvent)
{
  if (fVerbose>2) printf("ETagReconstruction::ProcessEvent - Reprocessing reconstructed event\n");
}

void ETagReconstruction::ProcessEvent(TRawEvent* rawEvent)
{
  if (fVerbose>2) printf("ETagReconstruction::ProcessEvent - Reconstructing RawData event\n");

  // Show run energy (only when run changes)
  static Int_t run = 0;
  if (fRunConditionSvc->GetCurrentRun() != run) {
    run = fRunConditionSvc->GetCurrentRun();
    if (fVerbose) printf("ETagReconstruction::ProcessEvent - Run %d has energy %.3f\n",run,fRunConditionSvc->GetRunEnergy());
  }

  fHits.clear();
  fClusters.clear();

  fETagDigitizer->BuildHits(rawEvent,fHits);
 
  ETagCluster* clus = new ETagCluster();
  clus->SetStatus(0x0000000000000003);
  clus->SetPosition(TVector3(1.,1.,2.));
  clus->SetEnergy(324.3);
  clus->SetTime(23456.456);
  clus->SetSeed(0);
  clus->SetNHits(1);
  std::vector<Int_t> v;
  v.push_back(0);
  clus->SetHitsVector(v);
  fClusters.push_back(clus);
  //if (fVerbose>3) clus->Print();
}

void ETagReconstruction::EndProcessing()
{
  if (fVerbose) printf("ETagReconstruction::EndProcessing - Finalizing ETag reconstruction\n");
}