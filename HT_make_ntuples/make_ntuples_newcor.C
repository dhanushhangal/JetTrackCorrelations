#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TH2D.h"
#include "TF1.h"

#include "class_def/JetAna.h"
#include "class_def/Tracks.h"
#include "class_def/HLT.h"
#include "class_def/HiTree.h"
#include "class_def/Skim.h"
#include "class_def/GenParticles.h"
#include "class_def/pfcand.h"

#include "TH2F.h"
#include "TMath.h"
#include <TNtuple.h>
#include "TChain.h"
#include <TString.h>
#include <TCut.h>

#include "assert.h"
#include <fstream>
#include "TMath.h"
#include <vector>


using namespace std;



//arg 1 = which data set, arg 2 = output file number

enum enum_dataset_types {e_Data2011,e_Data_pp,e_HydJet15,e_HydJet30,e_HydJet50, e_HydJet80, e_HydJet120,e_HydJet170,e_HydJet220,e_HydJet280, e_HydJet370,e_Pythia15,e_Pythia30,e_Pythia50, e_Pythia80, e_Pythia120,e_Pythia170,e_Pythia220,e_Pythia280, e_Pythia370, e_n_dataset_types};
TString dataset_type_strs[e_n_dataset_types] = {"Data2011","Data_pp","HydJet15","HydJet30","HydJet50","HydJet80", "HydJet120", "HydJet170","HydJet220","HydJet280","HydJet370","Pythia15","Pythia30","Pythia50","Pythia80", "Pythia120", "Pythia170","Pythia220","Pythia280","Pythia370"};

int dataset_pthats[e_n_dataset_types+1] = {0,0,15,30,50,80,120,170,220,280,370,15,30,50,80,120,170,220,280,370,999};

int dataset_type_code = -999;

