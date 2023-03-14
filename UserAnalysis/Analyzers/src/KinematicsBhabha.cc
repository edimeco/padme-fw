// Written by M. Raggi   20/07/2021 
// Last modified by M. Raggi 12/08/2021
#include "KinematicsBhabha.hh"
#include "TMath.h"
#include "VetoEndPoint.hh"
#include "TLorentzVector.h"
#include "PadmeVRecoConfig.hh"

KinematicsBhabha::KinematicsBhabha(TString cfgFile, Int_t verbose)
{
  //  Neve=0;
  fVerbose = verbose;
  if (fVerbose) {
    printf("---> Creating KinematicsBhabha\n");
    printf("     Configuration file %s\n",cfgFile.Data());
    if (fVerbose>1) printf("     Verbose level %d\n",fVerbose);
  }
  fHS = HistoSvc::GetInstance();
  fCfgParser = new utl::ConfigParser((const std::string)cfgFile.Data());
  fConfig = new PadmeVRecoConfig(fCfgParser,"PadmeReconstructionConfiguration");

  fSwimBhabha = fConfig->GetParOrDefault("KinematicsBhabha","SwimBhabha",1.);
  fVetoEndPoint = VetoEndPoint::GetInstance();

}

KinematicsBhabha::~KinematicsBhabha(){
  delete fCfgParser;
  //  delete fRndm;
}

Bool_t KinematicsBhabha::Init(PadmeAnalysisEvent* event){
  if (fVerbose) printf("---> Initializing KinematicsBhabha\n");
  fEvent = event;
  InitHistos();
  return true;
}

Bool_t KinematicsBhabha::InitHistos(){
  // KinematicsBhabha directory will contain all histograms related to this analysis
  fHS->CreateList("KinematicsBhabha");
  fHS->CreateList("KinematicsBhabha/ThetaAngles");
  fHS->CreateList("KinematicsBhabha/CosTheta");
  fHS->CreateList("KinematicsBhabha/PhiAngles");
  
  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hPosThetaAll",175,0,3.5);
  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hPosThetaGoodChas",175,0,3.5);
  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hPosThetaGoodChaSum",175,0,3.5);

  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hEleThetaAll",175,0,3.5);
  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hEleThetaGoodChas",175,0,3.5);
  fHS->BookHistoList("KinematicsBhabha/ThetaAngles","hEleThetaGoodChaSum",175,0,3.5);

  fHS->BookHistoList("KinematicsBhabha/CosTheta","hPosCosThetaAll",100,-1,1);
  fHS->BookHistoList("KinematicsBhabha/CosTheta","hPosCosThetaGoodChas",100,-1,1);
  fHS->BookHistoList("KinematicsBhabha/CosTheta","hPosCosThetaGoodChaSum",100,-1,1);

  fHS->BookHistoList("KinematicsBhabha/CosTheta","hEleCosThetaAll",100,-1,1);
  fHS->BookHistoList("KinematicsBhabha/CosTheta","hEleCosThetaGoodChas",100,-1,1);
  fHS->BookHistoList("KinematicsBhabha/CosTheta","hEleCosThetaGoodChaSum",100,-1,1);

  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hPosPhiAll",700,-3.5,3.5);
  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hPosPhiGoodChas",700,-3.5,3.5);
  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hPosPhiGoodChaSum",700,-3.5,3.5);
  
  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hElePhiAll",700,-3.5,3.5);
  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hElePhiGoodChas",700,-3.5,3.5);
  fHS->BookHistoList("KinematicsBhabha/PhiAngles","hElePhiGoodChaSum",700,-3.5,3.5);

  fHS->BookHistoList("KinematicsBhabha","hPosXMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hPosYMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hPosZMom",450,0,450);
 
  fHS->BookHistoList("KinematicsBhabha","hEleXMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hEleYMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hEleZMom",450,0,450);

  fHS->BookHistoList("KinematicsBhabha","hSumXMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hSumYMom",100,-5,5);
  fHS->BookHistoList("KinematicsBhabha","hSumZMom",450,0,450);
 
  fHS->BookHisto2List("KinematicsBhabha/ThetaAngles","hPosThetaAllChasVsPVetoCha",90,0,90,200,0,2);
  fHS->BookHisto2List("KinematicsBhabha/CosTheta","hPosCosThetaAllChasVsPVetoCha",90,0,90,100,-1,1);
  fHS->BookHisto2List("KinematicsBhabha/PhiAngles","hPosPhiAllChasVsPVetoCha",90,0,90,700,-3.5,3.5);
  
  fHS->BookHisto2List("KinematicsBhabha/ThetaAngles","hEleThetaAllChasVsEVetoCha",96,0,96,200,0,2);
  fHS->BookHisto2List("KinematicsBhabha/CosTheta","hEleCosThetaAllChasVsPVetoCha",90,0,90,100,-1,1);
  fHS->BookHisto2List("KinematicsBhabha/PhiAngles","hElePhiAllChasVsEVetoCha",96,0,96,700,-3.5,3.5);
  
  /*fHS->BookHisto2List("KinematicsBhabha","hPosThetaVsPosPhiAllChas",200,0,2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hPosThetaVsPosPhiGoodChas",50,0,0.2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hPosThetaVsPosPhiGoodChaSum",50,0,0.2,700,-3.5,3.5);
  
  fHS->BookHisto2List("KinematicsBhabha","hPosCosThetaVsPosPhiAllChas",200,0,2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hPosCosThetaVsPosPhiGoodChas",50,0,0.2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hPosCosThetaVsPosPhiGoodChaSum",50,0,0.2,700,-3.5,3.5);
  
  fHS->BookHisto2List("KinematicsBhabha","hEleCosThetaVsElePhiAllChas",200,0,2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hEleCosThetaVsElePhiGoodChas",50,0,0.2,700,-3.5,3.5);
  fHS->BookHisto2List("KinematicsBhabha","hEleCosThetaVsElePhiGoodChaSum",50,0,0.2,700,-3.5,3.5);*/

  return true;
}

