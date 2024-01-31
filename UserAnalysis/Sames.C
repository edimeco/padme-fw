#include "TH1D.h"
#include "TFile.h"
#include "TH1D.h"
#include "TString.h"

#include "TH2D.h"
#include "TLorentzVector.h"
#include <iostream>
#include <string>


void doSames(TString histomode){
TString histoname;
double min;
double max;
TFile *_file0 = TFile::Open("out_384.root");
histomode== "angle" ?  min,max = 0,2:  min,max =  -120, -80;
histomode== "angle" ?  histoname = "ECal_TP_DPHIAbs_probe" : histoname = "ECal_TP_DE_radiusSel";

//ECalSel->cd();

auto contarg =(TH1D*)_file0->Get("ECalSel/ECal_TP_DPHIAbs_probe")->Clone();
auto peakcontarg = (TH1D*) contarg->Clone();
peakcontarg->GetXaxis()->SetRangeUser(0, 2);
//contarg->Scale(1/peakcontarg->Integral())


TFile *_file1 = TFile::Open("out_385_notarg.root");
//ECalSel->cd();

auto notarg =(TH1D*)_file1->Get("ECalSel/ECal_TP_DPHIAbs_probe")->Clone();
auto peaknotarg = (TH1D*) notarg->Clone();
peaknotarg->GetXaxis()->SetRangeUser(0, 2);
notarg->Scale(1/(peaknotarg->Integral()/peakcontarg->Integral()));

notarg->SetLineColor(kRed);


contarg->Draw();

notarg->Draw("sames");


notarg->GetXaxis()->SetRangeUser(2.8,3.4);
notarg->Integral();

contarg->GetXaxis()->SetRangeUser(2.8,3.4);
contarg->Integral();

}