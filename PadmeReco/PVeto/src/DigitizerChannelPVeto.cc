#include "DigitizerChannelPVeto.hh"
#include "TObject.h"
#include "TVector3.h"
#include "TSpectrum.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TTree.h"

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>

void DigitizerChannelPVeto::PrintConfig(){
  //  std::cout << "Signal width: " << fSignalWidth << " samples" << std::endl;
  //  std::cout << "fUseAbsSignals: " << fUseAbsSignals << std::endl;  
}

DigitizerChannelPVeto::~DigitizerChannelPVeto(){
  if(fGlobalMode->IsPedestalMode() || fGlobalMode->GetGlobalDebugMode() || fSaveAnalog){
     if(fBethorMauro==1) SaveDebugHistosMauro();
     else               SaveDebugHistosBeth();
  }
}

void DigitizerChannelPVeto::Init(GlobalRecoConfigOptions *gMode, PadmeVRecoConfig *cfg){
  detectorname=cfg->GetName();
  fGlobalMode = gMode;
  fAnalogsPrinted=0;
  std::string name;
  name = cfg->GetParOrDefault("DETECTOR","NAME","DIGI");
  name += "DIGI";

  hSig    = new TH1D("hSig","hSig",fIMax,0.,fIMax);

  fTimeBin        = cfg->GetParOrDefault("ADC","TimeBin",1.);
  fPedMaxNSamples = cfg->GetParOrDefault("RECO","NumberOfSamplesPedestal",100);  

  fAmpThresholdLow = cfg->GetParOrDefault("RECO","AmplThrLow",10.);
  fAmpThresholdHigh= cfg->GetParOrDefault("RECO","AmplThrHigh",20.);

  fSaveAnalog = cfg->GetParOrDefault("Output","Analog",0); //M. Raggi: 03/03/2021  
  fTotalAnalogs = cfg->GetParOrDefault("Output","TotalAnalogs",0); //Beth 23/2/22: total number of analog signals to write to PVetoRecoAn.root

  fChannelEqualisation = cfg->GetParOrDefault("RECO","ChannelEqualisation",1);

  fUsePulseProcessing  = cfg->GetParOrDefault("RECO","UsePulseProcessing",1);
  fDerivPoints         = cfg->GetParOrDefault("RECO","DerivPoints",15);
  fPeakSearchWidth     = cfg->GetParOrDefault("RECO","PeakSearchWidth",5);
  fZeroSuppression     = cfg->GetParOrDefault("RECO","ZeroSuppression",5.);
  fDerivAmpToEnergy    = cfg->GetParOrDefault("RECO","DerivAmpToEnergy",0.046);
  fmVtoMeV             = cfg->GetParOrDefault("RECO","ConversionFactor",29.52);
  fBethorMauro         = cfg->GetParOrDefault("RECO","BethorMauro",0);
  std::cout << cfg->GetName() << "*******************************" <<  std::endl; 
  std::cout << cfg->GetName() << "*******************************" <<  std::endl;
  std::cout << cfg->GetName() << "***I'M THE NEW PVETO DIGITIZER*" <<  std::endl;
  std::cout << cfg->GetName() << "*******************************" <<  std::endl;  
  std::cout << cfg->GetName() << "*******************************" <<  std::endl;  
  PrintConfig();

  if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
    if(fBethorMauro==1) PrepareDebugHistosMauro(); //debugging mode histos
    else               PrepareDebugHistosBeth(); //debugging mode histos
  }

}

Double_t DigitizerChannelPVeto::CalcPedestal() {  //get NSamples from data card
  fPed = 0.;
  fNPedSamples = 0;
  // average of first fPedMaxNSamples
  for(Short_t i = 0 ; i  <   fPedMaxNSamples; i++) {
    fNPedSamples ++;
    fPed+= fSamples[i];
  }
  //std::cout << " fNPedSamples " << fNPedSamples <<" fPed Veto"<< std::endl;
  fPed /= fNPedSamples;//fNPedSamples is probably useless, just use fPedMaxSamples
  //std::cout <<  fPed << " fNPedSamples " << fNPedSamples << std::endl;
  return fPed;
}

Double_t DigitizerChannelPVeto::ZSupHit(Float_t Thr, UShort_t NAvg) {// NAvg = 1000, Thr by default is fZeroSuppression = 5. This parameter isn't in the config file, probably it should be?
  fRMS1000 = TMath::RMS(NAvg,&fSamples[0]);
  if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
    hNoiseRMSAvg->AddBinContent(GetChID()+1,fRMS1000);
    hNoiseRMSPerChannel[GetChID()]->Fill(fRMS1000);
    hNZSupEvents->AddBinContent(GetChID()+1,1);
  }

  Double_t ZSupHit=-1;
  // come back to a veto setup 
  if(fRMS1000>Thr){
    ZSupHit=0;
  }else{
    ZSupHit=1;   // signal to be removed
  }
  
// Beth add the thr for noisy channels
  return ZSupHit;
}

Double_t DigitizerChannelPVeto::CalcChaTimeMauro(std::vector<TRecoVHit *> &hitVec,UShort_t iMax, UShort_t ped) {
  Int_t npeaks = 20;
  static TSpectrum SpectrumProcessor(npeaks);// = new TSpectrum(20);
  double Time   = 0.;
  fCharge = 0.;
  //  Int_t ChID=GetElChID(); // MR TODAY MISSING CH ID //Beth 18/2/22: commented as not used
  //  std::cout<<"CH ID"<<ChID<<std::endl;

  //currently looking for peaks with TSpectrum to obtain multi hit times
  static Double_t AbsSamRec[1024];
  //  static Double_t AbsSamRecDP[1024]; //Beth 18/22/22: not used

  //Check if they are needed or not and the real dimension
  Double_t  dxdt[1024];

  //  Double_t  SigCounts[1024]; //Beth 18/22/22: not used
  //Double_t  SigmV[1024]; //Beth 18/22/22: not used

  for(UShort_t s=0;s<=iMax;s++){
    AbsSamRec[s] = (Double_t) (fEqualSamples[s]);
    //std::cout<< s << "     "  << fSamples[s]  <<" V "<< AbsSamRec[s]  <<std::endl;
  }
  for(UShort_t s=iMax+1;s<1024;s++){
    AbsSamRec[s] = 0;
  }

  hSig->SetContent(AbsSamRec);
  
  // check this number hard coded!!!!
  Int_t iDer=15;
  // compute derivative
  for(Int_t ll=0;ll<iMax;ll++){
    if(ll>=iDer){ 
      if(AbsSamRec[ll]-AbsSamRec[ll-iDer] >0) dxdt[ll]=(AbsSamRec[ll]-AbsSamRec[ll-iDer]);
      if(AbsSamRec[ll]-AbsSamRec[ll-iDer] <0) dxdt[ll]=0.;
    }else{
      dxdt[ll]=0;
    }
    htmp->SetBinContent(ll,dxdt[ll]);
    //    std::cout<<ll<<" sam "<<Temp[ll]<<" "<<Temp1[ll]<<" "<<dxdt[ll]<<std::endl;
  }

  Double_t dmax = htmp->GetMaximum();  //Max of the derivative
  Double_t VMax = hSig->GetMaximum();

  //  std::cout<<"Get Maximum     "<<VMax<<"   Get dxdt    "<<dmax<<std::endl;
  //if (VMax>fAmpThresholdHigh) std::cout<<VMax<<" VMax "<<std::endl;
  std::vector<Float_t> xp;
  std::vector<Float_t> yp;

  std::vector<Float_t> x1p;
  std::vector<Float_t> y1p;

  //****************************************************
  //*  start searching for the paeaks in the RAW signals
  //****************************************************

  if(VMax>fAmpThresholdHigh){
    TSpectrum *s1 = &SpectrumProcessor;// new TSpectrum(2*npeaks);  //npeaks max number of peaks
    Double_t peak_thr  = fAmpThresholdLow/VMax; //minimum peak height allowed.
    Int_t nfound = s1->Search(hSig,fPeakSearchWidth,"",peak_thr);     
    if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog) hNhitSig ->Fill(nfound);
    for(Int_t ll=0;ll<nfound;ll++){ //peak loop per channel
      Double_t t1xp = (s1->GetPositionX())[ll];
      Double_t t1yp = (s1->GetPositionY())[ll];
      x1p.push_back(t1xp);
      y1p.push_back(t1yp);
      if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog) hSigV->Fill(t1yp);
    }
    

  //****************************************************
  //*  start searching for paeaks in the DERIVATIVE 
  //****************************************************
    //    Double_t MinDxDt=15.;  //be careful depends on how big is iDer //Beth 18/2/22: commented because currently not used

    Double_t PeakSearchDerWdt = 5.; //from
    TSpectrum *s = &SpectrumProcessor;
    Double_t Der_peak_thr  = 0.2;
    //    Double_t source[895];
    Float_t source[986];
    for(Int_t i = 0; i <986; i++) source[i]=htmp->GetBinContent(i+1);
    //    s->SmoothMarkov(source,985,2);
    for (Int_t i=0; i<986; i++) htmp->SetBinContent(i+1,source[i]);



    Int_t nfoundd = s->Search(htmp,PeakSearchDerWdt,"",Der_peak_thr);
    if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
      hNhitDx   ->Fill(nfoundd);
      hDeltaN   ->Fill(nfoundd-nfound);
      hDeltaNvsN->Fill(nfoundd,nfoundd-nfound);
    }
    for(Int_t ll=0;ll<nfoundd;ll++){ //peak loop per channel
      fCharge = 0.;
      Double_t txp = (s->GetPositionX())[ll];
      Double_t typ = (s->GetPositionY())[ll];
      xp.push_back(txp);
      yp.push_back(typ);
      if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog) hDxV->Fill(typ);

      Time=xp[ll]*fTimeBin;    // this is too naive can we do better?
      if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
	if(nfound==nfoundd) hDeltaT->Fill(xp[ll]-x1p[ll]);
	if(nfound==nfoundd) hDeltaV->Fill(yp[ll]-y1p[ll]);
      }
      //      std::cout<<"xp "<<xp[ll]<<" xp1 "<<x1p[ll]<<std::endl;
      //      Int_t bin = hSig->GetXaxis()->FindBin(xp);
      //      fAmpli = FindY(bin);
      fAmpli = hSig->GetBinContent(xp[ll]);  // sbagliato perche' c'e' l'offset con derivata
      //      fEnergy = fAmpli/fmVtoMeV;       // sbagliato perche' puo dipendere 
      fEnergy = yp[ll]/27.;       // sbagliato perche' puo dipendere dal canale
      //      if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
      //      hMHVMax->Fill(VMax);
      //      hMHdxdtMax->Fill(yp[ll]);
      //      hVmaxvsDxdtMax->Fill(VMax,dmax);
      //      hVmaxOvDxdt->Fill(VMax/dmax);
      //      hMHEnergy->Fill(yp[ll]/27.);
      // }
      TRecoVHit *Hit = new TRecoVHit();  
      Hit->SetTime(Time);
      //Hit->SetEnergy(fAmpli);
      Hit->SetEnergy(fEnergy);
      hitVec.push_back(Hit);
      if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog){
	hVMax->Fill(VMax);
	hdxdtMax->Fill(dmax);
	hVmaxvsDxdtMax->Fill(VMax,dmax);
	hVMOvDxdtvsNHits->Fill(nfound,VMax/dmax); //il canale??
	hVmaxOvDxdt->Fill(VMax/dmax);
	hEnergy->Fill(dmax/20.); // what is this number??
      }
      
    }
    //    hSig->Write();
    //htmp->Write();
