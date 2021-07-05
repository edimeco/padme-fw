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

#include "TTargetMCEvent.hh"
#include "TEVetoMCEvent.hh"
#include "TPVetoMCEvent.hh"
#include "THEPVetoMCEvent.hh"
#include "TECalMCEvent.hh"
#include "TSACMCEvent.hh"
#include "TTPixMCEvent.hh"
#include <algorithm>
#include "TMCVHit.hh"

#include "Clustering.hh"



int main(int argc, char* argv[])
{  

  int c;
  int verbose = 0;
  int nevents = 0;

  TString inputFileName;
  TObjArray inputFileNameList;
  struct stat filestat;

  // Parse options
  while ((c = getopt (argc, argv, "i:l:n:v:h")) != -1) {
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

  // Create chain of input files
  //fprintf(stdout,"=== === === Chain of input files === === ===\n");
  TChain* fMCChain = new TChain("MC");
  for (Int_t iFile = 0; iFile < inputFileNameList.GetEntries(); iFile++) {
    // fprintf(stdout,"%4d %s\n",iFile,((TObjString*)inputFileNameList.At(iFile))->GetString().Data());
    fMCChain->AddFile(((TObjString*)inputFileNameList.At(iFile))->GetString());
  }
  if (fMCChain->GetEntries() == 0) {
    perror(Form("ERROR No events found for tree 'MCEvents' in input chain"));
    exit(1);
  }

  
  TMCEvent* fMCEvent;
  TTargetMCEvent* fTargetMCEvent;
  TEVetoMCEvent* fEVetoMCEvent;
  TPVetoMCEvent* fPVetoMCEvent;
  THEPVetoMCEvent* fHEPVetoMCEvent;
  TECalMCEvent* fECalMCEvent;
  TSACMCEvent* fSACMCEvent;
  TTPixMCEvent* fTPixMCEvent;

  TObjArray* branches = fMCChain->GetListOfBranches();
  for(Int_t iBranch = 0; iBranch < branches->GetEntries(); iBranch++){
    TString branchName = ((TBranch*)(*branches)[iBranch])->GetName();
    if (branchName=="Event") {
      fMCEvent = new TMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fMCEvent);
    } else if (branchName=="Target") {
      fTargetMCEvent = new TTargetMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fTargetMCEvent);
    } else if (branchName=="EVeto") {
      fEVetoMCEvent = new TEVetoMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fEVetoMCEvent);
    } else if (branchName=="PVeto") {
      fPVetoMCEvent = new TPVetoMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fPVetoMCEvent);
    } else if (branchName=="HEPVeto") {
      fHEPVetoMCEvent = new THEPVetoMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fHEPVetoMCEvent);
    } else if (branchName=="ECal") {
      fECalMCEvent = new TECalMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fECalMCEvent);
    } else if (branchName=="SAC") {
      fSACMCEvent = new TSACMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fSACMCEvent);
    } else if (branchName=="TPix") {
      fTPixMCEvent = new TTPixMCEvent();
      fMCChain->SetBranchAddress(branchName.Data(),&fTPixMCEvent);
    }
  }

  // Get some info about the input chain
  Int_t runNEntries = fMCChain->GetEntries();
  //std::cout << "Found Tree 'MCEvents' with " << runNEntries << " entries" << std::endl;
  //for(Int_t i=0; i < fMCChain->GetListOfBranches()->GetEntries(); i++) {
  //  std::cout << "Branch " << i << " is " << fMCChain->GetListOfBranches()->At(i)->GetName() << std::endl;
  //}
  //TRawEvent* rawEv = new TRawEvent();
  //fMCChain->SetBranchAddress("RawEvent",&rawEv);
 
  // Set number of events to read
  Int_t ntoread = runNEntries;
  if (nevents && nevents<runNEntries) {
    ntoread = nevents;
    //printf("Reading first %d events\n",ntoread);
  }

  //////////////////////////////////////////////////////////////////////////////////////////////

  TFile* OutputFile = new TFile("OutputPlots.root","RECREATE");

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
  
  std::vector<Cluster*> PVetoClusters;
  std::vector<Cluster*> EVetoClusters;
  int oversizedevents=0;
  int maxpvetoclusters=0;
  int maxevetoclusters=0;
    
   for(Int_t EventNo=0;EventNo<ntoread;EventNo++){
    fMCChain->GetEntry(EventNo);
    std::cout<<"EventNo "<<EventNo<<std::endl;
    PVetoClusters.clear();
    EVetoClusters.clear();
    ClusterStructure PVetoClusStruc;//contains a structure for vectors of clusters for each event
    ClusterStructure EVetoClusStruc;//contains a structure for vectors of clusters for each event
    int nhitpass=0;
    for(Int_t iPHit=0;iPHit<fPVetoMCEvent->GetNHits();iPHit++){
      hPVetoHitEnergy->Fill(fPVetoMCEvent->Hit(iPHit)->GetEnergy());
      if(fPVetoMCEvent->Hit(iPHit)->GetEnergy()>0.5){
	nhitpass++;
	PVetoClusStruc.AddHit(fPVetoMCEvent->Hit(iPHit),iPHit);
	//	std::cout<<"Ch "<<fPVetoMCEvent->Hit(iPHit)->GetChannelId()<<std::endl;
      }
    }
 
    PVetoClusStruc.HitSort();//sort hits in time
    PVetoClusStruc.Clusterise();//clusterise hits
    //Problema sta in MergeClusters()
    PVetoClusStruc.MergeClusters();//merge adjacent, in time clusters
    PVetoClusters = PVetoClusStruc.GetClusters();//vector of clusters
    
    for(Int_t iEHit=0;iEHit<fEVetoMCEvent->GetNHits();iEHit++){
      hEVetoHitEnergy->Fill(fEVetoMCEvent->Hit(iEHit)->GetEnergy());
      if(fEVetoMCEvent->Hit(iEHit)->GetEnergy()>0.5) EVetoClusStruc.AddHit(fEVetoMCEvent->Hit(iEHit),iEHit);
    }
    
    EVetoClusStruc.HitSort();
    EVetoClusStruc.Clusterise();
    EVetoClusStruc.MergeClusters();
    EVetoClusters = EVetoClusStruc.GetClusters();
    
    //if(EventNo%1000==0)
      std::cout<<"Event "<<EventNo<<" PVeto Clusters "<<PVetoClusters.size()<<" EVeto Clusters "<<EVetoClusters.size()<<std::endl;
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
   exit(0);
   
}

