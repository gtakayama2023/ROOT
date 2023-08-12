/*

=============================================
  RIDF2Tree for RIBF123[Ca&Ni RADII]
  written by M. Tanaka(Osaka Univ.)
=============================================

modifying flow
camac_treeG.cpp (default)
-> RIDF2Tree_H093dec2014.cpp (D. Nishimura) 
-> RIDF2Tree.cpp for H093Jul2016 (M. Tanaka)

Please write the modification.
ver0.0     2016/10/18 (M. Tanaka)
==> Original
ver1.0     2016/10/21 (M. Tanaka)
==> add ImPACT IC
    add GSIIC(F11IC)
    add VETO as F0PL
ver2.0     2016/11/03 (M. Tanaka)
==> add F11PL +2channel
*/

#include <signal.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

#include "TSystem.h"
#include "TString.h"
#include "TH1.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "TMatrix.h"
#include "TCutG.h"

#include "TArtEventStore.hh"
#include "TArtRawEventObject.hh"

#include "./include/ROOTStyle.h"

#include "./include/h093_interrupt.h"
#include "./include/h093_var.h"
#include "./include/h093_raw.h"
#include "./include/h093_branch.h"



bool keeploop = true;

void stop_int(int signal){
     char inp[15];
    int sig;
    cout << "interrupt" << endl;
    printf("Keep loop : 0\nEnd loop : 1\nExit App : 2\n");
    fgets(inp,15,stdin); /* assign stdin to inp. (stdin -> standard input)*/
    sscanf(inp,"%d",&sig);/* assign the number of "inp" to sig. */

    if(sig==0){
        keeploop = true;
    }else if(sig==1){
        keeploop = false;
    }else if(sig==2){
        gSystem->ExitLoop();
    }else{
        exit(1);
    }
    return;
};


int main(int argc, char* argv[]){
  /* argc= the numbebr of argument */
  /* argv= the character of each argument */
  /* argv[0], argv[1], .... */


  
  //  TApplication theApp("MKFILE", &argc, argv);
  

  //    gSystem->Load("libanacore.so");
  //    gSystem->Load("/home/exp/yoshilib/libMyFunc.so");
  
  /*  Initialize  */
  SetRootStyle();
  
  int run;
  TString ifname, ofname;
  TString ifname0, ofname0;
  //  ifname0 = "./test/test%04d.ridf";
  //  ofname0 = "./ridf2tree/test%04d.root";
  ifname0 = "./ridf/run%04d.ridf";
  ofname0 = "./ridf2tree/run%04d.root";

  /*  check command line  */
  if(argc==2){
    char *eptr;
    run = (int)strtol(argv[1], &eptr, 10); /* read run number */
    ifname = Form(ifname0,run);
    ofname = Form(ofname0,run);
    
  }else if(argc==3){
    char opt[3];
    sscanf(argv[1],"%s",opt);
    if(strcmp(opt,"-s")==0){
      char *eptr;
      run = (int)strtol(argv[2], &eptr, 10);
      ifname = run;
      ofname = Form("camac_s%d.root", run);
    }else{
      printf("usage: camac_treeG -s ID");
      return -1;
    }
  }else{
    printf("usage: camac_treeG run_number\n");
    return -1;
  }
  
  signal(SIGINT,stop_int);            // for user interrupt
  
  
  /*  Open File & Set EventStore, RawEventObject  */
  TArtEventStore *estore = new TArtEventStore();
  TArtRawEventObject *rawevent = new TArtRawEventObject();
  rawevent = estore->GetRawEventObject();/* return rawevent */
  estore->Open(ifname); /*confirm whether the ridf file is opened or not. */

  /*  Define File, Tree, Histo, Canvas  */
  TFile *fout = new TFile(ofname, "RECREATE");
  TTree *tree = new TTree("tree","tree");
    
  /*  Set Branch  */
  h093_branch_raw(tree);

  // all event
  int neve = 0; 
  /*  Event Loop & Decode  */
  while(estore->GetNextEvent() && keeploop){

    Nevent = rawevent->GetEventNumber();

    if(Nevent<0){ continue;}

    int numseg = rawevent->GetNumSeg();/* number of segment*/

    var_raw_init();
      
    for(int i=0; i<numseg; i++){
      TArtRawSegmentObject *seg = rawevent->GetSegment(i);
      int dev=seg->GetDevice(); // ex. BigRIPS
      int fpl=seg->GetFP(); // F5
      int det=seg->GetDetector();  // PPAC-T
      int mod=seg->GetModule(); // C16 16bit Fixed (TDC)
      int num=seg->GetNumData(); // 20ch

      if(Nevent==1){
	cout <<"(SEG,dev,fpl,det,mod,ndata)=("
	     <<i<<","<<dev<<","<<fpl<<","<<det<<","<<mod<<","<<num<<")"<<endl;
      }

      for(int j=0; j<num; j++){	    
	TArtRawDataObject *d = seg->GetData(j);
	unsigned int buf = d->GetVal(); //buf ni data ga haitteru
	int ch = d->GetCh();  // cg = nanbanme no data ka

	var_fill(dev,fpl,det,mod,num,ch,buf);

      } // number of data loop
    }	// segment loop

    tree->Fill();  
    neve++;

    if(Nevent%100==0){
      std::cout << " decoded " << Nevent << "events" << std::flush << "\r";
    }

  } // Event loop

  cout << "Total Event : " << neve << endl;
  fout->Write();
    
  return 0;
}