//    if( (nfoundd-nfound)>0 ){
//      hSig->Write();
//      htmp->Write();
//    }
    //    delete s;
  }
  return Time;
}

Double_t DigitizerChannelPVeto::CalcChaTimeBeth(std::vector<TRecoVHit *> &hitVec){//copied and modified from 211019-padmefw, Beth 18/2/22. 22/2/22 Made it independent of pedestal and iMax

  RawGetMax=0;
  DerivGetMax=0;
  
  int nfound=0;
  Int_t npeaks = 20;//maximum possible number of peaks
  static TSpectrum SpectrumProcessor(npeaks);
  double Time   = 0.;
  
  //currently looking for peaks with TSpectrum to obtain multi hit times
  //Int_t npeaks =25;
  static Double_t AbsSamRec[1024];
  static Double_t AbsSamRecDeriv[1024];
  static Double_t SamRecDeriv[1024];//not set to 0

  for(UShort_t s=0;s<fIMax;s++){
    AbsSamRec[s] = (Double_t) (fEqualSamples[s]);
    if(s>=fDerivPoints)    SamRecDeriv[s]=(AbsSamRec[s]-AbsSamRec[s-fDerivPoints]);
    else if(s<fDerivPoints) SamRecDeriv[s]=0;
    if(SamRecDeriv[s]<=0) AbsSamRecDeriv[s] = 0;//to prevent TSpectrum being confused by negative values, set negative values of the derivative to 0
    else if (SamRecDeriv[s]>0)
      AbsSamRecDeriv[s] = SamRecDeriv[s];
  }

  HTSpec->Reset();
  
  HTSpec->SetContent(AbsSamRecDeriv);

  Double_t VMax = HTSpec->GetMaximum();//check: should the reconstruction amp threshold be dependent on deriv amp or raw amp?
  //  Double_t VMin = HTSpec->GetMinimum(); //Beth 17/2/22: Not used

  TSpectrum *spec;
  TRecoVHit *Hit;
 
  Double_t xp;
  Double_t yp;
  Double_t rawAmp;

  //fampthresholds, fpeakthr should all go in the config file and then not be touched. fderivthr should probably be deleted and become fpeakthr

  /*if(GetChID()==58||GetChID()==60){//this goes in ZSup
    fAmpThresholdHigh=12;
    fAmpThresholdLow=12;
    }*/
  //  else{
    fAmpThresholdHigh=8;
    fAmpThresholdLow=8;
    //}

  if(VMax>fAmpThresholdHigh){
    spec = &SpectrumProcessor;
    Double_t peak_thr;
  
    double derivthr = fAmpThresholdLow/2.11;//2.11 is the ratio of amplitudes between raw signal and derivative. Check this, becuase it's a function of the number of samples used to take the derivative
    peak_thr = derivthr/VMax;   
    nfound = spec->Search(HTSpec,fPeakSearchWidth,"",peak_thr);     
    fAnalogPrint=0;
    for(Int_t ll=0;ll<nfound;ll++){ //peak loop per channel
      //      fCharge = 0.;
      xp = (spec->GetPositionX())[ll];
      yp = (spec->GetPositionY())[ll];
      rawAmp = AbsSamRec[int(xp+0.5)];
      vRawHitVec.push_back(rawAmp);
      vTSpecYPHitVec.push_back(yp);
      Time=xp*fTimeBin;
      tDerivHitVec.push_back(Time);
      if(nfound == 1){
	RawRise = rawAmp;
	RawGetMax=*std::max_element(AbsSamRec, AbsSamRec + fIMax);
	DerivGetMax=*std::max_element(AbsSamRecDeriv, AbsSamRecDeriv + fIMax);
      }
      if(Time<50||Time>350) fAnalogPrint=1;
    }//end of nfound loop
  }//closes if(VMax>thr)

  fNFoundPerChannel[GetChID()]=nfound;

  std::vector<int> index(nfound, 0);

  for (UShort_t i = 0 ; i != index.size() ; i++) {
    index[i] = i;
  }
  
  sort(index.begin(), index.end(),
       [&](const int& a, const int& b) {
	 return (tDerivHitVec[a] < tDerivHitVec[b]);
       }
       );

  if((index.size()!=nfound)){
    std::cout<<"CONFUSION!"<<std::endl;
    return -100;
  }
  
  //  Double_t hitV;//includes tail correction
  double tailcorrection=0;
  double DeltaTSortSamples=0;
 
  for (UShort_t ii = 0 ; ii != index.size() ; ii++) {
    tDerivSortHitVec.push_back(tDerivHitVec[index[ii]]);
    if(ii>0&&tDerivSortHitVec[ii]<tDerivSortHitVec[ii-1]){ //check that the time sorting has worked correctly
      std::cout<<"SORTING ISN'T WORKING"<<std::endl;
      return -100;
    }

    vRawSortHitVec.push_back(vRawHitVec[index[ii]]);
    vTSpecYPSortHitVec.push_back(vTSpecYPHitVec[index[ii]]);

    DeltaTSortSamples=tDerivSortHitVec[ii]/fTimeBin-tDerivSortHitVec[ii-1]/fTimeBin;
    //    tailcorrection = TailHeight(DeltaTSortSamples);
    tailcorrection = TailHeightDerivative(DeltaTSortSamples);

    /*    if(ii==0) vRawCorrectHitVec.push_back(vRawSortHitVec[ii]);//for the first hit, there's no tail correction because there's not tail from a previous hit
	  else    vRawCorrectHitVec.push_back(vRawSortHitVec[ii]-vRawCorrectHitVec[ii-1]*tailcorrection); //for all hits after the first, apply the tail correction*/

    if(ii==0)    vTSpecYPCorrectHitVec.push_back(vTSpecYPSortHitVec[index[ii]]);
    else    vTSpecYPCorrectHitVec.push_back(vTSpecYPSortHitVec[ii]-vTSpecYPCorrectHitVec[ii-1]*tailcorrection); //for all hits after the first, apply the tail correction

    //    fAmpli=vTSpecYPCorrectHitVec[ii];
    
    Hit = new TRecoVHit();  
    Hit->SetTime(tDerivSortHitVec[ii]);//commented because of sorting problem
    //    fEnergy=fAmpli*fDerivAmpToEnergy;
    fEnergy=vTSpecYPCorrectHitVec[ii]*fDerivAmpToEnergy;
    //    std::cout<<ii<<" "<<fAmpli<<" "<<vRawCorrectHitVec[ii]<<std::endl;
    Hit->SetEnergy(fEnergy);
    hitVec.push_back(Hit);
    //    std::cout<<"hitVec.size() "<<hitVec.size()<<std::endl;

  }//end loop over hits

  if(fGlobalMode->GetGlobalDebugMode() || fGlobalMode->IsPedestalMode() || fSaveAnalog)  HitPlots(hitVec);
  
  return Time;
}

