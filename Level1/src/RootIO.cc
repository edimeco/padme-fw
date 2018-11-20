#include <stdio.h>
#include <time.h>

#include "DBService.hh"
#include "Configuration.hh"
#include "RootIO.hh"

RootIO::RootIO()
{

  // Create TFile handle
  //fTFileHandle = new TFile();
  fTFileHandle = 0;

  // Create TRawEvent object
  fTRawEvent = new TRawEvent();

  // Connect to configuration class
  fConfig = Configuration::GetInstance();

  // Connect to DB service
  fDB = 0;
  if (fConfig->RunNumber()) fDB = DBService::GetInstance();

}

RootIO::~RootIO()
{
  delete fTFileHandle;
  delete fTRawEvent;
}

//int RootIO::Init(std::string outfiletemplate, int nevtsperfile)
int RootIO::Init()
{

  fOutFileDBId = 0;
  fOutFileIndex = 0;
  fOutFileEvents = 0;
  fOutEventsTotal = 0;
  fOutSizeTotal = 0;

  // Set initial output filename
  if (fConfig->NEventsPerFile() == 0) {
    fOutFile.Form("%s.root",fConfig->RawFileHeader().c_str());
  } else {
    SetOutFile();
  }

  // Open output file
  //printf("RootIO::Init - Creating %s output file.\n",fOutFile.Data());
  return (int)OpenOutFile();

}

int RootIO::Exit()
{
  printf("RootIO::Exit - Finalizing output.\n");
  if (CloseOutFile() != ROOTIO_OK) return ROOTIO_ERROR;

  printf("RootIO::Exit - Total files created:  %u\n",fOutFileIndex+1);
  printf("RootIO::Exit - Total events written: %u\n",fOutEventsTotal);
  printf("RootIO::Exit - Total output size:    %lu\n",fOutSizeTotal);

  return ROOTIO_OK;
}

Int_t RootIO::ChangeOutFile()
{

  if (CloseOutFile() != ROOTIO_OK) return ROOTIO_ERROR;

  // Update file index and create new filename
  fOutFileIndex++;
  SetOutFile();

  return OpenOutFile();

}

Int_t RootIO::OpenOutFile()
{

  if (file_exists(fOutFile.Data())) {
    printf("RootIO::OpenOutFile - ERROR: file %s already exists\n",fOutFile.Data());
    return ROOTIO_ERROR;
  }

  printf("RootIO::OpenOutFile - Opening output file %s\n",fOutFile.Data());
  fTFileHandle = TFile::Open(fOutFile,"NEW","PADME Merged Raw Events");
  if ( (!fTFileHandle) || fTFileHandle->IsZombie() ) {
    delete fTFileHandle;
    return ROOTIO_ERROR;
  }

  // Create TTree to hold raw events
  fTTreeMain = new TTree("RawEvents","PADME Raw Events Tree");

  // Attach branch to TRawEvent
  fTTreeMain->Branch("RawEvent",&fTRawEvent);

  // Reset event counter for this file
  fOutFileEvents = 0;

  // Register file in DB
  if (fDB) {
    int rc = fDB->OpenRawFile(fOutFileDBId,fConfig->MergerId(),fOutFile.Data(),fOutFileIndex);
    if (rc != DBSERVICE_OK) {
      printf("RootIO::OpenOutFile - ERROR while updating DB\n");
      return ROOTIO_ERROR;
    }
  }

  return ROOTIO_OK;

}

Int_t RootIO::CloseOutFile()
{

  printf("RootIO::CloseOutFile - Closing output file %s\n",fOutFile.Data());

  // Save TTree content
  fTTreeMain->Write();

  // Close output file
  fTFileHandle->Close();

  // Add size of this file to total
  fOutFileSize = fTFileHandle->GetSize();
  fOutSizeTotal += fOutFileSize;

  // Delete file handle for this file
  //delete fTTreeMain;
  delete fTFileHandle; // This takes also care of deleting the TTree

  // Update DB with file close information
  if (fDB) {
    int rc = fDB->CloseRawFile(fOutFileDBId,fOutFileEvents,fOutFileSize);
    if (rc != DBSERVICE_OK) {
      printf("RootIO::CloseOutFile - ERROR while updating DB\n");
      return ROOTIO_ERROR;
    }
  }

  return ROOTIO_OK;
}

