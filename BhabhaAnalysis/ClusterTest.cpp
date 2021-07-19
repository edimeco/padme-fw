#include "Riostream.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <sys/stat.h>

#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TBranch.h"
#include "TObjArray.h"
#include "TH2F.h"
#include "TProfile.h"

#include "TMCEvent.hh"
#include "TRecoEvent.hh"

#include "TTargetRecoEvent.hh"
#include "TEVetoRecoEvent.hh"
#include "TPVetoRecoEvent.hh"
#include "THEPVetoRecoEvent.hh"
#include "TECalRecoEvent.hh"
#include "TSACRecoEvent.hh"

#include "TTargetMCEvent.hh"
#include "TEVetoMCEvent.hh"
#include "TPVetoMCEvent.hh"
#include "THEPVetoMCEvent.hh"
#include "TECalMCEvent.hh"
#include "TSACMCEvent.hh"
//#include "TTPixMCEvent.hh"
#include <algorithm>
#include "TMCVHit.hh"

#include "Clustering.hh"
#include "ClusterHits.hh"


int main(int argc, char* argv[])
{  

  int c;
  int verbose = 0;
  int nevents = 0;

  TString inputFileName;
  TString OutputFileName("OutputPlots.root");
  TObjArray inputFileNameList;
  struct stat filestat;

  // Parse options
  while ((c = getopt (argc, argv, "i:l:o:n:v:h")) != -1) {
    switch (c)
      {
      case 'i':
        inputFileNameList.Add(new TObjString(optarg));
	//        fprintf(stdout,"Added input data file '%s'\n",optarg);
	break;
      case 'l':
	if ( stat(Form(optarg),&filestat) == 0 ) {
	  //	  fprintf(stdout,"Reading list of input files from '%s'\n",optarg);
	  std::ifstream inputList(optarg);
	  while( inputFileName.ReadLine(inputList) ){
	    if ( stat(Form(inputFileName.Data()),&filestat) == 0 ) {
	      inputFileNameList.Add(new TObjString(inputFileName.Data()));
	      //	      fprintf(stdout,"Added input data file '%s'\n",inputFileName.Data());
	    } else {
	      fprintf(stdout,"WARNING: file '%s' is not accessible\n",inputFileName.Data());
	    }
	  }
	} else {
	  fprintf(stdout,"WARNING: file list '%s' is not accessible\n",optarg);
	}
        break;
      case 'o':
	//	no++;
	OutputFileName = TString(optarg);
	break;
      case 'n':
        if ( sscanf(optarg,"%d",&nevents) != 1 ) {
          fprintf (stderr, "Error while processing option '-n'. Wrong parameter '%s'.\n", optarg);
          exit(1);
        }
        if (nevents<0) {
          fprintf (stderr, "Error while processing option '-n'. Required %d events (must be >=0).\n", nevents);
          exit(1);
        }
	if (nevents) {
	  fprintf(stdout,"Will read first %d events in file\n",nevents);
	} else {
	  fprintf(stdout,"Will read all events in file\n");
	}
        break;
      case 'v':
        if ( sscanf(optarg,"%d",&verbose) != 1 ) {
          fprintf (stderr, "Error while processing option '-v'. Wrong parameter '%s'.\n", optarg);
          exit(1);
        }
        if (verbose<0) {
          fprintf (stderr, "Error while processing option '-v'. Verbose level set to %d (must be >=0).\n", verbose);
          exit(1);
        }
        fprintf(stdout,"Set verbose level to %d\n",verbose);
        break;
      case 'h':
        fprintf(stdout,"\nReadTest [-i input root file] [-l list of input files] [-v verbosity] [-h]\n\n");
        fprintf(stdout,"  -i: define an input file in root format\n");
        fprintf(stdout,"  -l: define a list of input files\n");
        fprintf(stdout,"  -n: define number of events to read from input file (0: all events)\n");
        fprintf(stdout,"  -v: define verbose level\n");
        fprintf(stdout,"  -h: show this help message and exit\n\n");
        exit(0);
      case '?':
        if (optopt == 'v') {
          // verbose with no argument: just enable at minimal level
          verbose = 1;
          break;
        } else if (optopt == 'i' || optopt == 'l' || optopt == 'o')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint(optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
        exit(1);
      default:
        abort();
      }
  }

  if ( inputFileNameList.GetEntries() == 0 ) {
    perror(Form("ERROR No Input File specified"));
    exit(1);
  }

  struct stat buffer;
  if (!OutputFileName.EndsWith(".root") && !stat(OutputFileName.Data(), &buffer)) {
    std::cout << " [PadmeReco] Output file exists and is not *.root: potentially a destructive call" << std::endl;
    return 0;
  }

  TFile* OutputFile = new TFile(OutputFileName,"RECREATE");

  TRecoEvent* fRecoEvent;
  TTargetRecoEvent* fTargetRecoEvent;
  TEVetoRecoEvent* fEVetoRecoEvent;
  TPVetoRecoEvent* fPVetoRecoEvent;
  THEPVetoRecoEvent* fHEPVetoRecoEvent;
  TECalRecoEvent* fECalRecoEvent;
  TSACRecoEvent* fSACRecoEvent;
  //  TTPixRecoEvent* fTPixRecoEvent;

  TMCEvent* fMCEvent;
  TTargetMCEvent* fTargetMCEvent;
  TEVetoMCEvent* fEVetoMCEvent;
  TPVetoMCEvent* fPVetoMCEvent;
  THEPVetoMCEvent* fHEPVetoMCEvent;
  TECalMCEvent* fECalMCEvent;
  TSACMCEvent* fSACMCEvent;
  //TTPixMCEvent* fTPixMCEvent;

  // Create chain of input files
  //fprintf(stdout,"=== === === Chain of input files === === ===\n");
  /*TChain* fMCChain = new TChain("MC");
  for (Int_t iFile = 0; iFile < inputFileNameList.GetEntries(); iFile++) {
    // fprintf(stdout,"%4d %s\n",iFile,((TObjString*)inputFileNameList.At(iFile))->GetString().Data());
    fMCChain->AddFile(((TObjString*)inputFileNameList.At(iFile))->GetString());
  }
  if (fMCChain->GetEntries() == 0) {
    perror(Form("ERROR No events found for tree 'MCEvents' in input chain"));
    exit(1);
    }*/

  Int_t Data;//0=MC, 1=Data
  Int_t nEntries;

  nEntries = 0;
  TString mcTreeName = "MC";
  TChain*  fMCChain = new TChain(mcTreeName);
  std::cout<<" Looking for tree named "<<mcTreeName<<std::endl;

  for(Int_t iFile = 0; iFile < inputFileNameList.GetEntries(); iFile++)
    fMCChain->AddFile(((TObjString*)inputFileNameList.At(iFile))->GetString());
  if(fMCChain->GetEntries() == 0){
    delete fMCChain;
    fMCChain = 0;
  }

  TChain*  inputChain;

  if(fMCChain) {
    Data=0;
    inputChain=fMCChain;
    nEntries = inputChain->GetEntries();
    std::cout<<" Tree named "<<mcTreeName<<" found with "<<nEntries<<" events"<<std::endl;
    TObjArray* branches = inputChain->GetListOfBranches();
    for(Int_t iBranch = 0; iBranch < branches->GetEntries(); iBranch++){

      TString branchName = ((TBranch*)(*branches)[iBranch])->GetName();
      TClass* branchObjectClass = TClass::GetClass(((TBranch*)(*branches)[iBranch])->GetClassName());
      std::cout << "Found Branch " << branchName.Data() << " containing " << branchObjectClass->GetName() << std::endl;

      if (branchName=="Event") {
	fMCEvent = new TMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fMCEvent);
      } else if (branchName=="Target") {
	fTargetMCEvent = new TTargetMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fTargetMCEvent);
      } else if (branchName=="EVeto") {
	fEVetoMCEvent = new TEVetoMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fEVetoMCEvent);
      } else if (branchName=="PVeto") {
	fPVetoMCEvent = new TPVetoMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fPVetoMCEvent);
      } else if (branchName=="HEPVeto") {
	fHEPVetoMCEvent = new THEPVetoMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fHEPVetoMCEvent);
      } else if (branchName=="ECal") {
	fECalMCEvent = new TECalMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fECalMCEvent);
      } else if (branchName=="SAC") {
	fSACMCEvent = new TSACMCEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fSACMCEvent);
      } 
    }

  }
  else std::cout << " Tree " << mcTreeName << " not found "<<std::endl;
  
  
  // input are reco hits
  nEntries = 0;
  TString recoTreeName = "Events";
  TChain*  fRecoChain = new TChain(recoTreeName);
  std::cout<<" Looking for tree named "<<recoTreeName<<std::endl;
  
  for(Int_t iFile = 0; iFile < inputFileNameList.GetEntries(); iFile++){
    //    std::cout<<"here "<<((TObjString*)inputFileNameList.At(iFile))->GetString()<<std::endl;
    fRecoChain->AddFile(((TObjString*)inputFileNameList.At(iFile))->GetString());
  }
  if(fRecoChain->GetEntries() == 0){
    delete fRecoChain;
    fRecoChain = 0;
  }

  if(fRecoChain) {
    Data=1;
    inputChain=fRecoChain;
    nEntries = inputChain->GetEntries();
    std::cout<<" Tree named "<<recoTreeName<<" found with "<<nEntries<<" events"<<std::endl;
    TObjArray* branches = inputChain->GetListOfBranches();
    std::cout << "Found Tree '" << recoTreeName << "' with " << branches->GetEntries() << " branches and " << nEntries << " entries" << std::endl;

    for(Int_t iBranch = 0; iBranch < branches->GetEntries(); iBranch++){

      TString branchName = ((TBranch*)(*branches)[iBranch])->GetName();
      TClass* branchObjectClass = TClass::GetClass(((TBranch*)(*branches)[iBranch])->GetClassName());
      std::cout << "Found Branch " << branchName.Data() << " containing " << branchObjectClass->GetName() << std::endl;

      if (branchName=="RecoEvent") {
	fRecoEvent = new TRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fRecoEvent);
      } else if (branchName=="Target_Hits") {
	fTargetRecoEvent = new TTargetRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fTargetRecoEvent);
      } else if (branchName=="EVeto_Hits") {
	fEVetoRecoEvent = new TEVetoRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fEVetoRecoEvent);
      } else if (branchName=="PVeto_Hits") {
	fPVetoRecoEvent = new TPVetoRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fPVetoRecoEvent);
      } else if (branchName=="HEPVeto_Hits") {
	fHEPVetoRecoEvent = new THEPVetoRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fHEPVetoRecoEvent);
      } else if (branchName=="ECal_Hits") {
	fECalRecoEvent = new TECalRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fECalRecoEvent);
      } else if (branchName=="SAC_Hits") {
	fSACRecoEvent = new TSACRecoEvent();
	inputChain->SetBranchAddress(branchName.Data(),&fSACRecoEvent);
	//      } else if (branchName=="TPix") {
	//	fTPixRecoEvent = new TTPixRecoEvent();
	//	fRecoChain->SetBranchAddress(branchName.Data(),&fTPixRecoEvent);
      }

    }

  }
  else std::cout << " Tree " << recoTreeName << " not found "<<std::endl;
  //////////////////////////////////////////////////////////////////////////////////////////////