void DigitizerChannelPVeto::PrepareDebugHistosBeth(){ //Beth 20/10/21 copied from 190917padme-fw, in turn copied from Mauro's DigitizerChannelECal structure to create a set of debug histograms that are produced only in debug mode
  TString fileoutname;

  if(detectorname=="EVeto") fileoutname="EVetoRecoAnDeriv.root";
  else if(detectorname=="PVeto") fileoutname="PVetoRecoAnDeriv.root";

  fileOut    = new TFile(fileoutname, "RECREATE");
  
  hNoEventsReconstructed     = new TH1F("NoEventsReconstructed","NoEventsReconstructed",4,0,2);//number of hits reconstructed by TSpectrum on derivatives    
  hOccupancy                 = new TH1F("hOccupancy","hOccupancyAvg",90,0,90);
  hOccupancyOneHit           = new TH1F("hOccupancyOneHit","hOccupancyOneHitAvg",90,0,90);
  hNoHitsDeriv               = new TH1F("NoHitsDeriv","NoHitsDeriv",20,0,20);//number of hits reconstructed by TSpectrum on derivatives    
  hRawV                      = new TH1F("RawV","RawV",400,0,400);
  hRawVCorrect               = new TH1F("RawVCorrect","RawVCorrect",400,0,400);
  hRawVCorrectChannels20to70 = new TH1F("RawVCorrectChannels20to70","RawVCorrectChannels20to70",400,0,400);
  hRawVOneHit                = new TH1F("RawVOneHit","RawVOneHit",400,0,400);
  hRawVMultiHit              = new TH1F("RawVMultiHit","RawVMultiHit",400,0,400);
  hRawVMultiHitCorrect       = new TH1F("RawVMultiHitCorrect","RawVMultiHitCorrect",400,0,400);
  hDerivV                    = new TH1F("DerivV","DerivV",400,0,400);
  hDerivVOneHit              = new TH1F("DerivVOneHit","DerivVOneHit",100,0,200);
  hDerivVCorrect             = new TH1F("DerivVCorrect","DerivVCorrect",400,0,400);
  hHitTime                   = new TH1F("HitTime","HitTime",400,0,800);
  hHitEnergy                 = new TH1F("HitEnergy","HitEnergy",100,0,10);
  hHitEnergySingleHit        = new TH1F("HitEnergySingleHit","HitEnergySingleHit",100,0,10);
  hMinTimeDiffDeriv          = new TH1F("MinTimeDiffDeriv","MinTimeDiffDeriv",100,0,100);
  hVRatio                    = new TH1F("VRatio","VRatio",50,0,5);  
  hNZSupEvents               = new TH1F("hNZSupEvents","hNZSupEvents",96,0,96);
  hNoiseRMSAvg               = new TH1F("hNoiseRMSAvg","hNoiseRMSAvg",96,0,96);
  hYMaxRawYTSpecRatio        = new TH1F("hYMaxRawYTSpecRatio","hYMaxRawYTSpecRatio",50,0,2);
  hYRiseYTSpecRatio          = new TH1F("hYRiseYTSpecRatio","hYRiseYTSpecRatio",50,0,2);
  hYMaxDerivYTSpecRatio      = new TH1F("hYMaxDerivYTSpecRatio","hYMaxDerivYTSpecRatio",50,0,2);
  //  hYTSpecYMaxDiff            = new TH1F("hYTSpecYMaxDiff","HYTSpecYMaxDiff",100,-50,50);

  hAmpDiffVsUncorrectAmp                    = new TH2F("hAmpDiffVsUncorrectAmp","hAmpDiffVsUncorrectAmp",100,0,400,300,-100,200);
  hAmpDiffVsUncorrectAmpChannels20to70      = new TH2F("hAmpDiffVsUncorrectAmpChannels20to70","hAmpDiffVsUncorrectAmpChannels20to70",100,0,400,300,-100,200);

  hCorrectedAmpVsUncorrectAmp               = new TH2F("hCorrectedAmpVsUncorrectAmp","hCorrectedAmpVsUncorrectAmp",50,0,100,50,0,100);
  hCorrectedAmpVsUncorrectAmpChannels20to70 = new TH2F("hCorrectedAmpVsUncorrectAmpChannels20to70","hCorrectedAmpVsUncorrectAmpChannels20to70",50,0,100,50,0,100);

  hAmpDiffVsUncorrectAmpNotFirstHit         = new TH2F("hAmpDiffVsUncorrectAmpNotFirstHit","hAmpDiffVsUncorrectAmpNotFirstHit",100,0,400,300,-100,200);
  hCorrectedAmpVsUncorrectAmpNotFirstHit    = new TH2F("hCorrectedAmpVsUncorrectAmpNotFirstHit","hCorrectedAmpVsUncorrectAmpNotFirstHit",50,0,100,50,0,100);

  hYMaxRawYTSpecRatioVsYMax                    = new TH2F("hYMaxRawYTSpecRatioVsYMax","hYMaxRawYTSpecRatioVsYMax",100,0,100,50,0,2);
  // hYMaxVsYTSpecAllHits = new TH2F("hYMaxVsYTSpecAllHits","hYMaxVsYTSpecAllHits",100,0,100,100,0,100);
  // hYMaxVsYTSpecSingleHits = new TH2F("hYMaxVsYTSpecSingleHits","hYMaxVsYTSpecSingleHits",100,0,100,100,0,100);

  gUnAbsSigs = new TGraph(fNSamples);
  
  for(int ii=0;ii<96;ii++){
    sprintf(name, "NoHitsDerivChannelPerEvent%d",ii);
    hNoHitsDerivPerChannel[ii] = new TH1F(name,name,20,0,20);

    sprintf(name, "RawVChannel%d",ii);
    hRawVPerChannel[ii] = new TH1F(name,name,400,0,400);
    
    sprintf(name, "RawVOneHitChannel%d",ii);
    hRawVOneHitPerChannel[ii] = new TH1F(name,name,400,0,400);

    sprintf(name, "RawVCorrectChannel%d",ii);
    hRawVCorrectPerChannel[ii] = new TH1F(name,name,400,0,400);
    
    sprintf(name, "DerivVChannel%d",ii);
    hDerivVPerChannel[ii] = new TH1F(name,name,400,0,400);

    sprintf(name, "DerivVOneHitChannel%d",ii);
    hDerivVOneHitPerChannel[ii] = new TH1F(name,name,400,0,400);

    sprintf(name, "DerivVCorrectChannel%d",ii);
    hDerivVCorrectPerChannel[ii] = new TH1F(name,name,400,0,400);

    sprintf(name,"hNoiseRMSPerChannel%d",ii);
    hNoiseRMSPerChannel[ii] = new TH1F(name,name,20,0,10);

  }
}

 void DigitizerChannelPVeto::PrepareDebugHistosMauro(){
//   fileOut    = new TFile("PVetoAn.root", "RECREATE");

//   hListPVeto = new TList();  

//   hListPVeto->Add(hNhitDx      = new TH1F("hNhitDx","hNhitDx",25,-0.5,24.5));
//   hListPVeto->Add(hNhitSig     = new TH1F("hNhitSig","hNhitSig",25,-0.5,24.5));

//   hListPVeto->Add(hDeltaN     = new TH1F("hDeltaN","hDeltaN",25,-12.5,12.5));
//   hListPVeto->Add(hDeltaNvsN= new TH2F("hDeltaNvsN","hDeltaNvsN",25,-0.5,24.5,25,-12.5,12.5)); 

//   hListPVeto->Add(hDeltaT     = new TH1F("hDeltaT","hDeltaT",150,-24.5,24.5));
//   hListPVeto->Add(hDeltaV     = new TH1F("hDeltaV","hDeltaV",250,-125,125));
//   //Single HIT
//   hListPVeto->Add(hdxdtMax      = new TH1F("hdxdtMax","hdxdtMax",1000,0.,1000.));
//   hListPVeto->Add(hVMax         = new TH1F("hVMax","hVMax",1000,0.,1000.));
//   hListPVeto->Add(hVmaxvsDxdtMax= new TH2F("hVmaxvsDxdtMax","hVmaxvsDxdtMax",1000,0.,1000.,1000,0.,1000.));
//   hListPVeto->Add(hVMOvDxdtvsNHits = new TH2F("hVMOvDxdtvsNHits","hVMOvDxdtvsNHits",20,-0.5,19.5,250,0.,5.)); 
//   hListPVeto->Add(hVmaxOvDxdt   = new TH1F("hVmaxOvDxdt","hVmaxOvDxdt",500,0.,5.));
//   hListPVeto->Add(hEnergy       = new TH1F("hEnergy","hEnergy",500,0.,5.));

//   //Derivative hits
//   hListPVeto->Add(hDxV = new TH1F("hDxV","hDxV",1000,0.,1000.));
//   hListPVeto->Add(hDxE = new TH1F("hDxE","hDxE",500,0.,10.));

//   //Signal hits
//   hListPVeto->Add(hSigV = new TH1F("hSigV","hSigV",1000,0.,1000.));
//   hListPVeto->Add(hSigE = new TH1F("hSigE","hSigE",500,0.,10.));

//   //Time distributions of hit times
//   hListPVeto->Add(hTHits = new TH1F("hTHits","hTHits",400,-300.,500.));
  
//   //MULTI HIT
// //  hListPVeto->Add(hMHdxdtMax      = new TH1F("hMHdxdtMax","hMHdxdtMax",1000,0.,1000.));
// //  hListPVeto->Add(hMHVMax         = new TH1F("hMHVMax","hMHVMax",1000,0.,1000.));
// //  hListPVeto->Add(hMHVmaxvsDxdtMax= new TH2F("hMHVmaxvsDxdtMax","hMHVmaxvsDxdtMax",1000,0.,1000.,1000,0.,1000.)); 
// //  hListPVeto->Add(hMHVmaxOvDxdt   = new TH1F("hMHVmaxOvDxdt","hMHVmaxOvDxdt",500,0.,5.));
// //  hListPVeto->Add(hMHEnergy       = new TH1F("hMHEnergy","hMHEnergy",500,0.,5.));

// //Beth's histos to prevent seg faults, not written to file
//   hNoiseRMSAvg               = new TH1F("hNoiseRMSAvg","hNoiseRMSAvg",96,0,96);
//   hNZSupEvents               = new TH1F("hNZSupEvents","hNZSupEvents",96,0,96);
//   for(int ii=0;ii<96;ii++){
//     sprintf(name, "RawVChannel%d",ii);
//     hRawVPerChannel[ii] = new TH1F(name,name,400,0,400);
    
//     sprintf(name, "RawVOneHitChannel%d",ii);
//     hRawVOneHitPerChannel[ii] = new TH1F(name,name,400,0,400);

//     sprintf(name, "RawVCorrectChannel%d",ii);
//     hRawVCorrectPerChannel[ii] = new TH1F(name,name,400,0,400);
    
//     sprintf(name,"hNoiseRMSPerChannel%d",ii);
//     hNoiseRMSPerChannel[ii] = new TH1F(name,name,20,0,10);
//   }
}

