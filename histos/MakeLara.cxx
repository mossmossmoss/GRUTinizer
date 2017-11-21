#include <TFile.h>
#include "TRuntimeObjects.h"
#include <TStopwatch.h>
#include <TMath.h>

#include <TLara.h>
#include <time.h>

#include <GCutG.h>

GCutG* timingcut =0;

// extern "C" is needed to prevent name mangling.
// The function signature must be exactly as shown here,
//   or else bad things will happen.
extern "C" 
void MakeHistograms(TRuntimeObjects& obj) {

  TLara *lara = obj.GetDetector<TLara>();
  if(!lara) return;
  TList *list = &(obj.GetObjects());
  int numobj = list->GetSize();
  unsigned long ts = lara->Timestamp();

  double GeWindow = 110;
  double BGOGeWindow = 1200;

  // if(!timingcut) {
  //   TDirectory *c =gDirectory;
  //   TFile f("/home/daquser/newrootfiles/cuts.root");
  //   timingcut=(GCutG*)f.Get("bannana");
  //   std:: cout << timingcut->GetName() << "  loaded!" << std::endl;
  //   c->cd();
  //  }


   //std::cout << ts << std::endl;
  static TStopwatch stopwatch;
  static unsigned long firstts = ts;
  if(firstts==ts) stopwatch.Start();

  for(size_t x=0;x<lara->Size();x++) {
    TLaraHit hit = lara->GetLaraHit(x);
    // obj.FillHistogram("summaries","charge_summary",4096,0,4096,hit.Charge(),
    //                                    15,0,15,hit.Channel());
    // obj.FillHistogram("summaries","time_summary",4096,0,4096,hit.Time(),
    //                                    15,0,15,hit.Channel());
    // obj.FillHistogram("summaries","bgo_summary",4096,0,4096,hit.GetBGOTime(),
    //                                    15,0,15,hit.Channel());
    // obj.FillHistogram("summaries","charge_summary_cal",4096,0,4096,hit.GetEnergy(),
    //                                    15,0,15,hit.Channel());
    // obj.FillHistogram("hit_patt",15,0,15,hit.Channel());
      
    obj.FillHistogram("energy_singles",Form("adc%02i",hit.Channel()),4096,0,4096,hit.Charge());
    obj.FillHistogram("energy_singles","sumspec",4096,0,4096,hit.GetEnergy());
    obj.FillHistogram("energy_singles",hit.GetName(),4096,0,4096,hit.GetEnergy());

    obj.FillHistogram("time_singles",Form("tdc%02i",hit.Channel()),4096,0,4096,hit.Time());
    obj.FillHistogram("time_singles",Form("bgo%02i",hit.Channel()),4096,0,4096,hit.GetBGOTime());
      
    obj.FillHistogram("deltas",Form("GeBGOdelta%i",hit.Channel()),1000,-3000,3000,hit.Time()-hit.GetBGOTime());	

    double bgoffset = 0;
    switch(hit.Channel()){
    case 0:{
      bgoffset = 116;
      break;
    }
    case 1:{
      bgoffset = 79;
      break;
    }
    case 2:{
      bgoffset = -237;
      break;
    }
    case 3:{
      bgoffset = -312;
      break;
    }
    }
    // obj.FillHistogram("deltas",Form("GeBGOdelta%i",hit.Channel()),1000,-3000,3000,hit.Time()-hit.GetBGOTime());

    if(hit.Time()-hit.GetBGOTime() + bgoffset > -BGOGeWindow){ 
      obj.FillHistogram("GeBGOdelta",1000,-3000,3000,hit.Time()-hit.GetBGOTime() + bgoffset); //aligned GeBGOdelta plot
    }

    if(std::abs( hit.Time()-hit.GetBGOTime() + bgoffset ) > BGOGeWindow){ 
      obj.FillHistogram("supression_singles",Form("adc%02i",hit.Channel()),4096,0,4096,hit.Charge());
      obj.FillHistogram("supression_singles",hit.GetName(),4096,0,4096,hit.GetEnergy());
    }

    for(size_t y=x+1;y<lara->Size();y++) {         //!!!!!!!!!!!!!!!!!MULTIPLICITY CODE STARTS HERE!!!!!!!!!!
     TLaraHit hit1 = lara->GetLaraHit(y);
     if(hit.GetTime()>100 && hit1.GetTime()>100){ //&& hit.GetTime() != hit1.GetTime()???

     obj.FillHistogram("deltas",Form("Gedelta%i_%i",hit.Channel(),hit1.Channel()),1000,-3000,3000,hit.Time()-hit1.Time());	

     double offset = 0;     
     switch(hit.Channel()){
     case 1:{
       offset = 40;
       break;
     }
     case 2:{
       switch(hit1.Channel()){
       case 0:{
       offset = -177;
	 break;
       }
       case 1:{
       offset = -212;
	 break;
       }	
       }
       break;
     }
     case 3:{
       switch(hit1.Channel()){
       case 0:{
       offset = -141;
	 break;
       }
       case 1:{
       offset = -179;
	 break;
       }	
       case 2:{
       offset = 42;
	 break;
       }
       }
       break;
     }
     }

     obj.FillHistogram("delta",6000,-3000,3000,hit.Time()-hit1.Time() + offset);	 
 
     float time = hit.Time();
     float time1 = hit1.Time();
     float energy = hit.GetEnergy();
     float energy1 = hit1.GetEnergy();

     if(energy1<energy){
       std::swap(energy,energy1);
       std::swap(time,time1);
       offset = offset * (-1);
     }

     if(std::abs(time-time1 + offset) < 1250){ // < 1250 to get rid of self coincident lines
       obj.FillHistogram("delta_v_en",3000,-1500,1500,time-time1 + offset,2048,0,2048,energy);
       obj.FillHistogram("en_v_delta",2048,0,2048,energy,3000,-1500,1500,time-time1 + offset);
     }

     if(std::abs(time-time1 + offset) <= GeWindow && std::abs( hit.Time()-hit.GetBGOTime() + bgoffset ) > BGOGeWindow){
     obj.FillHistogram("Coincidence_Matrix",4096,0,4096,hit.GetEnergy(),
		       4096,0,4096,hit1.GetEnergy());
     obj.FillHistogram("Coincidence_Matrix",4096,0,4096,hit1.GetEnergy(),
		       4096,0,4096,hit.GetEnergy());
     }

     // if(hit.GetEnergy()>10 && hit1.GetEnergy()>10){
     //   obj.FillHistogram("hit_pattern_en",4,0,4,hit.Channel(),4,0,4,hit1.Channel());	
     //   obj.FillHistogram("hit_pattern_en",4,0,4,hit1.Channel(),4,0,4,hit.Channel());	
     // }

     // if(hit.GetTime()>10 && hit.GetTime()<3000 && hit1.GetTime()>10 && hit.GetTime()<3000){
     //   obj.FillHistogram("hit_pattern_time",4,0,4,hit.Channel(),4,0,4,hit1.Channel());	
     //   obj.FillHistogram("hit_pattern_time",4,0,4,hit1.Channel(),4,0,4,hit.Channel());	
     // }

     }
    }
  }

    obj.FillHistogram("multiplicity",6,0,6,lara->Size());  
 
    if(numobj!=list->GetSize())
      list->Sort();
}    