//Plots to make:

  TH1F* hPVetoHitEnergy = new TH1F("PVetoHitEnergy","PVetoHitEnergy",50,0,2.5);
  TH1F* hEVetoHitEnergy = new TH1F("EVetoHitEnergy","EVetoHitEnergy",50,0,2.5);
  TH1F* hUpstreamPVeto = new TH1F("UpstreamPVeto","MostUpstreamPVetoChannel",90,0,90);
  TH1F* hDownstreamPVeto = new TH1F("DownstreamPVeto","MostDownstreamPVetoChannel",90,0,90);
  TH1F* hUpstreamEVeto = new TH1F("UpstreamEVeto","MostUpstreamEVetoChannel",90,0,90);
  TH1F* hDownstreamEVeto = new TH1F("DownstreamEVeto","MostDownstreamEVetoChannel",90,0,90);
  TH1F* hPVetoClusterLength = new TH1F("PVetoClusterLength","PVetoClusterChannelLength",5,0,5);
  TH1F* hEVetoClusterLength = new TH1F("EVetoClusterLength","EVetoClusterChannelLength",5,0,5);
  TH1F* hAvgTimeDiff = new TH1F("AvgTimeDiff","AvgPVetoTime-AvgEVetoTime",200,-10,10);
  TH2F* hPVetoLengthVsUpChannel = new TH2F("PVetoLengthVsUpChannel","PVetoClusterLengthVsUpChannel",90,0,90,5,0,5);
  TH2F* hEVetoLengthVsUpChannel = new TH2F("EVetoLengthVsUpChannel","EVetoClusterLengthVsUpChannel",90,0,90,5,0,5);
  TH2F* hPVetoUpChannelVsEVetoUpChannel = new TH2F("PVetoUpChannelVsEVetoUpChannel","PVetoUpstreamVsEVetoUpstream",90,0,90,90,0,90);
  TH1F* hSailorTDiff = new TH1F("SailorTDiff","AvgTimeDiffForSailors",200,-10,10);
  TH1F* hCowboyTDiff = new TH1F("CowboyTDiff","AvgTimeDiffForCowboys",200,-10,10);
  TH2F* hSailorPVetoUpChannelVsEVetoUpChannel = new TH2F("SailorsPVetoUpChannelVsEVetoUpChannel","PVetoUpstreamVsEVetoUpstreamForSailors",90,0,90,90,0,90);
  TH2F* hCowboyPVetoUpChannelVsEVetoUpChannel = new TH2F("CowboysPVetoUpChannelVsEVetoUpChannel","PVetoUpstreamVsEVetoUpstreamForCowboys",90,0,90,90,0,90);
  
  std::vector<Cluster*> PVetoClusters;
  std::vector<Cluster*> EVetoClusters;

  std::vector<ClusterHits*> PVetoClusterHitVec;
  std::vector<ClusterHits*> EVetoClusterHitVec;

  int oversizedevents=0;
  int maxpvetoclusters=0;
  int maxevetoclusters=0;
     
  Int_t runNEntries = inputChain->GetEntries();
  Int_t ntoread = runNEntries;

  ClusterHits* PVetoClusterHit;
  ClusterHits* EVetoClusterHit;
  
  for(Int_t EventNo=30e3;EventNo<ntoread;EventNo++){
    inputChain->GetEntry(EventNo);
    if(EventNo%1000==0){
      if(Data==0)      std::cout<<"------------------\nMCEventNo "<<EventNo<<" hits "<<fPVetoMCEvent->GetNHits()<<std::endl;
      if(Data==1)      std::cout<<"------------------\nRecoEventNo "<<EventNo<<" hits "<<fPVetoRecoEvent->GetNHits()<<std::endl;
    }
    PVetoClusters.clear();
    EVetoClusters.clear();
    ClusterStructure PVetoClusStruc;//contains a structure for vectors of clusters for each event
    ClusterStructure EVetoClusStruc;//contains a structure for vectors of clusters for each event
    int nhitpass=0;
    if(Data==0){
      for(Int_t ii=0;ii<fPVetoMCEvent->GetNHits();ii++){
	PVetoClusterHit = new ClusterHits();
	PVetoClusterHit->SetEnergy(fPVetoMCEvent->Hit(ii)->GetEnergy());
	PVetoClusterHit->SetTime(fPVetoMCEvent->Hit(ii)->GetTime());
	PVetoClusterHit->SetChannelId(fPVetoMCEvent->Hit(ii)->GetChannelId());
	PVetoClusterHit->SetPosition(fPVetoMCEvent->Hit(ii)->GetPosition());
	PVetoClusterHitVec.push_back(PVetoClusterHit);
      }
    }else if(Data==1){
      for(Int_t jj=0;jj<fPVetoRecoEvent->GetNHits();jj++){
	PVetoClusterHit = new ClusterHits();
	PVetoClusterHit->SetEnergy(fPVetoRecoEvent->Hit(jj)->GetEnergy());
	PVetoClusterHit->SetTime(fPVetoRecoEvent->Hit(jj)->GetTime());
	PVetoClusterHit->SetChannelId(fPVetoRecoEvent->Hit(jj)->GetChannelId());
	PVetoClusterHit->SetPosition(fPVetoRecoEvent->Hit(jj)->GetPosition());
	PVetoClusterHitVec.push_back(PVetoClusterHit);
      }
    }    
    //    continue;
    if(Data==0){
      for(Int_t ii=0;ii<fEVetoMCEvent->GetNHits();ii++){
	EVetoClusterHit = new ClusterHits();
	EVetoClusterHit->SetEnergy(fEVetoMCEvent->Hit(ii)->GetEnergy());
	EVetoClusterHit->SetTime(fEVetoMCEvent->Hit(ii)->GetTime());
	EVetoClusterHit->SetChannelId(fEVetoMCEvent->Hit(ii)->GetChannelId());
	EVetoClusterHit->SetPosition(fEVetoMCEvent->Hit(ii)->GetPosition());
	EVetoClusterHitVec.push_back(EVetoClusterHit);
      }
    }else if(Data==1){
      for(Int_t jj=0;jj<fEVetoRecoEvent->GetNHits();jj++){
	EVetoClusterHit = new ClusterHits();
	EVetoClusterHit->SetEnergy(fEVetoRecoEvent->Hit(jj)->GetEnergy());
	EVetoClusterHit->SetTime(fEVetoRecoEvent->Hit(jj)->GetTime());
	EVetoClusterHit->SetChannelId(fEVetoRecoEvent->Hit(jj)->GetChannelId());
	EVetoClusterHit->SetPosition(fEVetoRecoEvent->Hit(jj)->GetPosition());
	EVetoClusterHitVec.push_back(EVetoClusterHit);
      }
    }    
    for(Int_t iPHit=0;iPHit<PVetoClusterHitVec.size();iPHit++){
      //      std::cout<<" iPHit "<<iPHit<<std::endl;
      hPVetoHitEnergy->Fill(PVetoClusterHitVec[iPHit]->GetEnergy());
      if(PVetoClusterHitVec[iPHit]->GetEnergy()>0.5){
	//	std::cout<<"passed"<<std::endl;
	nhitpass++;
	PVetoClusStruc.AddHit(PVetoClusterHitVec[iPHit],iPHit);
	//std::cout<<"iPHit "<<iPHit<<" Ch "<<fPVetoMCEvent->Hit(iPHit)->GetChannelId()<<" time "<<fPVetoMCEvent->Hit(iPHit)->GetTime()<<std::endl;
	//	std::cout<<"Ch "<<fPVetoMCEvent->Hit(iPHit)->GetChannelId()<<std::endl;
      }
    }
    PVetoClusStruc.HitSort();//sort hits in time
    //    continue;
    PVetoClusStruc.Clusterise();//clusterise hits
    continue;
    //    std::cout<<"Merging PVeto"<<std::endl;
    PVetoClusStruc.MergeClusters();//merge adjacent, in time clusters
    PVetoClusters = PVetoClusStruc.GetClusters();//vector of clusters

    for(int iPClus=0;iPClus<PVetoClusters.size();iPClus++) std::cout<<"Cluster "<<iPClus<<" NHits "<<PVetoClusters[iPClus]->GetNHits()<<" Up "<<PVetoClusters[iPClus]->GetMostUpstreamChannel()<<" Down "<<PVetoClusters[iPClus]->GetMostDownstreamChannel()<<" avgT "<<PVetoClusters[iPClus]->GetAverageTime()<<std::endl;
    //    continue;
    for(Int_t iEHit=0;iEHit<EVetoClusterHitVec.size();iEHit++){
      hEVetoHitEnergy->Fill(EVetoClusterHitVec[iEHit]->GetEnergy());
      if(EVetoClusterHitVec[iEHit]->GetEnergy()>0.5) EVetoClusStruc.AddHit(EVetoClusterHitVec[iEHit],iEHit);
    }
    
    EVetoClusStruc.HitSort();
    EVetoClusStruc.Clusterise();
    //    std::cout<<"**\nMerging EVeto"<<std::endl;
    EVetoClusStruc.MergeClusters();
    EVetoClusters = EVetoClusStruc.GetClusters();
    
    //if(EventNo%1000==0)
    //      std::cout<<"Event "<<EventNo<<" PVeto Clusters "<<PVetoClusters.size()<<" EVeto Clusters "<<EVetoClusters.size()<<std::endl;
    if(PVetoClusters.size()>2||EVetoClusters.size()>2) oversizedevents++;

    if(PVetoClusters.size()>maxpvetoclusters) maxpvetoclusters=PVetoClusters.size();
    if(EVetoClusters.size()>maxevetoclusters) maxevetoclusters=EVetoClusters.size();
    
    for(Int_t iPClus=0;iPClus<PVetoClusters.size();iPClus++){
      //      std::cout<<EventNo<<" "<<iPClus<<" "<<PVetoClusters[iPClus]->GetNHits()<<std::endl;
      hUpstreamPVeto->Fill(PVetoClusters[iPClus]->GetMostUpstreamChannel());
      hDownstreamPVeto->Fill(PVetoClusters[iPClus]->GetMostDownstreamChannel());
      hPVetoClusterLength->Fill(PVetoClusters[iPClus]->GetMostDownstreamChannel()-PVetoClusters[iPClus]->GetMostUpstreamChannel());
      hPVetoLengthVsUpChannel->Fill(PVetoClusters[iPClus]->GetMostUpstreamChannel(),PVetoClusters[iPClus]->GetMostDownstreamChannel()-PVetoClusters[iPClus]->GetMostUpstreamChannel());
      for(Int_t iEClus=0;iEClus<EVetoClusters.size();iEClus++){
	hAvgTimeDiff->Fill(PVetoClusters[iPClus]->GetAverageTime()-EVetoClusters[iEClus]->GetAverageTime());
	if(EVetoClusters[iEClus]->GetMostUpstreamChannel()<(-1*PVetoClusters[iPClus]->GetMostUpstreamChannel()+120)){
	  hSailorTDiff->Fill(PVetoClusters[iPClus]->GetAverageTime()-EVetoClusters[iEClus]->GetAverageTime());
	  hSailorPVetoUpChannelVsEVetoUpChannel->Fill(PVetoClusters[iPClus]->GetMostUpstreamChannel(),EVetoClusters[iEClus]->GetMostUpstreamChannel());
	}
	else{
	  hCowboyTDiff->Fill(PVetoClusters[iPClus]->GetAverageTime()-EVetoClusters[iEClus]->GetAverageTime());
	  hCowboyPVetoUpChannelVsEVetoUpChannel->Fill(PVetoClusters[iPClus]->GetMostUpstreamChannel(),EVetoClusters[iEClus]->GetMostUpstreamChannel());
	}
	hPVetoUpChannelVsEVetoUpChannel->Fill(PVetoClusters[iPClus]->GetMostUpstreamChannel(),EVetoClusters[iEClus]->GetMostUpstreamChannel());
	
      }
    }

    for(Int_t iEClus=0;iEClus<EVetoClusters.size();iEClus++){
      //      std::cout<<EventNo<<" "<<iEClus<<" "<<EVetoClusters[iEClus]->GetNHits()<<std::endl;
      hUpstreamEVeto->Fill(EVetoClusters[iEClus]->GetMostUpstreamChannel());
      hDownstreamEVeto->Fill(EVetoClusters[iEClus]->GetMostDownstreamChannel());
      hEVetoClusterLength->Fill(EVetoClusters[iEClus]->GetMostDownstreamChannel()-EVetoClusters[iEClus]->GetMostUpstreamChannel());
      hEVetoLengthVsUpChannel->Fill(EVetoClusters[iEClus]->GetMostUpstreamChannel(),EVetoClusters[iEClus]->GetMostDownstreamChannel()-EVetoClusters[iEClus]->GetMostUpstreamChannel());
    }
    //    std::cout<<"nhitpass "<<nhitpass<<std::endl;

   }//end event loop

   std::cout<<"oversizedevents "<<oversizedevents<<" maxpvetoclusters "<<maxpvetoclusters<<" maxevetoclusters "<<maxevetoclusters<<std::endl;
   
   hPVetoHitEnergy->Write();
   hEVetoHitEnergy->Write();
   hUpstreamPVeto->Write();
   hDownstreamPVeto->Write();
   hUpstreamEVeto->Write();
   hDownstreamEVeto->Write();
   hPVetoClusterLength->Write();
   hEVetoClusterLength->Write();
   hAvgTimeDiff->Write();
   hPVetoLengthVsUpChannel->Write();
   hEVetoLengthVsUpChannel->Write();
   hPVetoUpChannelVsEVetoUpChannel->Write();
   hSailorTDiff->Write();
   hSailorPVetoUpChannelVsEVetoUpChannel->Write();
   hCowboyTDiff->Write();
   hCowboyPVetoUpChannelVsEVetoUpChannel->Write();
   exit(0);
   
}