void DigitizerChannelPVeto::SaveDebugHistosMauro(){
  fileOut->cd();
  std::cout<<"fSaveAnalog "<<fSaveAnalog<<std::endl;
  if(fGlobalMode->IsPedestalMode() || fGlobalMode->GetGlobalDebugMode() || fSaveAnalog)
    hListPVeto->Write(); //use it in monitor mode only  
  fileOut->Close();
}

void DigitizerChannelPVeto::SaveDebugHistosBeth(){
  if(fileOut!=0) std::cout<<"not zero "<<fileOut<<std::endl;
  fileOut->cd();
  
  std::cout<<fGlobalMode->IsPedestalMode()<<" "<<fGlobalMode->GetGlobalDebugMode()<<" "<<fSaveAnalog<<std::endl;

  if(fGlobalMode->IsPedestalMode() || fGlobalMode->GetGlobalDebugMode() || fSaveAnalog){
    hNoEventsReconstructed->Write();
    hOccupancy->Scale(1./hNoEventsReconstructed->GetEntries());
    hOccupancy->Write();
    hOccupancyOneHit->Scale(1./hNoEventsReconstructed->GetEntries());
    hOccupancyOneHit->Write();
    hNoHitsDeriv->Write();
    hRawV->Write();
    hDerivV->Write();
    hHitEnergy->Write();
    hHitEnergySingleHit->Write();
    hHitTime->Write();

    hRawVCorrect->Write();
    hRawVCorrectChannels20to70->Write();
    hRawVOneHit->Write();
    hDerivVOneHit->Write();
    hDerivVCorrect->Write();
    hAmpDiffVsUncorrectAmp->Write();
    hAmpDiffVsUncorrectAmpChannels20to70->Write();
    hAmpDiffVsUncorrectAmpNotFirstHit->Write();

    hCorrectedAmpVsUncorrectAmp->Write();
    hCorrectedAmpVsUncorrectAmpChannels20to70->Write();
    hCorrectedAmpVsUncorrectAmpNotFirstHit->Write();

    hNZSupEvents->Write();
    hNoiseRMSAvg->Divide(hNZSupEvents);
    hNoiseRMSAvg->Write();

    hYMaxRawYTSpecRatio->Write();
    hYRiseYTSpecRatio->Write();
    hYMaxDerivYTSpecRatio->Write();
    hYMaxRawYTSpecRatioVsYMax->Write();

    // hYTSpecYMaxDiff->Write();
    // hYMaxVsYTSpecAllHits->Write();
    // hYMaxVsYTSpecSingleHits->Write();

    fileOut->mkdir("NoHitDerivChannel");  
    fileOut->cd("NoHitDerivChannel");  
    for(int ii=0;ii<96;ii++) hNoHitsDerivPerChannel[ii]->Write();

    fileOut->mkdir("RawVOneHitChannel");  
    fileOut->cd("RawVOneHitChannel");  
    for(int ii=0;ii<96;ii++) hRawVOneHitPerChannel[ii]->Write();

    fileOut->mkdir("RawVChannel");  
    fileOut->cd("RawVChannel");  
    for(int ii=0;ii<96;ii++) hRawVPerChannel[ii]->Write();

    fileOut->mkdir("RawVCorrectChannel");  
    fileOut->cd("RawVCorrectChannel");  
    for(int ii=0;ii<96;ii++) hRawVCorrectPerChannel[ii]->Write();

    fileOut->mkdir("DerivVOneHitChannel");  
    fileOut->cd("DerivVOneHitChannel");  
    for(int ii=0;ii<96;ii++) hDerivVOneHitPerChannel[ii]->Write();

    fileOut->mkdir("DerivVCorrectChannel");  
    fileOut->cd("DerivVCorrectChannel");  
    for(int ii=0;ii<96;ii++) hDerivVCorrectPerChannel[ii]->Write();

    fileOut->mkdir("DerivVChannel");  
    fileOut->cd("DerivVChannel");  
    for(int ii=0;ii<96;ii++) hDerivVPerChannel[ii]->Write();

    fileOut->mkdir("Waveforms");
    fileOut->cd("Waveforms");
    for(UInt_t ii = 0;ii<hRaw.size();ii++)
      hRaw[ii]->Write();  
  
    fileOut->mkdir("DerivativeTSpec");
    fileOut->cd("DerivativeTSpec");
    for(UInt_t ii = 0;ii<hDeriv.size();ii++)
      hDeriv[ii]->Write();  

    fileOut->mkdir("NegativeSignals");
    fileOut->cd("NegativeSignals");
    for(UInt_t ii = 0;ii<gUnAbsSigGraphs.size();ii++)
      gUnAbsSigGraphs[ii]->Write();  

    fileOut->mkdir("NoiseRMS");  
    fileOut->cd("NoiseRMS");  
    for(int ii=0;ii<96;ii++)  hNoiseRMSPerChannel[ii]->Write();
    fileOut->Close();
    }
}

