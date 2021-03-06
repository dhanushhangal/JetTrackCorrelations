#include "TFile.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TLatex.h"
#include <iostream>
#include "TVirtualFitter.h"

using namespace std;


//assuming fit errors are uncorrelated...
double calcIntegralError(TF1 *input, double* covMatrix=0){
	
	double err2=0;
	for(int ipar=0; ipar<input->GetNpar(); ipar++){
		if(!covMatrix){
			err2 += input->GetParameter(ipar)*input->GetParameter(ipar)*input->GetParError(ipar)*input->GetParError(ipar);
		}
		else{
			double s = 0;
			for (int j = 0; j < input->GetNpar(); ++j) {
				s += covMatrix[ipar*input->GetNpar() + j] * input->GetParameter(j);
			}
			err2 += input->GetParameter(ipar) * s;
		}
	}
	return sqrt(err2);
}

void factorizeCorrs(){
	
	bool doSpillover = false;
	
	const double xTrkBinDouble[11] = {0.5,0.7,1.,2.,3.,4.,8.,12.,16.,20.,30.};
	const string xCentBins[5] = {"Cent0", "Cent10", "Cent30","Cent50", "Cent100"};
	
	TFile *fsum = new TFile("JFFcorrs_PythHydjet_pTweighted_sube0_newJFFJEC_withFits.root");
	
	TFile *fq = new TFile("JFFcorrs_PythiaOnly_pTweighted_sube0_newJFFJEC_withFits.root");
	TFile *fg = new TFile("JFFcorrs_Pythiapp_oldJFFJEC_withFits.root");
	
	//TFile *fout;
	//if(doSpillover){
	//	fout = new TFile("JFFcorrs_spilloverFromsube0_newJFFs.root","recreate");
	//	fout->cd();
	//}
	TH2D *hnum[10][4], *hden[10][4];
	TH1D *fp1[10][4];
	
	TH2D *hq[10][4];
	TH2D *hg[10][4];
	
	TH1D *fpq[10][4];
	TH1D *fpg[10][4];
	
	TLatex *labels[4];
	TCanvas *cc = new TCanvas("cc","",2000,1600);
	cc->Divide(4,8);
	TLatex *labels2[10][4];
	
	TH1D *ptClosure[4];
	TH1D *ptClosureFit[4];
	TH1D *gptClosure[4];
	TH1D *qptClosure[4];
	
	TF1 *gausFit[10][4];
	TF1 *gausFitg[10][4];
	TF1 *gausFitq[10][4];
	
	double *covMatrix[10][4];
	double *qcovMatrix[10][4];
	double *gcovMatrix[10][4];
	
	for(int j=0; j<4; j++){
		
		ptClosureFit[j] = new TH1D(Form("ptClosureFit_%d",j),"",10,xTrkBinDouble); ptClosureFit[j]->Sumw2();
		ptClosure[j] = new TH1D(Form("ptClosure_%d",j),"",10,xTrkBinDouble); ptClosure[j]->Sumw2();
		gptClosure[j] = new TH1D(Form("gptClosureFit_%d",j),"",10,xTrkBinDouble); gptClosure[j]->Sumw2();
		qptClosure[j] = new TH1D(Form("qptClosure_%d",j),"",10,xTrkBinDouble); qptClosure[j]->Sumw2();
		
		for(int i=0; i<10; i++){
			
			hnum[i][j] = (TH2D*)fsum->Get(Form("JFFcorrs_cent%d_pt%d",j,i))->Clone(Form("den_cent%d_pt%d",j,i));
			
			hq[i][j] = (TH2D*)fq->Get(Form("JFFcorrs_cent%d_pt%d",j,i))->Clone(Form("hq_jffCorr_cent%d_pt%d",j,i));
			hg[i][j] = (TH2D*)fg->Get(Form("JFFcorrs_cent%d_pt%d",j,i))->Clone(Form("hg_jffCorr_cent%d_pt%d",j,i));
			
			//if(doSpillover){
			//	hnum[i][j]->Add(hden[i][j],-1);
			//	hnum[i][j]->Write();
			//}
			//else hnum[i][j] = hden[i][j];
			//hnum[i][j] = hden[i][j];
			
			gausFit[i][j] = new TF1(Form("gausFit_%d_%d",i,j),"gaus+[3]",-1,1);
			gausFit[i][j]->SetParLimits(0,0,1);
			gausFit[i][j]->SetParLimits(2,9e-2,5e-1);
			gausFit[i][j]->FixParameter(1,0);
			if(i<2) gausFit[i][j]->FixParameter(3,0);
			
			gausFitq[i][j] = new TF1(Form("qgausFit_%d_%d",i,j),"gaus+[3]",-1,1);
			gausFitg[i][j] = new TF1(Form("ggausFit_%d_%d",i,j),"gaus+[3]",-1,1);
			gausFitq[i][j]->SetParLimits(0,0,1);
			gausFitq[i][j]->SetParLimits(2,9e-2,5e-1);
			gausFitq[i][j]->FixParameter(1,0);
			if(i<2) gausFitq[i][j]->FixParameter(3,0);
			gausFitg[i][j]->SetParLimits(0,0,1);
			gausFitg[i][j]->SetParLimits(2,9e-2,5e-1);
			gausFitg[i][j]->FixParameter(1,0);
			if(i<2) gausFitg[i][j]->FixParameter(3,0);

			if(i>0 && i<10){
				cc->cd(i*4+(3-j)+1-4);
				int lowbin = hnum[i][j]->GetYaxis()->FindBin(-1);
				int hibin = hnum[i][j]->GetYaxis()->FindBin(1);
				fp1[i][j] = (TH1D*)hnum[i][j]->ProjectionX(Form("ipfx_%d%d",i,j),lowbin,hibin,"e");
				fpq[i][j] = (TH1D*)hq[i][j]->ProjectionX(Form("qpfx_%d%d",i,j),0,-1,"e");
				fpg[i][j] = (TH1D*)hg[i][j]->ProjectionX(Form("gpfx_%d%d",i,j),0,-1,"e");
				if(doSpillover){
					fp1[i][j]->Fit(gausFit[i][j],"B","",-1,1);
					TVirtualFitter *fitter = TVirtualFitter::GetFitter();
					assert(fitter!=0);
					covMatrix[i][j] = fitter->GetCovarianceMatrix();
					fpg[i][j]->Fit(gausFitg[i][j],"BqN0","",-1,1);				
					fpq[i][j]->Fit(gausFitq[i][j],"BqN0","",-1,1);
				}
				fp1[i][j]->Rebin(5);
				fp1[i][j]->SetLineColor(1);
				fpq[i][j]->Rebin(5);
				fpq[i][j]->SetLineColor(4);
				fpg[i][j]->Rebin(5);
				fpg[i][j]->SetLineColor(2);
				fp1[i][j]->GetXaxis()->SetRangeUser(-1.5,1.5);
				fp1[i][j]->GetYaxis()->SetNdivisions(505);
				fp1[i][j]->GetYaxis()->SetLabelSize(0.15);
				fp1[i][j]->GetXaxis()->SetLabelSize(0.15);
				fp1[i][j]->SetMaximum(fpg[i][j]->GetMaximum()*1.5);
				fp1[i][j]->SetMinimum(fpq[i][j]->GetMinimum()*1.5);
				fp1[i][j]->Draw();
				//fpq[i][j]->Divide(fp1[i][j]);
				fpq[i][j]->Draw("same");
				//fpg[i][j]->Divide(fp1[i][j]);
				//fpg[i][j]->Draw("same");
				labels2[i][j] = new TLatex(-1,fp1[i][j]->GetMaximum()*0.65,Form("%f < pt < %f, %s-%s",xTrkBinDouble[i],xTrkBinDouble[i+1],xCentBins[j].c_str(),xCentBins[j+1].c_str()));
				labels2[i][j]->SetTextSize(0.15);
				labels2[i][j]->Draw("same");
			
				double integrErr =0.;
				double integr = fp1[i][j]->IntegralAndError(1, fp1[i][j]->GetNbinsX(), integrErr);
				double integr2, integrErr2;
				double gInteg, qInteg;
				if(doSpillover){
					integr = gausFit[i][j]->Integral(-1,1) - gausFit[i][j]->GetParameter(3)*2.;
					integrErr = calcIntegralError(gausFit[i][j]);
					
					cout << "integral error: "<< integrErr << endl;
					
					gInteg = gausFitg[i][j]->Integral(-1,1) - gausFitg[i][j]->GetParameter(3)*2.;
					qInteg = gausFitq[i][j]->Integral(-1,1) - gausFitq[i][j]->GetParameter(3)*2.;
				}
				else{
					gInteg = fpg[i][j]->Integral();
					qInteg = fpq[i][j]->Integral();
				}				
				
				ptClosure[j]->SetBinContent(i+1, integr/ptClosure[j]->GetBinWidth(i+1));
				if(doSpillover) ptClosure[j]->SetBinContent(i+1, ptClosure[j]->GetBinContent(i+1)/fp1[i][j]->GetBinWidth(2));
				ptClosure[j]->SetBinError(i+1, integrErr/ptClosure[j]->GetBinWidth(i+1));
				
				//ptClosureFit[j]->SetBinContent(i+1, integr2/ptClosureFit[j]->GetBinWidth(i+1)/fp1[i][j]->GetBinWidth(2));
				//ptClosureFit[j]->SetBinError(i+1, integrErr2/ptClosureFit[j]->GetBinWidth(i+1)/fp1[i][j]->GetBinWidth(2));
				
				gptClosure[j]->SetBinContent(i+1, gInteg/ptClosure[j]->GetBinWidth(i+1));
				if(doSpillover) gptClosure[j]->SetBinContent(i+1, gptClosure[j]->GetBinContent(i+1)/fpg[i][j]->GetBinWidth(2));
				gptClosure[j]->SetBinError(i+1, ptClosure[j]->GetBinError(i+1));
				
				qptClosure[j]->SetBinContent(i+1, qInteg/ptClosure[j]->GetBinWidth(i+1));
				if(doSpillover) qptClosure[j]->SetBinContent(i+1, qptClosure[j]->GetBinContent(i+1)/fpq[i][j]->GetBinWidth(2));
				qptClosure[j]->SetBinError(i+1, ptClosure[j]->GetBinError(i+1));
								
				cout << "cent bin " << j << " pt bin " << i << " integral: " << ptClosure[j]->GetBinContent(i+1) << " error: "<< ptClosure[j]->GetBinError(i+1) << endl;
				cout << "cent bin " << j << " pt bin " << i << " gluon integral: " << gInteg << endl;
				cout << "cent bin " << j << " pt bin " << i << " quark integral: " << qInteg << endl;
			}
			
		}
	}
	
	TCanvas *c2 = new TCanvas("c2","",2000,400);
	c2->Divide(4,1);
	for(int j=0; j<4; j++){
		c2->cd(4-j);
		//if(doSpillover) ptClosure[j] = ptClosureFit[j];
		ptClosure[j]->SetMaximum(1.5);
		ptClosure[j]->SetMinimum(-0.5-1*(!doSpillover));
		ptClosure[j]->SetXTitle("Track p_{T} (GeV/c)");
		if(!doSpillover) ptClosure[j]->SetYTitle("JFF Correction");
		else ptClosure[j]->SetYTitle("Spillover Correction");
		//ptClosure[j]->Divide(qptClosure[j]);
		ptClosure[j]->Draw();
		//ptClosureFit[j]->SetMarkerColor(2);
		//ptClosureFit[j]->SetLineColor(2);
		//if(doSpillover) ptClosureFit[j]->Draw("");
		gptClosure[j]->SetMaximum(2);
		gptClosure[j]->SetMinimum(0);
		gptClosure[j]->SetMarkerColor(2);
		//gptClosure[j]->SetYTitle("New/Old JFF Corrections");
		gptClosure[j]->SetXTitle("Track p_{T} (GeV/c)");
		//gptClosure[j]->Divide(ptClosure[j],gptClosure[j]);
		//gptClosure[j]->Draw("same");
		qptClosure[j]->SetMarkerColor(4);
		//qptClosure[j]->Divide(ptClosure[j]);
		qptClosure[j]->Draw("same");
		labels[j] = new TLatex(3,ptClosure[j]->GetMaximum()*0.85,Form("%s-%s",xCentBins[j].c_str(),xCentBins[j+1].c_str()));
		labels[j]->SetTextSize(0.06);
		labels[j]->Draw("same");
		if(j==0){
			cout << endl;
			for(int ibin=1; ibin<=qptClosure[j]->GetNbinsX(); ibin++){
				cout << qptClosure[j]->GetBinContent(ibin) << ", ";
			}
			cout << endl;
		}
	}
	
	TFile *fout = new TFile("Spillover_gausFits.root","recreate");
	fout->cd();
	for(int j=0; j<4; j++){
		for(int i=0; i<10; i++){
			gausFit[i][j]->Write();
		}
	}
	fout->Close();
			
}