Int_t RootIO::SetOutFile()
{
  //fOutFile.Form("%s_%03d.root",fOutFileTemplate.Data(),fOutFileIndex);
  fOutFile.Form("%s_%03d.root",fConfig->RawFileHeader().c_str(),fOutFileIndex);
  return ROOTIO_OK;
}

//int RootIO::FillRawEvent(int runnr, int evtnr, std::vector<ADCBoard*>& boards)
//int RootIO::FillRawEvent(int runnr, int evtnr, unsigned long int runtime, unsigned int trgmsk, unsigned int evtstatus, std::vector<ADCBoard*>& boards)
int RootIO::FillRawEvent(int run_number,
			 unsigned int event_number,
			 struct timespec event_time,
			 unsigned long long int event_run_time,
			 unsigned int event_trigger_mask,
			 unsigned int event_status,
			 unsigned int missing_adcboards,
			 unsigned int trigger_mask,
			 unsigned int trigger_counter,
			 unsigned long long int trigger_clock,
			 unsigned int number_adc_boards,
			 ADCBoard** boards)
{

  // Emtpy event structure
  fTRawEvent->Clear("C");

  // Set run and event number
  fTRawEvent->SetRunNumber((Int_t)run_number);
  fTRawEvent->SetEventNumber((UInt_t)event_number);

  // Save system time when event was merged
  TTimeStamp systime = TTimeStamp(event_time.tv_sec,event_time.tv_nsec);
  fTRawEvent->SetEventAbsTime(systime);

  // Set event time since start of run
  fTRawEvent->SetEventRunTime((ULong64_t)event_run_time);

  // Set trigger mask
  fTRawEvent->SetEventTrigMask((UInt_t)event_trigger_mask);

  // Set event status
  fTRawEvent->SetEventStatus((UInt_t)event_status);

  // Set missing boards bit mask
  fTRawEvent->SetMissingADCBoards((UInt_t)missing_adcboards);

  // Print event info when in verbose mode
  if (fConfig->Verbose() >= 1) {
    printf("RootIO - Run %d Event %u Time %s Clock %llu Trig 0x%08x Status 0x%08x Missing boards 0x%08x\n",run_number,event_number,systime.AsString(),event_run_time,event_trigger_mask,event_status,missing_adcboards);
  }

  // Loop over all ADC boards
  for(unsigned int b=0; b<number_adc_boards; b++) {

    TADCBoard* tBoard = fTRawEvent->AddADCBoard();

    // Save general board information for this event
    tBoard->SetBoardId            (boards[b]->Event()->GetBoardId());
    tBoard->SetBoardSN            (boards[b]->Event()->GetBoardSN());
    tBoard->SetLVDSPattern        (boards[b]->Event()->GetLVDSPattern());
    tBoard->SetBoardStatus        (boards[b]->Event()->GetBoardStatus());
    tBoard->SetGroupMask          (boards[b]->Event()->GetGroupMask());
    tBoard->SetEventCounter       (boards[b]->Event()->GetEventCounter());
    tBoard->SetEventTimeTag       (boards[b]->Event()->GetEventTimeTag());
    tBoard->Set0SuppAlgrtm        (boards[b]->Event()->Get0SuppAlgrtm());
    tBoard->SetActiveChannelMask  (boards[b]->Event()->GetActiveChannelMask());
    tBoard->SetAcceptedChannelMask(boards[b]->Event()->GetAcceptedChannelMask());
    //printf("TRawEvent board info saved\n");

    if (fConfig->Verbose() >= 2)
      printf("RootIO - Board %u S/N %u LVDS 0x%04x Status 0x%03x GMask 0x%1x Event %u Time %u 0SupAlg %u ActiveCh 0x%08x AcceptCh 0x%08x\n",
	     tBoard->GetBoardId(),tBoard->GetBoardSN(),tBoard->GetLVDSPattern(),tBoard->GetBoardStatus(),tBoard->GetGroupMask(),
	     tBoard->GetEventCounter(),tBoard->GetEventTimeTag(),tBoard->Get0SuppAlgrtm(),tBoard->GetActiveChannelMask(),tBoard->GetAcceptedChannelMask());

    // Get mask of channel to save taking into account the zero suppression functioning mode
    unsigned int channelmask;
    if ( tBoard->GetBoardStatus() & (0x1 << 2) ) {
      channelmask = tBoard->GetActiveChannelMask(); // Zero suppression is in flagging mode
    } else {
      channelmask = tBoard->GetAcceptedChannelMask(); // Zero suppression is in rejection mode
    }

    // If a tigger group has no channels, do not save its trigger information
    unsigned char groupmask = 0;
    for(unsigned int t=0; t<ADCEVENT_NTRIGGERS; t++) {
      if ( channelmask & ( 0xff << (t*8) ) ) groupmask |= (0x1 << t);
    }

    // Save triggers information for this board
    for(unsigned int t=0; t<ADCEVENT_NTRIGGERS; t++) {
      //if ( boards[b]->Event()->GetGroupMask() & (0x1 << t) ) {
      //if ( tBoard->GetGroupMask() & (0x1 << t) ) {
      if ( groupmask & (0x1 << t) ) {
	TADCTrigger* tTrig = tBoard->AddADCTrigger();
	//printf("TRawEvent board %d trigger %d created with address %ld\n",b,t,(long)tTrig);
	tTrig->SetGroupNumber   (t);
	tTrig->SetStartIndexCell(boards[b]->Event()->GetTriggerStartIndexCell(t));
	tTrig->SetFrequency     (boards[b]->Event()->GetTriggerFrequency(t));
	tTrig->SetTriggerSignal (boards[b]->Event()->GetTriggerHasSignal(t));
	tTrig->SetTriggerTimeTag(boards[b]->Event()->GetTriggerTimeTag(t));
	if (fConfig->Verbose() >= 3)
	  printf("RootIO - Board %u Group %u SIC %u Freq 0x%1x HasData 0x%1x TTT 0x%08x\n",
	  tBoard->GetBoardId(),tTrig->GetGroupNumber(),tTrig->GetStartIndexCell(),tTrig->GetFrequency(),tTrig->GetTriggerSignal(),tTrig->GetTriggerTimeTag());
	if ( boards[b]->Event()->GetTriggerHasSignal(t) ) {
	  for(unsigned int s=0; s<ADCEVENT_NSAMPLES; s++)
	    tTrig->SetSample(s,boards[b]->Event()->GetADCTriggerSample(t,s));
	  if (fConfig->Verbose() >= 4) {
	    for(unsigned int s=0; s<ADCEVENT_NSAMPLES; s++) {
	      if (s%16 == 0) printf("\t");
	      printf("%4x ",tTrig->GetSample(s));
	      if (s%16 == 15) printf("\n");
	    }
	  }
	}
      }
    }

    // Save channels information for this board
    for(unsigned int c=0; c<ADCEVENT_NCHANNELS; c++) {
      if ( channelmask & (0x1 << c) ) {
	TADCChannel* tChan = tBoard->AddADCChannel();
	tChan->SetChannelNumber(c);
	for(unsigned int s=0; s<ADCEVENT_NSAMPLES; s++)
	  tChan->SetSample(s,boards[b]->Event()->GetADCChannelSample(c,s));
	if (fConfig->Verbose() >= 4) {
	  printf("RootIO - Board %u Channel %u\n",tBoard->GetBoardId(),tChan->GetChannelNumber());
	  for(unsigned int s=0; s<ADCEVENT_NSAMPLES; s++) {
	    if (s%16 == 0) printf("\t");
	    printf("%4x ",tChan->GetSample(s));
	    if (s%16 == 15) printf("\n");
	  }
	}
      }
    }

  }

  // Write current event to file
  fTTreeMain->Fill();

  // Count event and see if we have to change file
  fOutEventsTotal++;
  fOutFileEvents++;
  if (fConfig->NEventsPerFile() && (fOutFileEvents>=fConfig->NEventsPerFile())) {
    if (ChangeOutFile() != ROOTIO_OK) return ROOTIO_ERROR;
  }

  return ROOTIO_OK;

}