void DigitizerChannelPVeto::Reconstruct(std::vector<TRecoVHit *> &hitVec){  //using TSpectrum
  if(GetChID()==0) hNoEventsReconstructed->Fill(1);
  if(GetChID()>89) return;//Beth 10/3/22: There are only 90 PVeto channels but in PVeto.cfg 96 channels are still listed
  if(fUsePulseProcessing==0){ 
    Double_t IsZeroSup = ZSupHit(fZeroSuppression,1000.);  //Beth 10/3/22: we should use the parameter signal window instead of hard-coded 1000. fZeroSuppression is not in PVeto.cfg but defaults to 5.
    if(IsZeroSup) return;
    Double_t ped=CalcPedestal();  // takes the starting sample from data cards
    SetAbsSignals(ped);//Beth 21/2/22: moved from before zero suppression to after. Turns samples into positive mV and performs channel equalisation
    hSig->SetContent(fEqualSamples);

    if(fBethorMauro==1) CalcChaTimeMauro(hitVec,fIMax,ped); //seaching up to fIMax only
    else               CalcChaTimeBeth(hitVec);
  }
  else{
    // Giorgi pulse processing code goes here
  }
  AnalogPlotting(); //plot analog signals

  tDerivHitVec          .clear();
  tDerivSortHitVec      .clear();
  tDerivDiffHitVec      .clear();
  vRawHitVec	        .clear();
  vRawSortHitVec        .clear();
  vRawCorrectHitVec     .clear();
  vTSpecYPHitVec        .clear();
  vTSpecYPSortHitVec    .clear();
  vTSpecYPCorrectHitVec .clear();
}

void DigitizerChannelPVeto::SetAbsSignals(Double_t ped){
  Double_t ScaleFactor=1;
  if(fChannelEqualisation)  ScaleFactor=SetPVetoChaGain();   
  //fNSamples is 1024 but I can't find where it's set
  for(UShort_t i = 0;i<fNSamples;i++){
    fEqualSamples[i] = (ped-fSamples[i])/4096.*1000.; //convert fSamples to positive mV
    fEqualSamples[i] = (fEqualSamples[i])*ScaleFactor;//equalise channel gains
  }
}

void DigitizerChannelPVeto::AnalogPlotting(){
  //plot analog signals
  if(fAnalogsPrinted<fTotalAnalogs){
    if(fAnalogPrint==1){//vRawCorrectHitVec.size()==1&&vTSpecYPHitVec[0]<20){
      hRaw.push_back((TH1F*)hSig->Clone());
      sprintf(name, "hRawEvent%iChannel%d", EventCounter,GetChID());
      hRaw[hRaw.size()-1]->SetNameTitle(name,name);
      
      hDeriv.push_back((TH1F*)HTSpec->Clone());
      sprintf(name, "hDerivEvent%iChannel%d", EventCounter,GetChID());
      hDeriv[hDeriv.size()-1]->SetNameTitle(name,name);
    
      for(int ii = 0; ii<fNSamples;ii++) gUnAbsSigs->SetPoint(ii,ii,fSamples[ii]);
      gUnAbsSigGraphs.push_back(gUnAbsSigs);

      fAnalogsPrinted++;
    }  
  }
}

void DigitizerChannelPVeto::HitPlots(std::vector<TRecoVHit *> &hitVec){
  //  std::cout<<"Event "<<EventCounter<<" Ch "<<GetChID()<<" hitVec.size() "<<hitVec.size()<<" fNFoundPerChannel "<<fNFoundPerChannel[GetChID()]<<std::endl;
  hNoHitsDeriv->Fill(fNFoundPerChannel[GetChID()]);
  hNoHitsDerivPerChannel[GetChID()]->Fill(fNFoundPerChannel[GetChID()]);

  if(vRawCorrectHitVec.size()==1) hOccupancyOneHit->Fill(GetChID());

  //  double hitV=0;
  double AmpDiff=0;
  //  std::cout<<"Tot "<<vRawCorrectHitVec.size()<<std::endl;
  for(UInt_t myiHit=0;myiHit<fNFoundPerChannel[GetChID()];myiHit++){
    hOccupancy->Fill(GetChID());
    hHitTime->Fill(tDerivSortHitVec[myiHit]);

    //uncorrected amplitude
    hDerivV->Fill(vTSpecYPSortHitVec[myiHit]);
    hDerivVPerChannel[GetChID()]->Fill(vTSpecYPSortHitVec[myiHit]);
    hHitEnergy->Fill(vTSpecYPCorrectHitVec[myiHit]*fDerivAmpToEnergy);

    //corrected amplitude
    hDerivVCorrect->Fill(vTSpecYPCorrectHitVec[myiHit]);//hitV);
    hDerivVCorrectPerChannel[GetChID()]->Fill(vTSpecYPCorrectHitVec[myiHit]);
    //    AmpDiff=vTSpecYPSortHitVec[myiHit]-hitV;
    //hAmpDiffVsUncorrectAmp->Fill(vTSpecYPSortHitVec[myiHit],AmpDiff);
    //    hCorrectedAmpVsUncorrectAmp->Fill(vTSpecYPSortHitVec[myiHit],hitV);

    // hYTSpecYMaxDiff->Fill(vTSpecYPSortHitVec[myiHit]-vRawSortHitVec[myiHit]);
    //    hYMaxVsYTSpecAllHits->Fill(vRawSortHitVec[myiHit],vTSpecYPSortHitVec[myiHit]);

    //corrected amplitude for "good" channels
    if(GetChID()>=20 &&GetChID()<=70){
      //      hRawVCorrectChannels20to70->Fill(hitV);
      //      hAmpDiffVsUncorrectAmpChannels20to70->Fill(vRawSortHitVec[myiHit],AmpDiff);
      //hCorrectedAmpVsUncorrectAmpChannels20to70->Fill(vRawSortHitVec[myiHit],hitV);
    }
    //events with exactly one hit
    if(fNFoundPerChannel[GetChID()]==1){
      hRawVOneHit->Fill(RawGetMax);
      hRawVOneHitPerChannel[GetChID()]->Fill(RawGetMax);
      hDerivVOneHit->Fill(vTSpecYPSortHitVec[myiHit]);
      hDerivVOneHitPerChannel[GetChID()]->Fill(vTSpecYPSortHitVec[myiHit]);
      hHitEnergySingleHit->Fill(vTSpecYPSortHitVec[myiHit]*fDerivAmpToEnergy);
      //hYMaxVsYTSpecSingleHits->Fill(vRawSortHitVec[myiHit],vTSpecYPSortHitVec[myiHit]);

      //ratio between derivative peak amplitude as found by TSpectrum and maximum amplitude of raw signal for single hit events
      hYMaxRawYTSpecRatio->Fill(RawGetMax/vTSpecYPSortHitVec[myiHit]);
      hYRiseYTSpecRatio->Fill(RawRise/vTSpecYPSortHitVec[myiHit]);
      hYMaxDerivYTSpecRatio->Fill(DerivGetMax/vTSpecYPSortHitVec[myiHit]);

      hYMaxRawYTSpecRatioVsYMax->Fill(RawGetMax,RawGetMax/vTSpecYPSortHitVec[myiHit]);
    }

    //hits after the first
    if(myiHit>1){
      hAmpDiffVsUncorrectAmpNotFirstHit->Fill(vRawSortHitVec[myiHit],AmpDiff);
      //      hCorrectedAmpVsUncorrectAmpNotFirstHit->Fill(vRawSortHitVec[myiHit],hitV);
    }
  }
  
}