Bool_t KinematicsBhabha::Process(){
  if (fEvent->RecoEvent->GetEventStatusBit(TRECOEVENT_STATUSBIT_SIMULATED)) {
    isMC=true;
    //    std::cout<<"Bhabha input data are simulated "<<std::endl;
  }

  TLorentzVector FourMomentumP; //four momentum of e+ or e- resulting from Bhabha
  TLorentzVector FourMomentumE; //four momentum of e+ or e- resulting from Bhabha

  //motion of CoM in lab frame
  double EBeam;
  double betagamma;
  TVector3 beta;
  double gamma;

  TVector3 momentumP;           //momentum of e+ or e- resulting from Bhabha
  TVector3 momentumE;           //momentum of e+ or e- resulting from Bhabha
  TVector3 momentumTot;

  TVector3 CoMmomentumP;           //momentum of e+ or e- resulting from Bhabha
  TVector3 CoMmomentumE;           //momentum of e+ or e- resulting from Bhabha

  TVector3 directionP;          //unit vector parallel to e+ momentum in lab frame
  TVector3 directionE;          //unit vector parallel to e- momentum in lab frame

  TVector3 orthomomentumP;      //orthognal part of momentum of e+ or e- resulting from Bhabha
  TVector3 orthomomentumE;      //orthognal part of momentum of e+ or e- resulting from Bhabha

  TVector3 beamXfromPADMECoord;       //X axis of beam, wrt which to calculate phi, transformed from PADME coordinate
  TVector3 unitbeamXfromPADMECoord;   //unit vector X axis of beam

  TVector3 beamYfromPADMECoord;       //Y axis of beam, wrt which to calculate phi, transformed from PADME coordinate
  TVector3 unitbeamYfromPADMECoord;   //unit vector Y axis of beam

  Double_t TotDotP;             //dot product of e+ momentum and total momentum from Bhabha 
  Double_t TotDotE;             //dot product of e- momentum and total momentum from Bhabha 

  Double_t energyP;             //energy of e+ or e- resulting from Bhabha
  Double_t energyE;             //energy of e+ or e- resulting from Bhabha

  Int_t chargeP;
  Int_t chargeE;
  
  Double_t PosTheta;
  Double_t PosCosTheta;
  Double_t PosPhi;
  Double_t EleTheta;
  Double_t EleCosTheta;
  Double_t ElePhi;

  Double_t m_e = 0.510999;
  Double_t sqrts;

  TMCParticle* mcOutPart;
  TMCParticle* mcInPart;

  TVector3 momentumPIn;
  Double_t energyPIn;

  TVector3 VertexPos;
  Double_t VertexTime=-1000;

  Int_t PVetoEndCh=-1000;
  Int_t EVetoEndCh=-1000;

  if(isMC&&fSwimBhabha){//&&fEvent->MCTruthEvent->GetEventNumber()<50){
    for(Int_t iV = 0; iV<fEvent->MCTruthEvent->GetNVertices(); iV++) {
      mcVtx = fEvent->MCTruthEvent->Vertex(iV);
      
      if(mcVtx->GetProcess() == "eIoni"||mcVtx->GetProcess() == "Bhabha"){
	VertexPos=mcVtx->GetPosition();
	VertexTime = mcVtx->GetTime();
	for(Int_t iIn = 0; iIn<mcVtx->GetNParticleIn(); iIn++) {
	  mcInPart = mcVtx->ParticleOut(iIn);
	  if(mcInPart->GetPDGCode()==-11){
	    momentumPIn = mcInPart->GetMomentum();
	    energyPIn = mcInPart->GetEnergy();
	    // fHS->FillHistoList("KinematicsBhabha","hInPosE",energyPIn);
	    // fHS->FillHistoList("KinematicsBhabha","hInPosTheta",momentumPIn.Theta());
	    // fHS->FillHistoList("KinematicsBhabha","hInPosPhi",momentumPIn.Phi());
	  }
	}
	//get final state particles from vertex
	for(Int_t iOut = 0; iOut<mcVtx->GetNParticleOut(); iOut++) {
	  mcOutPart = mcVtx->ParticleOut(iOut);
	  
	  //PDGCode: matter = +ve, antimatter = -ve
	  //Electrons
	  if(mcOutPart->GetPDGCode()==11){
	    momentumE = mcOutPart->GetMomentum();
	    directionE = momentumE*(1./momentumE.Mag());
	    energyE = mcOutPart->GetEnergy();
	    FourMomentumE = TLorentzVector(momentumE,energyE);
	    chargeE = -1;
	    fVetoEndPoint->ParticleSwim(FourMomentumE,VertexPos,VertexTime,chargeE);
	    EVetoEndCh = fVetoEndPoint->GetEndFinger();  
	  }
	  //Positrons
	  else if(mcOutPart->GetPDGCode()==-11){
	    momentumP = mcOutPart->GetMomentum();
	    directionP = momentumP*(1./momentumP.Mag());
	    energyP = mcOutPart->GetEnergy();
	    FourMomentumP = TLorentzVector(momentumP,energyP);
	    chargeP = 1;
	    fVetoEndPoint->ParticleSwim(FourMomentumP,VertexPos,VertexTime,chargeP); 
	    PVetoEndCh = fVetoEndPoint->GetEndFinger();   
	  }
	  else{
	    if(mcVtx->GetProcess() == "eIoni"||mcVtx->GetProcess() == "Bhabha")
	      std::cout<<"weirdly, PDGCode = "<<mcOutPart->GetPDGCode()<<std::endl;
	  }
	}//end out particles

	momentumTot = momentumP+momentumE;
	EBeam = (FourMomentumE+FourMomentumP).E();

	//	sqrts = TMath::Sqrt(2*m_e*m_e+2*m_e*momentumTot.Mag());

	beta = momentumTot*(1./EBeam);

	FourMomentumP.Boost(-1*beta);
	CoMmomentumP.SetXYZ(FourMomentumP.X(),FourMomentumP.Y(),FourMomentumP.Z());

	FourMomentumE.Boost(-1*beta);
	CoMmomentumE.SetXYZ(FourMomentumE.X(),FourMomentumE.Y(),FourMomentumE.Z());

	TotDotP = momentumTot.Dot(CoMmomentumP);
	TotDotE = momentumTot.Dot(CoMmomentumE);

	PosCosTheta = TotDotP/(momentumTot.Mag()*CoMmomentumP.Mag());
	EleCosTheta = TotDotE/(momentumTot.Mag()*CoMmomentumE.Mag());

	PosTheta = TMath::ACos(PosCosTheta);
	EleTheta = TMath::ACos(EleCosTheta);
		
	//components of momentum orthogonal to beam 
	orthomomentumP = CoMmomentumP-CoMmomentumP*(TotDotP/CoMmomentumP.Mag2()); //Mag2 gives magnitude squared
	orthomomentumE = CoMmomentumE-CoMmomentumE*(TotDotE/CoMmomentumE.Mag2()); //Mag2 gives magnitude squared
	
	//transform to coordinate system centred on beam axis, not on z
	beamYfromPADMECoord = TVector3(-1.*momentumTot.Y(),momentumTot.X(),0)*(1./sqrt(momentumTot.Y()*momentumTot.Y()+momentumTot.X()*momentumTot.X()));
	unitbeamYfromPADMECoord = beamYfromPADMECoord*(1./beamYfromPADMECoord.Mag());
	
	beamXfromPADMECoord = beamYfromPADMECoord.Cross(momentumTot)*(1./momentumTot.Mag());
	unitbeamXfromPADMECoord = beamXfromPADMECoord*(1./beamXfromPADMECoord.Mag());
	
	PosPhi = TMath::ATan2(orthomomentumP.Dot(unitbeamYfromPADMECoord),orthomomentumP.Dot(unitbeamXfromPADMECoord));
	ElePhi = TMath::ATan2(orthomomentumE.Dot(unitbeamYfromPADMECoord),orthomomentumE.Dot(unitbeamXfromPADMECoord));
	
	// PosTheta = CoMmomentumP.Theta();
	// EleTheta = CoMmomentumE.Theta();

	// PosPhi = CoMmomentumP.Phi();
	// ElePhi = CoMmomentumE.Phi();

	fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hEleThetaAll",EleTheta);
	fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hPosThetaAll",PosTheta);

	fHS->FillHistoList("KinematicsBhabha/CosTheta","hEleCosThetaAll",EleTheta);
	fHS->FillHistoList("KinematicsBhabha/CosTheta","hPosCosThetaAll",PosTheta);

	fHS->FillHistoList("KinematicsBhabha/PhiAngles","hPosPhiAll",PosPhi);
	fHS->FillHistoList("KinematicsBhabha/PhiAngles","hElePhiAll",ElePhi);

	fHS->FillHisto2List("KinematicsBhabha/ThetaAngles","hPosThetaAllChasVsPVetoCha",PVetoEndCh,PosTheta);
	fHS->FillHisto2List("KinematicsBhabha/CosTheta","hPosCosThetaAllChasVsPVetoCha",PVetoEndCh,PosTheta);
	fHS->FillHisto2List("KinematicsBhabha/PhiAngles","hPosPhiAllChasVsPVetoCha",PVetoEndCh,PosPhi);

	fHS->FillHisto2List("KinematicsBhabha/ThetaAngles","hEleThetaAllChasVsEVetoCha",EVetoEndCh,EleTheta);
	fHS->FillHisto2List("KinematicsBhabha/CosTheta","hEleCosThetaAllChasVsPVetoCha",EVetoEndCh,EleTheta);
	fHS->FillHisto2List("KinematicsBhabha/PhiAngles","hElePhiAllChasVsEVetoCha",EVetoEndCh,ElePhi);

	// fHS->FillHisto2List("KinematicsBhabha","hPosThetaVsPosPhiAllChas",PosTheta,PosPhi);
	// fHS->FillHisto2List("KinematicsBhabha","hEleThetaVsElePhiAllChas",EleTheta,ElePhi);	
	
	//if both particles end up in good channels
	if(PVetoEndCh>29&&PVetoEndCh<71&&EVetoEndCh>29&&EVetoEndCh<71){
	  fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hPosThetaGoodChas",PosTheta);
	  fHS->FillHistoList("KinematicsBhabha/CosTheta","hPosCosThetaGoodChas",PosCosTheta);
	  fHS->FillHistoList("KinematicsBhabha/PhiAngles","hPosPhiGoodChas",PosPhi);
	  //	  fHS->FillHisto2List("KinematicsBhabha","hPosThetaVsPosPhiGoodChas",PosTheta,PosPhi);
	  
	  fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hEleThetaGoodChas",EleTheta);
	  fHS->FillHistoList("KinematicsBhabha/CosTheta","hEleCosThetaGoodChas",EleCosTheta);
	  fHS->FillHistoList("KinematicsBhabha/PhiAngles","hElePhiGoodChas",ElePhi);
	  //	  fHS->FillHisto2List("KinematicsBhabha","hEleThetaVsElePhiGoodChas",EleTheta,ElePhi);

	  if(PVetoEndCh+EVetoEndCh>=95){
	    fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hPosThetaGoodChaSum",PosTheta);
	    fHS->FillHistoList("KinematicsBhabha/CosTheta","hPosCosThetaGoodChaSum",PosCosTheta);
	    fHS->FillHistoList("KinematicsBhabha/PhiAngles","hPosPhiGoodChaSum",PosPhi);
	    //	    fHS->FillHisto2List("KinematicsBhabha","hPosThetaVsPosPhiGoodChaSum",PosTheta,PosPhi);
	  
	    fHS->FillHistoList("KinematicsBhabha/ThetaAngles","hEleThetaGoodChaSum",EleTheta);
	    fHS->FillHistoList("KinematicsBhabha/CosTheta","hEleCosThetaGoodChaSum",EleCosTheta);
	    fHS->FillHistoList("KinematicsBhabha/PhiAngles","hElePhiGoodChaSum",ElePhi);
	    //	    fHS->FillHisto2List("KinematicsBhabha","hEleThetaVsElePhiGoodChaSum",EleTheta,ElePhi);	    
	  }
	}//end in acceptance
      }//end if(mcVtx->GetProcess() == "eIoni"||mcVtx->GetProcess() == "Bhabha"){
    }//end for vertices
  }//end if MC
}

Bool_t KinematicsBhabha::Finalize()
{
  if (fVerbose) printf("---> Finalizing BhabhaAnalysis\n");
  return true;
}