int main(int argc, char *argv[])
{
  assert( argc == 3);

  dataset_type_code = atoi(argv[1]);  

  bool is_data = false;
  
  if(dataset_type_code == e_Data2011 || dataset_type_code == e_Data_pp) is_data = true;
 
 
  // assert(!is_data); //for now I'm interested in MC

  int output_file_num = atoi(argv[2]);



  //-----------------------------
  // Set JFF-dependent corrections
  //-----------------------------

  float reco_eta, reco_phi, reco_pt, pfPt_temp, pfEta_temp, pfPhi_temp, pfId_temp, pfVsPt_temp, r, corr_pt;

  TFile *f_table = new TFile("JFF_JEC_Corrections_Pythia_Merged.root","READ");
  TH2F *corr_table = (TH2F*)f_table->Get("Corrections_by_RecoPt_Npf");


  float radius = 3;
  bool do_PbPb = 1;
  double Pf_pt_cut = 2;

  if(dataset_type_code==1||dataset_type_code>11){
    do_PbPb=0;
  }


  //--------------------------------

  //////////###### centrality Bins ###########///////////////
  
  TTree *inp_tree;
  TTree *inp_tree2;
  TTree *inp_tree3;
  TTree *inp_tree4;
  TTree *inp_tree5;
  TTree *inp_tree6;
  TTree *inp_tree7;

  TString in_file_name;
 
 
  if(is_data&&!do_PbPb){
    in_file_name = "/data/mzakaria/pp_HIForest_20150518/PPHighPtData_ForestTag_PYTHIA_localdb_ppJEC_merged_forest_0.root";
    
  }else if(dataset_type_code > 11){
    in_file_name = "/data/htrauger/Pythia_HiForest/HiForest_PYTHIA_pthat";
    in_file_name+=dataset_pthats[dataset_type_code];
    in_file_name+= ".root";
  }else{
    cerr<<"need to set up to run on that sample..."<<endl;
  }


  cout<<"File name is "<<in_file_name<<endl;

  TFile *my_file = TFile::Open(in_file_name);

  if(my_file->IsZombie()) { 
    std::cout << "Is zombie" << std::endl;
  }    

  cout<<"I've opened the file, it's not at MIT"<<endl;

  inp_tree = (TTree*)  my_file->Get("akVs3CaloJetAnalyzer/t");
  JetAna *my_ct = new JetAna(inp_tree);

  cout<<"got jets"<<endl;

  inp_tree2 = (TTree*)  my_file->Get("pfcandAnalyzer/pfTree");
  pfcand *my_ct2 = new pfcand(inp_tree2);

  cout<<"got pfcands"<<endl;

  inp_tree3 = (TTree*) my_file->Get("hiEvtAnalyzer/HiTree");
  HiTree *my_ct3 = new HiTree(inp_tree3);
  
  cout<<"got Hi tree"<<endl;

  inp_tree4 = (TTree*) my_file->Get("skimanalysis/HltTree");
  Skim *my_ct4   = new Skim(inp_tree4);

  cout<<"got HLT"<<endl;

  Tracks *my_ct5;
  if(do_PbPb){
    inp_tree5 = (TTree*) my_file->Get("anaTrack/trackTree");
    my_ct5 = new Tracks(inp_tree5);
  }else{
    inp_tree5 = (TTree*) my_file->Get("ppTrack/trackTree");
    my_ct5 = new Tracks(inp_tree5);
  }
  cout<<"got reco tracks"<<endl;


  inp_tree6 = (TTree*) my_file->Get("hltanalysis/HltTree");
  HLT *my_ct6 = new HLT(inp_tree6);

  cout<<"got other hlt tree"<<endl;
 
  GenParticles *my_ct7;
 
  if(!is_data){ 
    inp_tree7 = (TTree*) my_file->Get("HiGenParticleAna/hi"); 
    my_ct7 = new GenParticles(inp_tree7);
  }


  std::cout << "Got CT" << std::endl;

  int n_evt = my_ct->fChain->GetEntriesFast();
  
    //MC
  TString output_file_base;

  if(is_data && !do_PbPb){
    output_file_base= "/data/htrauger/pp_skim_May18/";
  }else if(dataset_type_code > 11){
    output_file_base= "/data/htrauger/Pythia_NewCorr_June8/";
      
  }else{
    cerr<<"nope, we can't handle that data set"<<endl;
    return -1;
  }

  output_file_base +=dataset_type_strs[dataset_type_code];

  cout<<"here 0"<<endl;

  TString output_file_extension = "_p";   output_file_extension += output_file_num;   output_file_extension += ".root";
  TFile *output_file = new TFile((TString) (output_file_base + output_file_extension), "RECREATE");
  output_file->cd();
  TTree *mixing_tree = new TTree("mixing_tree", "");


  cout<<"here 1"<<endl;

  Int_t mult = -999;
  Int_t nTrk = -999;
  Int_t nParticle = -999;
  Int_t nPFpart = -999;

  std::vector<Float_t> *trkEta = new std::vector<Float_t>();   trkEta->clear();
  std::vector<Float_t> *trkPhi = new std::vector<Float_t>();   trkPhi->clear();
  std::vector<Float_t> *trkPt = new std::vector<Float_t>();   trkPt->clear();
  std::vector<Float_t> *trkAlgo = new std::vector<Float_t>();   trkAlgo->clear();
  std::vector<Float_t> *highPurity = new std::vector<Float_t>();   highPurity->clear();
  std::vector<Float_t> *vz = new std::vector<Float_t>();   vz->clear();
  std::vector<Float_t> *jteta = new std::vector<Float_t>();   jteta->clear();
  std::vector<Float_t> *jtphi = new std::vector<Float_t>();   jtphi->clear();
  std::vector<Float_t> *jtpt = new std::vector<Float_t>();   jtpt->clear();
  std::vector<Float_t> *corrpt = new std::vector<Float_t>();   corrpt->clear();
  //  std::vector<Float_t> *rawpt = new std::vector<Float_t>();   rawpt->clear(); 

  std::vector<Float_t> *trackMax = new std::vector<Float_t>();   trackMax->clear();
  std::vector<Float_t> *trkDxy1 = new std::vector<Float_t>();   trkDxy1->clear();
  std::vector<Float_t> *trkDxyError1 = new std::vector<Float_t>();   trkDxyError1->clear();
  std::vector<Float_t> *trkDz1 = new std::vector<Float_t>();   trkDz1->clear();
  std::vector<Float_t> *trkDzError1 = new std::vector<Float_t>();   trkDzError1->clear();
  std::vector<Float_t> *trkPtError = new std::vector<Float_t>();   trkPtError->clear();

  std::vector<Int_t> *pfId = new std::vector<Int_t>(); pfId->clear();
  std::vector<Float_t> *pfPt = new std::vector<Float_t>(); pfPt->clear();
  std::vector<Float_t> *pfVsPt = new std::vector<Float_t>(); pfVsPt->clear();
  std::vector<Float_t> *pfEta = new std::vector<Float_t>(); pfEta->clear();
  std::vector<Float_t> *pfPhi = new std::vector<Float_t>(); pfPhi->clear();
  std::vector<Float_t> *sumpt = new std::vector<Float_t>(); sumpt->clear();


  std::vector<Int_t> *sube= new std::vector<Int_t>();  sube->clear();
  std::vector<Float_t> *pt = new std::vector<Float_t>();  pt->clear();
  std::vector<Float_t> *phi = new std::vector<Float_t>();  phi->clear();
  std::vector<Float_t> *eta = new std::vector<Float_t>();  eta->clear();
  std::vector<Int_t> *chg= new std::vector<Int_t>();  chg->clear();
  std::vector<Float_t> *pPt = new std::vector<Float_t>();  pPt->clear();
  std::vector<Float_t> *pPhi = new std::vector<Float_t>();  pPhi->clear();
  std::vector<Float_t> *pEta = new std::vector<Float_t>();  pEta->clear();
  std::vector<Float_t> *geneta = new std::vector<Float_t>();   geneta->clear();
  std::vector<Float_t> *genphi = new std::vector<Float_t>();   genphi->clear();
  std::vector<Float_t> *genpt = new std::vector<Float_t>();   genpt->clear();
 



  Int_t pHBHENoiseFilter = -999;
  Int_t pcollisionEventSelection = -999;
  Int_t HLT_HIJet80_v1 = -999;
  Int_t HLT_HIJet80_v7 = -999;
  Int_t pPAcollisionEventSelectionPA = -999;
  Int_t HLT_PAJet80_NoJetID_v1 = -999;
  Int_t hiBin = -999;
  Float_t pthat = -999;

  /// higenparticles

  mixing_tree->Branch("nTrk", &nTrk, "nTrk/I");
  mixing_tree->Branch("trkEta", "vector<Float_t>", &trkEta);
  mixing_tree->Branch("trkPhi", "vector<Float_t>", &trkPhi);
  mixing_tree->Branch("trkPt", "vector<Float_t>", &trkPt);
  mixing_tree->Branch("trkAlgo", "vector<Float_t>", &trkAlgo);
  mixing_tree->Branch("highPurity", "vector<Float_t>", &highPurity);
  mixing_tree->Branch("vz", "vector<Float_t>", &vz);
  mixing_tree->Branch("pHBHENoiseFilter", &pHBHENoiseFilter, "pHBHENoiseFilter/I");
  mixing_tree->Branch("pcollisionEventSelection", &pcollisionEventSelection, "pcollisionEventSelection/I");
  mixing_tree->Branch("HLT_HIJet80_v1", &HLT_HIJet80_v1, "HLT_HIJet80_v1/I");
  mixing_tree->Branch("HLT_HIJet80_v7", &HLT_HIJet80_v7, "HLT_HIJet80_v7/I");
 mixing_tree->Branch("pPAcollisionEventSelectionPA", &pPAcollisionEventSelectionPA, "pPAcollisionEventSelectionPA/I");
  mixing_tree->Branch("HLT_PAJet80_NoJetID_v1", &HLT_PAJet80_NoJetID_v1, "HLT_PAJet80_NoJetID_v1/I");

  mixing_tree->Branch("hiBin", &hiBin, "hiBin/I");

  mixing_tree->Branch("jteta", "vector<Float_t>", &jteta);
  mixing_tree->Branch("jtphi", "vector<Float_t>", &jtphi);
  mixing_tree->Branch("jtpt", "vector<Float_t>", &jtpt);
  mixing_tree->Branch("corrpt", "vector<Float_t>", &corrpt);
  // mixing_tree->Branch("rawpt", "vector<Float_t>", &rawpt);

  mixing_tree->Branch("pthat", &pthat, "pthat/F");
  mixing_tree->Branch("trackMax", "vector<Float_t>", &trackMax);
  mixing_tree->Branch("trkDxy1", "vector<Float_t>", &trkDxy1);
  mixing_tree->Branch("trkDxyError1", "vector<Float_t>", &trkDxyError1);
  mixing_tree->Branch("trkDz1", "vector<Float_t>", &trkDz1);
  mixing_tree->Branch("trkDzError1", "vector<Float_t>", &trkDzError1);
  mixing_tree->Branch("trkPtError", "vector<Float_t>", &trkPtError);

  if(!is_data){
    mixing_tree->Branch("mult", &mult, "mult/I");
    mixing_tree->Branch("pt", "vector<Float_t>", &pt);
    mixing_tree->Branch("phi", "vector<Float_t>", &phi);
    mixing_tree->Branch("eta", "vector<Float_t>", &eta);
    mixing_tree->Branch("chg", "vector<Int_t>", &chg);
    mixing_tree->Branch("sube", "vector<Int_t>", &sube);
    mixing_tree->Branch("nParticle", &nParticle, "nParticle/I");
    mixing_tree->Branch("pPt", "vector<Float_t>", &pPt);
    mixing_tree->Branch("pPhi", "vector<Float_t>", &pPhi);
    mixing_tree->Branch("pEta", "vector<Float_t>", &pEta);


    mixing_tree->Branch("geneta", "vector<Float_t>", &geneta);
    mixing_tree->Branch("genphi", "vector<Float_t>", &genphi);
    mixing_tree->Branch("genpt", "vector<Float_t>", &genpt);
  }

  mixing_tree->Branch("nPFpart", &nPFpart, "nPFpart/I");
  mixing_tree->Branch("pfId", "vector<Int_t>", &pfId);
  mixing_tree->Branch("pfPt", "vector<Float_t>", &pfPt);
  mixing_tree->Branch("pfVsPt", "vector<Float_t>", &pfVsPt);
  mixing_tree->Branch("pfEta", "vector<Float_t>", &pfEta);
  mixing_tree->Branch("pfPhi", "vector<Float_t>", &pfPhi);
  mixing_tree->Branch("sumpt", "vector<Float_t>", &sumpt);
  
  output_file->cd();

  int ev_min = output_file_num*100000;
  int ev_max = ev_min + 99999;
     
  cout << "ev_min: " << ev_min << ", Entries: " << n_evt << std::endl;

  assert( ev_min < n_evt );

  // n_evt = 10000;

  if( ev_max >= n_evt ) ev_max = n_evt;

  // if(!do_PbPb){ev_max = n_evt;}

  std::cout << "Will run from event number " << ev_min << " to " << ev_max << "\n";
  for(int evi = ev_min; evi < ev_max; evi++) {
      
    if( evi % 1000 == 0 )  std::cout << "evi: " << evi <<  " of " << n_evt << "\n";
    //if( evi > 1000 ) break;

  
    my_ct->fChain->GetEntry(evi);

    //cout<<"got entry 1"<<endl;

    my_ct2->fChain->GetEntry(evi);
    //cout<<"got entry 2"<<endl;

    my_ct3->fChain->GetEntry(evi);
    //cout<<"got entry 3"<<endl;
 
    my_ct4->fChain->GetEntry(evi);
    //cout<<"got entry 4"<<endl;

    my_ct5->fChain->GetEntry(evi);
    //cout<<"got entry 5"<<endl;

    my_ct6->fChain->GetEntry(evi);
    //cout<<"got entry 6"<<endl;

    if(!is_data){   my_ct7->fChain->GetEntry(evi); }

    //cout<<"so far so good"<<endl;
      
    //std::cout << "evi: " << evi << ", number of tracks: " << my_ct5->nTrk << std::endl;
    nTrk = my_ct5->nTrk;

    if( evi % 1000 == 0 ) std::cout << "Filled successfully" << std::endl;


    for(int j4i = 0; j4i < my_ct->nref ; j4i++) {

      if( fabs(my_ct->jteta[j4i]) > 2. ) continue;
     
      //-----------------------------------------------------------------------------------------
      // Jet Energy Corrections (JFF-dependent, store final corrected values in vector corr_pt)
      //----------------------------------------------------------------------------------------


      reco_pt = my_ct->jtpt[j4i];
      reco_phi = my_ct->jtphi[j4i];
      reco_eta = my_ct->jteta[j4i];
	
      int npf=0;
	
      int nPFpart = my_ct2->nPFpart;

      for(int ipf=0;ipf< nPFpart; ipf++){

	pfPt_temp = my_ct2->pfPt[ipf];
	pfVsPt_temp = my_ct2->pfVsPt[ipf];
	pfEta_temp =  my_ct2->pfEta[ipf];
	pfPhi_temp =  my_ct2->pfPhi[ipf];
	pfId_temp = my_ct2->pfId[ipf];  //pfId == 1 for hadrons only

	//cout<<pfPt<<" "<<pfVsPt<<" "<<pfEta<<" "<<pfPhi<<" "<<pfId<<endl;
	r=sqrt(pow(reco_eta-pfEta_temp,2)+pow(acos(cos(reco_phi-pfPhi_temp)),2));
	 
	if(do_PbPb&&r<((double)(radius)*0.1)&& pfVsPt_temp > Pf_pt_cut && pfEta_temp <2.4 && pfId_temp==1) npf++; 

	if(!do_PbPb&&r<((double)(radius)*0.1)&& pfPt_temp > Pf_pt_cut && pfEta_temp <2.4 && pfId_temp==1) npf++; 
      }
    
      corr_pt = reco_pt; 

      if(npf>18){npf = 18;}
	 
      if(reco_pt > 35.&&reco_pt<350.){

	corr_pt = reco_pt*corr_table->GetBinContent(corr_table->GetXaxis()->FindBin(npf),corr_table->GetYaxis()->FindBin(reco_pt));
      }
      if( my_ct->jtpt[j4i] < 35. && corr_pt < 35.) continue;
      
      jteta->push_back(reco_eta);
      jtphi->push_back(reco_phi);
      jtpt->push_back(reco_pt);
      corrpt->push_back(corr_pt);

      trackMax->push_back(my_ct->trackMax[j4i]);

    

    } /// jet loop

 
    if(!is_data){


      for(int j4i_gen = 0; j4i_gen < my_ct->ngen ; j4i_gen++) {

	if( fabs(my_ct->geneta[j4i_gen]) > 2 ) continue;
	if( my_ct->genpt[j4i_gen] < 30 ) continue;
	
	geneta->push_back(my_ct->geneta[j4i_gen]);
	genphi->push_back(my_ct->genphi[j4i_gen]);
	genpt->push_back(my_ct->genpt[j4i_gen]);
	
      } /// genjet loop

    }

    nPFpart = my_ct2->nPFpart;

    for(int pfi = 0; pfi< my_ct2->nPFpart ; pfi++) {

    
      pfId->push_back(my_ct2->pfId[pfi]);
      pfPt->push_back(my_ct2->pfPt[pfi]);
      pfVsPt->push_back(my_ct2->pfVsPt[pfi]);
      pfEta->push_back(my_ct2->pfEta[pfi]);
      pfPhi->push_back(my_ct2->pfPhi[pfi]);
      sumpt->push_back(my_ct2->sumpt[pfi]);
     
    } /// particle flow candidate loop


    //// reco track loop
    for(int itrk=0;itrk<my_ct5->nTrk;itrk++){

      //very basic cuts

      if(my_ct5->trkPtError[itrk]/my_ct5->trkPt[itrk]>=0.1 || TMath::Abs(my_ct5->trkDz1[itrk]/my_ct5->trkDzError1[itrk])>=3.0 ||TMath::Abs(my_ct5->trkDxy1[itrk]/my_ct5->trkDxyError1[itrk])>=3.0) continue ;

      float eta=my_ct5->trkEta[itrk];
      if(fabs(eta)>2.4) continue; //acceptance of the tracker   

      float pt=my_ct5->trkPt[itrk];
      if(pt < 0.5) continue; //pt min

      // reco track quantities

      if(!is_data){
	pEta->push_back(my_ct5->pEta[itrk]);
	pPhi->push_back(my_ct5->pPhi[itrk]);
	pPt->push_back(my_ct5->pPt[itrk]);

      }
	
      trkEta->push_back(my_ct5->trkEta[itrk]);
      trkPhi->push_back(my_ct5->trkPhi[itrk]);
      trkPt->push_back(my_ct5->trkPt[itrk]);
      trkAlgo->push_back(my_ct5->trkAlgo[itrk]);
      highPurity->push_back(my_ct5->highPurity[itrk]);
    
      trkDxy1 -> push_back(my_ct5->trkDxy1[itrk]);
      trkDxyError1 -> push_back(my_ct5->trkDxyError1[itrk]);
      trkDz1 -> push_back(my_ct5->trkDz1[itrk]);
      trkDzError1 -> push_back(my_ct5->trkDzError1[itrk]);
      trkPtError -> push_back(my_ct5->trkPtError[itrk]);
     
    }    
    
    if(!is_data){

      //gen particles loop
      for(int ipart=0;ipart<my_ct7->mult;ipart++){
 
	float temp_eta=my_ct7->eta[ipart];
	if(fabs(temp_eta)>2.4) continue; //acceptance of the tracker   

	float temp_pt= my_ct7->pt[ipart];
	if(temp_pt < 0.5) continue; //acceptance of the tracker
 
	// reco track quantities
	
	eta->push_back(my_ct7->eta[ipart]);
	phi->push_back(my_ct7->phi[ipart]);
	pt->push_back(my_ct7->pt[ipart]);
	chg->push_back(my_ct7->chg[ipart]);
	sube->push_back(my_ct7->sube[ipart]);
         
      }
    }

    
    pHBHENoiseFilter = my_ct4->pHBHENoiseFilter;
    pcollisionEventSelection = my_ct4->pcollisionEventSelection;
    pPAcollisionEventSelectionPA = my_ct4->pPAcollisionEventSelectionPA;
    HLT_HIJet80_v1 = my_ct6->HLT_HIJet80_v1;
    HLT_HIJet80_v7 = my_ct6->HLT_HIJet80_v7;
    HLT_PAJet80_NoJetID_v1 = my_ct6->HLT_PAJet80_NoJetID_v1;
  
    hiBin = my_ct3->hiBin;
    vz->push_back(my_ct3->vz);
    pthat = my_ct->pthat;

    ///// Fill it
    mixing_tree->Fill();

    
    ///// Reset
   
   	 
    trkEta->clear();
    trkPhi->clear();
    trkPt->clear();
    trkAlgo->clear();
    highPurity->clear();
  
    vz->clear();
  
    jteta->clear();
    jtphi->clear();
    jtpt->clear();
    corrpt->clear();
    //   rawpt->clear();
 
    trackMax->clear();

    trkDxy1->clear();
    trkDxyError1->clear();
    trkDz1->clear();
    trkDzError1->clear();
    trkPtError->clear();

    pfId->clear();
    pfPt->clear();
    pfVsPt->clear();
    pfEta->clear();
    pfPhi->clear();
    sumpt->clear();

    if(!is_data){
      pt->clear();
      phi->clear();
      eta->clear();
      chg->clear();
      sube->clear();

      pPt->clear();
      pPhi->clear();
      pEta->clear();


      geneta->clear();
      genphi->clear();
      genpt->clear();
    }


  }  ///event loop
   
  
  cout<<"writing"<<endl;
  
  //mixing_tree->Write();
  output_file->Write();
  output_file->Close();
  
  cout<<"done"<<endl;

}