Double_t DigitizerChannelPVeto::SetPVetoChaGain(){
  Double_t ScaleFactor = 1;
  //  std::cout<<"Setting cha gain"<<std::endl;
  if(GetChID()==0) 	 ScaleFactor = 1.20007;
  if(GetChID()==1) 	 ScaleFactor = 1.20517;
  if(GetChID()==2) 	 ScaleFactor = 1.34357;
  if(GetChID()==3) 	 ScaleFactor = 1.30759;
  if(GetChID()==4) 	 ScaleFactor = 1.26678;
  if(GetChID()==5) 	 ScaleFactor = 2.31421;
  if(GetChID()==6) 	 ScaleFactor = 1.1933 ;
  if(GetChID()==7) 	 ScaleFactor = 1.42206;
  if(GetChID()==8) 	 ScaleFactor = 1.36612;
  if(GetChID()==9) 	 ScaleFactor = 1.36997;
  if(GetChID()==10)	 ScaleFactor = 1.52649;
  if(GetChID()==11)	 ScaleFactor = 1.36823;
  if(GetChID()==12)	 ScaleFactor = 1.43113;
  if(GetChID()==13)	 ScaleFactor = 1.36435;
  if(GetChID()==14)	 ScaleFactor = 1.43819;
  if(GetChID()==15)	 ScaleFactor = 1.30947;
  if(GetChID()==16)	 ScaleFactor = 1.05575;
  if(GetChID()==17)	 ScaleFactor = 1.04161;
  if(GetChID()==18)	 ScaleFactor = 0.95391;
  if(GetChID()==19)	 ScaleFactor = 0.90890;
  if(GetChID()==20)	 ScaleFactor = 0.95665;
  if(GetChID()==21)	 ScaleFactor = 0.91797;
  if(GetChID()==22)	 ScaleFactor = 0.918981;
  if(GetChID()==23)      ScaleFactor = 0.884638;
  if(GetChID()==24)	 ScaleFactor = 0.86368;
  if(GetChID()==25)	 ScaleFactor = 0.87166;
  if(GetChID()==26)	 ScaleFactor = 1.13524;
  if(GetChID()==27)	 ScaleFactor = 0.89272;
  if(GetChID()==28)	 ScaleFactor = 1.07496;
  if(GetChID()==29)	 ScaleFactor = 1.17894;
  if(GetChID()==30)	 ScaleFactor = 0.91751;
  if(GetChID()==31)	 ScaleFactor = 0.80332;
  if(GetChID()==32)	 ScaleFactor = 1.17808;
  if(GetChID()==33)	 ScaleFactor = 0.82741;
  if(GetChID()==34)	 ScaleFactor = 0.92725;
  if(GetChID()==35)	 ScaleFactor = 0.91137;
  if(GetChID()==36)	 ScaleFactor = 0.87394;
  if(GetChID()==37)	 ScaleFactor = 1.00689;
  if(GetChID()==38)	 ScaleFactor = 1.08942;
  if(GetChID()==39)	 ScaleFactor = 1.08354;
  if(GetChID()==40)	 ScaleFactor = 1.06285;
  if(GetChID()==41)	 ScaleFactor = 1.03217;
  if(GetChID()==42)	 ScaleFactor = 0.90686;
  if(GetChID()==43)	 ScaleFactor = 0.87261;
  if(GetChID()==44)	 ScaleFactor = 1.05954;
  if(GetChID()==45)	 ScaleFactor = 0.99362;
  if(GetChID()==46)	 ScaleFactor = 1.18334;
  if(GetChID()==47)	 ScaleFactor = 1.14441;
  if(GetChID()==48)	 ScaleFactor = 1.0729 ;
  if(GetChID()==49)	 ScaleFactor = 1.03824;
  if(GetChID()==50)	 ScaleFactor = 1.21671;
  if(GetChID()==51)	 ScaleFactor = 0.84024;
  if(GetChID()==52)	 ScaleFactor = 0.91855;
  if(GetChID()==53)	 ScaleFactor = 1.10666;
  if(GetChID()==54)	 ScaleFactor = 1.03583;
  if(GetChID()==55)	 ScaleFactor = 0.82296;
  if(GetChID()==56)	 ScaleFactor = 0.91125;
  if(GetChID()==57)	 ScaleFactor = 0.96735;
  if(GetChID()==58)	 ScaleFactor = 1.75645;
  if(GetChID()==59)	 ScaleFactor = 1.00093;
  if(GetChID()==60)	 ScaleFactor = 1.39221;
  if(GetChID()==61)	 ScaleFactor = 0.85481;
  if(GetChID()==62)	 ScaleFactor = 1.08227;
  if(GetChID()==63)	 ScaleFactor = 0.96390;
  if(GetChID()==64)	 ScaleFactor = 1.01518;
  if(GetChID()==65)	 ScaleFactor = 0.99412;
  if(GetChID()==66)	 ScaleFactor = 1.07244;
  if(GetChID()==67)	 ScaleFactor = 0.96944;
  if(GetChID()==68)	 ScaleFactor = 1.03648;
  if(GetChID()==69)	 ScaleFactor = 1.03518;
  if(GetChID()==70)	 ScaleFactor = 1.14892;
  if(GetChID()==71)	 ScaleFactor = 1.16687;
  if(GetChID()==72)	 ScaleFactor = 1.01364;
  if(GetChID()==73)	 ScaleFactor = 0.91313;
  if(GetChID()==74)	 ScaleFactor = 1.04913;
  if(GetChID()==75)	 ScaleFactor = 0.92306;
  if(GetChID()==76)	 ScaleFactor = 0.96247;
  if(GetChID()==77)	 ScaleFactor = 0.81604;
  if(GetChID()==78)	 ScaleFactor = 0.93498;
  if(GetChID()==79)	 ScaleFactor = 1.44282;
  if(GetChID()==80)	 ScaleFactor = 1.13769;
  if(GetChID()==81)	 ScaleFactor = 1.50241;
  if(GetChID()==82)	 ScaleFactor = 1.22141;
  if(GetChID()==83)	 ScaleFactor = 1.19654;
  if(GetChID()==84)	 ScaleFactor = 1.19121;
  if(GetChID()==85)	 ScaleFactor = 2.05119;
  if(GetChID()==86)	 ScaleFactor = 1.38753;
  if(GetChID()==87)	 ScaleFactor = 1.16416;
  if(GetChID()==88)	 ScaleFactor = 1.14429;
  if(GetChID()==89)	 ScaleFactor = 1.10903;
  return ScaleFactor;
}

Double_t DigitizerChannelPVeto::TailHeight(Int_t DeltaT){//DeltaT in samples. Returns fraction of maximum signal height that a signal will have at time DeltaT samples after the peak
  Double_t HeightFrac=0;
  Double_t Frac[152];
  Frac[0]= -0.980571   ; 
  Frac[1]= -0.981553   ;
  Frac[2]= -0.980736   ;
  Frac[3]= -0.97778    ;
  Frac[4]= -0.971722   ;
  Frac[5]= -0.964676   ;
  Frac[6]= -0.956346   ;
  Frac[7]= -0.947524   ;
  Frac[8]= -0.936815   ;
  Frac[9]= -0.925396   ;
  Frac[10]=-0.913626   ;
  Frac[11]=-0.902825   ;
  Frac[12]=-0.889087   ;
  Frac[13]=-0.875895   ;
  Frac[14]=-0.862712   ;
  Frac[15]=-0.8499     ;
  Frac[16]=-0.83571    ;
  Frac[17]=-0.820816   ;
  Frac[18]=-0.807027   ;
  Frac[19]=-0.793632   ;
  Frac[20]=-0.778125   ;
  Frac[21]=-0.763295   ;
  Frac[22]=-0.749179   ;
  Frac[23]=-0.736244   ;
  Frac[24]=-0.72276    ;
  Frac[25]=-0.70841    ;
  Frac[26]=-0.695512   ;
  Frac[27]=-0.683497   ;
  Frac[28]=-0.671098   ;
  Frac[29]=-0.658029   ;
  Frac[30]=-0.645816   ;
  Frac[31]=-0.635959   ;
  Frac[32]=-0.625102   ;
  Frac[33]=-0.614064   ;
  Frac[34]=-0.603985   ;
  Frac[35]=-0.594267   ;
  Frac[36]=-0.584422   ;
  Frac[37]=-0.573764   ;
  Frac[38]=-0.564061   ;
  Frac[39]=-0.55551    ;
  Frac[40]=-0.546319   ;
  Frac[41]=-0.535383   ;
  Frac[42]=-0.526042   ;
  Frac[43]=-0.517341   ;
  Frac[44]=-0.507091   ;
  Frac[45]=-0.49763    ;
  Frac[46]=-0.488771   ;
  Frac[47]=-0.480027   ;
  Frac[48]=-0.470536   ;
  Frac[49]=-0.460673   ;
  Frac[50]=-0.452545   ;
  Frac[51]=-0.443358   ;
  Frac[52]= -0.434426  ;
  Frac[53]= -0.425019  ;
  Frac[54]= -0.416526  ;
  Frac[55]= -0.40837   ;
  Frac[56]= -0.398906  ;
  Frac[57]= -0.388772  ;
  Frac[58]= -0.381358  ;
  Frac[59]= -0.372678  ;
  Frac[60]= -0.363733  ;
  Frac[61]= -0.354299  ;
  Frac[62]= -0.345341  ;
  Frac[63]= -0.337202  ;
  Frac[64]= -0.32762   ;
  Frac[65]= -0.317911  ;
  Frac[66]= -0.309973  ;
  Frac[67]= -0.302964  ;
  Frac[68]= -0.294315  ;
  Frac[69]= -0.285437  ;
  Frac[70]= -0.278386  ;
  Frac[71]= -0.271311  ;
  Frac[72]= -0.264793  ;
  Frac[73]= -0.256882  ;
  Frac[74]= -0.250253  ;
  Frac[75]= -0.244707  ;
  Frac[76]= -0.237993  ;
  Frac[77]= -0.231874  ;
  Frac[78]= -0.226499  ;
  Frac[79]= -0.221193  ;
  Frac[80]= -0.215178  ;
  Frac[81]= -0.209712  ;
  Frac[82]= -0.204143  ;
  Frac[83]= -0.198955  ;
  Frac[84]= -0.194389  ;
  Frac[85]= -0.188292  ;
  Frac[86]= -0.183074  ;
  Frac[87]= -0.178442  ;
  Frac[88]= -0.173292  ;
  Frac[89]= -0.166721  ;
  Frac[90]= -0.162393  ;
  Frac[91]= -0.15742   ;
  Frac[92]= -0.152343  ;
  Frac[93]= -0.147001  ;
  Frac[94]= -0.142241  ;
  Frac[95]= -0.137595  ;
  Frac[96]= -0.133313  ;
  Frac[97]= -0.128435  ;
  Frac[98]= -0.124631  ;
  Frac[99]= -0.121106  ;
  Frac[100]= -0.116854 ;
  Frac[101]= -0.112579 ;
  Frac[102]= -0.109927 ;
  Frac[103]= -0.107044 ;
  Frac[104]= -0.103422 ;
  Frac[105]= -0.0997127;
  Frac[106]= -0.0967979;
  Frac[107]= -0.094159 ;
  Frac[108]= -0.0911629;
  Frac[109]= -0.0879251;
  Frac[110]= -0.0850502 ;
  Frac[111]= -0.0825226;
  Frac[112]= -0.079926 ;
  Frac[113]= -0.0767524;
  Frac[114]= -0.0745703;
  Frac[115]= -0.0722342;
  Frac[116]= -0.0694902;
  Frac[117]= -0.0661824;
  Frac[118]= -0.0644465;
  Frac[119]= -0.0624207;
  Frac[120]= -0.0603031;
  Frac[121]= -0.0573222;
  Frac[122]= -0.0552809;
  Frac[123]= -0.0533513;
  Frac[124]= -0.0513799;
  Frac[125]= -0.0488961;
  Frac[126]= -0.0473815;
  Frac[127]= -0.0456104;
  Frac[128]= -0.0440546;
  Frac[129]= -0.0414905;
  Frac[130]= -0.0400789;
  Frac[131]= -0.0383941;
  Frac[132]= -0.0363219;
  Frac[133]= -0.0342883;
  Frac[134]= -0.0328866;
  Frac[135]= -0.0312922;
  Frac[136]= -0.029854 ;
  Frac[137]= -0.0274867;
  Frac[138]= -0.0255809;
  Frac[139]= -0.0241595;
  Frac[140]= -0.0217186;
  Frac[141]= -0.0202501;
  Frac[142]= -0.0183742;
  Frac[143]= -0.0168845;
  Frac[144]= -0.0142908;
  Frac[145]= -0.0126114;
  Frac[146]= -0.0110446;
  Frac[147]= -0.0093668;
  Frac[148]= -0.0068487;
  Frac[149]= -0.0049751;
  Frac[150]= -0.0037547;
  Frac[151]= -0.0024154;
  Frac[152]= -0.0003822;

  if(DeltaT<153)  HeightFrac = -1*Frac[DeltaT];//The fractions of heights in this method are for a negative signal, in the rest of the reco positive signals are used, so signal height fraction needs to be multiplied by -1.
  else HeightFrac=0;
  return HeightFrac;

}

Double_t DigitizerChannelPVeto::TailHeightDerivative(Int_t DeltaT){//DeltaT in samples. Returns fraction of maximum signal height that a signal will have at time DeltaT samples after the peak
  Double_t HeightFrac=0;
  Double_t Frac[152];
  Frac[0]   =  1; 
  Frac[1]   =  0.997254     ;
  Frac[2]   =  0.980119     ;
  Frac[3]   =  0.964593     ;
  Frac[4]   =  0.941908     ;
  Frac[5]   =  0.866346     ;
  Frac[6]   =  0.786271     ;
  Frac[7]   =  0.744847     ;
  Frac[8]   =  0.681817     ;
  Frac[9]   =  0.605922     ;
  Frac[10]  =  0.52764      ;
  Frac[11]  =  0.45165      ;
  Frac[12]  =  0.372615     ;
  Frac[13]  =  0.297546     ;
  Frac[14]  =  0.223813     ;
  Frac[15]  =  0.156913     ;
  Frac[16]  =  0.0931688    ;
  Frac[17]  =  0.0337405    ;
  Frac[18]  = -0.0213706    ;
  Frac[19]  = -0.0669654    ;
  Frac[20]  = -0.109943     ;
  Frac[21]  = -0.152183     ;
  Frac[22]  = -0.181495     ;
  Frac[23]  = -0.206573     ;
  Frac[24]  = -0.224541     ;
  Frac[25]  = -0.249835     ;
  Frac[26]  = -0.264263     ;
  Frac[27]  = -0.274342     ;
  Frac[28]  = -0.288237     ;
  Frac[29]  = -0.296333     ;
  Frac[30]  = -0.302538     ;
  Frac[31]  = -0.307553     ;
  Frac[32]  = -0.308717     ;
  Frac[33]  = -0.311267     ;
  Frac[34]  = -0.310586     ;
  Frac[35]  = -0.309731     ;
  Frac[36]  = -0.30875      ;
  Frac[37]  = -0.306781     ;
  Frac[38]  = -0.305272     ;
  Frac[39]  = -0.300329     ;
  Frac[40]  = -0.293151     ;
  Frac[41]  = -0.290462     ;
  Frac[42]  = -0.284854     ;
  Frac[43]  = -0.2765	    ;
  Frac[44]  = -0.270725     ;
  Frac[45]  = -0.264902     ;
  Frac[46]  = -0.260046     ;
  Frac[47]  = -0.254269     ;
  Frac[48]  = -0.249207     ;
  Frac[49]  = -0.248261     ;
  Frac[50]  = -0.243831     ;
  Frac[51]  = -0.235288     ;
  Frac[52]  = -0.234772     ;
  Frac[53]  = -0.229418     ;
  Frac[54]  = -0.229754     ;
  Frac[55]  = -0.224102     ;
  Frac[56]  = -0.224207     ;
  Frac[57]  = -0.225017     ;
  Frac[58]  = -0.22231      ;
  Frac[59]  = -0.221657     ;
  Frac[60]  = -0.222108     ;
  Frac[61]  = -0.219502     ;
  Frac[62]  = -0.219925     ;
  Frac[63]  = -0.217694     ;
  Frac[64]  = -0.215415     ;
  Frac[65]  = -0.215071     ;
  Frac[66]  = -0.214287     ;
  Frac[67]  = -0.209114     ;
  Frac[68]  = -0.20973      ;
  Frac[69]  = -0.209181     ;
  Frac[70]  = -0.208435     ;
  Frac[71]  = -0.203314     ;
  Frac[72]  = -0.201796     ;
  Frac[73]  = -0.201014     ;
  Frac[74]  = -0.198875     ;
  Frac[75]  = -0.193166     ;
  Frac[76]  = -0.191475     ;
  Frac[77]  = -0.18906      ;
  Frac[78]  = -0.187891     ;
  Frac[79]  = -0.181985     ;
  Frac[80]  = -0.179791     ;
  Frac[81]  = -0.175351     ;
  Frac[82]  = -0.175295     ;
  Frac[83]  = -0.170323     ;
  Frac[84]  = -0.167377     ;
  Frac[85]  = -0.161963     ;
  Frac[86]  = -0.160955     ;
  Frac[87]  = -0.153541     ;
  Frac[88]  = -0.150095     ;
  Frac[89]  = -0.145247     ;
  Frac[90]  = -0.142584     ;
  Frac[91]  = -0.139147     ;
  Frac[92]  = -0.138427     ;
  Frac[93]  = -0.133732     ;
  Frac[94]  = -0.132274     ;
  Frac[95]  = -0.124256     ;
  Frac[96]  = -0.126357     ;
  Frac[97]  = -0.122266     ;
  Frac[98]  = -0.121186     ;
  Frac[99]  = -0.117688     ;
  Frac[100] = -0.118717     ;
  Frac[101] = -0.115563     ;
  Frac[102] = -0.114932     ;
  Frac[103] = -0.110287     ;
  Frac[104] = -0.110468     ;
  Frac[105] = -0.110157     ;
  Frac[106] = -0.109457     ;
  Frac[107] = -0.104666     ;
  Frac[108] = -0.103959     ;
  Frac[109] = -0.101067     ;
  Frac[110] = -0.100501     ;
  Frac[111] = -0.0948209    ;
  Frac[112] = -0.0958058    ;
  Frac[113] = -0.0907429    ;
  Frac[114] = -0.089684     ;
  Frac[115] = -0.0848281    ;
  Frac[116] = -0.0835924    ;
  Frac[117] = -0.0827835    ;
  Frac[118] = -0.0837584    ;
  Frac[119] = -0.0792539    ;
  Frac[120] = -0.0766105    ;
  Frac[121] = -0.0717581    ;
  Frac[122] = -0.0702401    ;
  Frac[123] = -0.0697511    ;
  Frac[124] = -0.0690636    ;
  Frac[125] = -0.0695008    ;
  Frac[126] = -0.0668789    ;
  Frac[127] = -0.0606407    ;
  Frac[128] = -0.0609923    ;
  Frac[129] = -0.0611206    ;
  Frac[130] = -0.0628294    ;
  Frac[131] = -0.0592398    ;
  Frac[132] = -0.0566166    ;
  Frac[133] = -0.0545669    ;
  Frac[134] = -0.0523624    ;
  Frac[135] = -0.0509565    ;
  Frac[136] = -0.0531841    ;
  Frac[137] = -0.0513809    ;
  Frac[138] = -0.0483326    ;
  Frac[139] = -0.0459102    ;
  Frac[140] = -0.0454662    ;
  Frac[141] = -0.0467786    ;
  Frac[142] = -0.0479043    ;
  Frac[143] = -0.0452532    ;
  Frac[144] = -0.0421873    ;
  Frac[145] = -0.0399463    ;
  Frac[146] = -0.0420936    ;
  Frac[147] = -0.0393451    ;
  Frac[148] = -0.043388     ;
  Frac[149] = -0.0408676    ;
  Frac[150] = -0.0420922    ;
  Frac[151] = -0.037542     ;
  Frac[152] = -0.0409631    ;
  Frac[153]  =  -0.0391775 ;
  Frac[154]  =  -0.0418197 ;
  Frac[155]  =  -0.0390806 ;  
  Frac[156]  =    -0.0408229 ;
  Frac[157]  =    -0.0381236 ;
  Frac[158]  =    -0.0404099 ;
  Frac[159]  =    -0.0388342 ;
  Frac[160]  =    -0.0418864 ;
  Frac[161]  =    -0.0362089 ;
  Frac[162]  =    -0.0397867 ;
  Frac[163]  =    -0.0341513 ;
  Frac[164]  =    -0.0373749 ;
  Frac[165]  =    -0.0334771 ;
  Frac[166]  =    -0.0361808 ;
  Frac[167]  =    -0.0307574 ;
  Frac[168]  =    -0.034892  ;
  Frac[169]  =    -0.0308241 ;
  Frac[170]  =    -0.0323418 ;
  Frac[171]  =    -0.0277144 ;
  Frac[172]  =    -0.0294595 ;
  Frac[173]  =    -0.0294106 ;
  Frac[174]  =    -0.0305658 ;
  Frac[175]  =    -0.0245063 ;
  Frac[176]  =    -0.029801  ;
  Frac[177]  =    -0.0273992 ;
  Frac[178]  =    -0.0278916 ;
  Frac[179]  =    -0.0278331 ;
  Frac[180]  =    -0.0325746 ;
  Frac[181]  =    -0.0302618 ;
  Frac[182]  =    -0.0326252 ;
  Frac[183]  =    -0.0287132 ;
  Frac[184]  =    -0.0303228 ;
  Frac[185]  =    -0.0280644 ;
  Frac[186]  =    -0.0317901 ;
  Frac[187]  =    -0.0298637 ;
  Frac[188]  =    -0.0304833 ;
  Frac[189]  =    -0.0262541 ;
  Frac[190]  =    -0.0300384 ;
  Frac[191]  =    -0.0278753 ;
  Frac[192]  =    -0.0277963 ;
  Frac[193]  =    -0.0274074 ;
  Frac[194]  =    -0.0247811 ;
  Frac[195]  =    -0.0221116 ;
  Frac[196]  =    -0.0229999 ;
  Frac[197]  =    -0.0198742 ;
  Frac[198]  =    -0.020908  ;
  Frac[199]  =    -0.0194822 ;
  Frac[200]  =    -0.0205471 ;
  Frac[201]  =    -0.015884  ;
  Frac[202]  =    -0.0185419 ;
  Frac[203]  =    -0.0142732 ;
  Frac[204]  =    -0.0185146 ;
  Frac[205]  =    -0.0142009 ;
  Frac[206]  =    -0.0135252 ;
  Frac[207]  =    -0.0106508 ;
  Frac[208]  =    -0.00973468;
  Frac[209]  =    -0.0101464 ;
  Frac[210]  =    -0.00836321;
  Frac[211]  =    -0.00826306;
  Frac[212]  =    -0.00947927;
  Frac[213]  =    -0.00749842;
  Frac[214]  =    -0.00702742;
  Frac[215]  =    -0.00558369;
  Frac[216]  =    -0.00793642;
  Frac[217]  =    -0.00540675;
  Frac[218]  =    -0.00414951;
  Frac[219]  =    -0.00336889;
  Frac[220]  =    -0.00553797;
  Frac[221]  =    -0.00389439;
  Frac[222]  =    -0.00497594;
  Frac[223]  =    -0.00729843;
  Frac[224]  =    -0.00641133;
  Frac[225]  =    -0.00677281;
  Frac[226]  =    -0.00458745;
  Frac[227]  =    -0.00302479;
  Frac[228]  =    -0.00393472;
  Frac[229]  =    -0.00628557;
  Frac[230]  =    -0.00620519;
  Frac[231]  =    -0.00488894;
  Frac[232]  =    -0.0061514 ;
  Frac[233]  =    -0.00773842;
  Frac[234]  =    -0.00627602;
  Frac[235]  =    -0.0044926 ;
  Frac[236]  =    -0.0059019 ;
  Frac[237]  =    -0.00607805;
  Frac[238] =     -0.00315486;
  Frac[239] =     -0.00809495;
  Frac[240] =     -0.00581927;
  Frac[241] =     -0.00692714;
  Frac[242] =     -0.0083298 ;
  Frac[243] =     -0.00686902;
  Frac[244] =     -0.00649547;
  Frac[245] =     -0.00638095;
  Frac[246] =     -0.00716051;
  Frac[247] =     -0.00640555;
  Frac[248] =     -0.00820994;
  Frac[249] =     -0.00831537;
  Frac[250] =     -0.00961994;
  Frac[251] =     -0.00790926;
  Frac[252] =     -0.00652269;
  Frac[253] =     -0.00774797;
  Frac[254] =     -0.00572676;
  Frac[255] =     -0.00605236;
  Frac[256] =     -0.0057508 ;
  Frac[257] =     -0.00453228;
  Frac[258] =     -0.0069553 ;
  Frac[259] =     -0.00519885;
  Frac[260] =     -0.00603265;
  Frac[261] =     -0.00523939;
  Frac[262] =     -0.00299523;
  Frac[263] =     -0.00096314;
  Frac[264] =     -0.00405634;
  Frac[265] =     -0.00369099;
  Frac[266] =     -0.00506103;
  Frac[267] =     -0.00441765;
  Frac[268] =     -0.00514607;
  Frac[269] =     -0.00259789;
  Frac[270] =     -0.00653257;
  Frac[271] =     -0.00386856;
  Frac[272] =     -0.00312448;
  Frac[273] =     -0.00145314;
  Frac[274] =     -0.00241513;
  Frac[275] =     -0.00087400;
  Frac[276] =     0.000892302;
  Frac[277] =     -0.00249199;
  Frac[278] =     -0.00282059;
  Frac[279] =     0.00117736 ;
  Frac[280] =     0.0012085  ;
  Frac[281] =     0.00132184 ;
  Frac[282] =     -0.00119959;
  Frac[283] =     0.00144912 ;
  Frac[284] =     0.000605838;
  Frac[285] =     -1.06317e-0;
  Frac[286] =     -0.00142336;
  Frac[287] =     -0.00288644;
  Frac[288] =     -0.00378898;
  Frac[289] =     -0.00194118;

  if(DeltaT<290)  HeightFrac = Frac[DeltaT];
  else HeightFrac=0;						     
  return HeightFrac;						     
 									     
 }                                                                        