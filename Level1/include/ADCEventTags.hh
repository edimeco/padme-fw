#ifndef ADCEventTags_H
#define ADCEventTags_H

// Informations which MUST NOT vary with version:
// Section tags (highest 4 bits of each section)
// --- file header: 0x9 - event header: 0xE - file tail: 0x5
// Data format version (next 16 bits of file header)
// Event section size in 4-bytes words (next 28 bits of event header)

// Section tags

#define ADCEVENT_TAG_LIN 0
#define ADCEVENT_TAG_POS 28
#define ADCEVENT_TAG_BIT 0xF0000000

#define ADCEVENT_FHEAD_TAG 0x9
#define ADCEVENT_EVENT_TAG 0xE
#define ADCEVENT_FTAIL_TAG 0x5

// Data format version

#define ADCEVENT_VERSION_LIN 0
#define ADCEVENT_VERSION_POS 16
#define ADCEVENT_VERSION_BIT 0x0FFF0000

// Event section total size in 4-bytes words

#define ADCEVENT_EVENTSIZE_LIN 0
#define ADCEVENT_EVENTSIZE_POS 0
#define ADCEVENT_EVENTSIZE_BIT 0x0FFFFFFF

// Information related to physical structure of V1742 ADC board

#define ADCEVENT_NCHANNELS 32
#define ADCEVENT_NTRIGGERS 4
#define ADCEVENT_NSAMPLES 1024

  ////////////////
 // VERSION 01 //
////////////////

#define ADCEVENT_V01_FORMAT_VERSION 1

// File header structure

#define ADCEVENT_V01_FHEAD_LEN 3

#define ADCEVENT_V01_FILEINDEX_LIN 0
#define ADCEVENT_V01_FILEINDEX_POS 0
#define ADCEVENT_V01_FILEINDEX_BIT 0x0000FFFF

#define ADCEVENT_V01_RUNNUMBER_LIN 1
#define ADCEVENT_V01_RUNNUMBER_POS 0
#define ADCEVENT_V01_RUNNUMBER_BIT 0xFFFFFFFF

#define ADCEVENT_V01_TTAGSOF_LIN 2
#define ADCEVENT_V01_TTAGSOF_POS 0
#define ADCEVENT_V01_TTAGSOF_BIT 0xFFFFFFFF

// File tail structure

#define ADCEVENT_V01_FTAIL_LEN 4

#define ADCEVENT_V01_NEVENTS_LIN 0
#define ADCEVENT_V01_NEVENTS_POS 0
#define ADCEVENT_V01_NEVENTS_BIT 0x0FFFFFFF

#define ADCEVENT_V01_FILESIZELO_LIN 1
#define ADCEVENT_V01_FILESIZELO_POS 0
#define ADCEVENT_V01_FILESIZELO_BIT 0xFFFFFFFF

#define ADCEVENT_V01_FILESIZEHI_LIN 2
#define ADCEVENT_V01_FILESIZEHI_POS 0
#define ADCEVENT_V01_FILESIZEHI_BIT 0xFFFFFFFF

#define ADCEVENT_V01_TTAGEOF_LIN 3
#define ADCEVENT_V01_TTAGEOF_POS 0
#define ADCEVENT_V01_TTAGEOF_BIT 0xFFFFFFFF

// Event header structure

#define ADCEVENT_V01_EVENTHEAD_LEN 6

#define ADCEVENT_V01_BOARDID_LIN 1
#define ADCEVENT_V01_BOARDID_POS 27
#define ADCEVENT_V01_BOARDID_BIT 0xF8000000

#define ADCEVENT_V01_PATTERN_LIN 1
#define ADCEVENT_V01_PATTERN_POS 8
#define ADCEVENT_V01_PATTERN_BIT 0x00FFFF00

#define ADCEVENT_V01_STATUS_LIN 1
#define ADCEVENT_V01_STATUS_POS 4
#define ADCEVENT_V01_STATUS_BIT 0x000000F0

#define ADCEVENT_V01_GRMASK_LIN 1
#define ADCEVENT_V01_GRMASK_POS 0
#define ADCEVENT_V01_GRMASK_BIT 0x0000000F

#define ADCEVENT_V01_EVENTNUMBER_LIN 2
#define ADCEVENT_V01_EVENTNUMBER_POS 0
#define ADCEVENT_V01_EVENTNUMBER_BIT 0x003FFFFF

#define ADCEVENT_V01_TTAGEVENT_LIN 3
#define ADCEVENT_V01_TTAGEVENT_POS 0
#define ADCEVENT_V01_TTAGEVENT_BIT 0xFFFFFFFF

#define ADCEVENT_V01_CHMASKACTIVE_LIN 4
#define ADCEVENT_V01_CHMASKACTIVE_POS 0
#define ADCEVENT_V01_CHMASKACTIVE_BIT 0xFFFFFFFF

#define ADCEVENT_V01_CHMASKACCEPTED_LIN 5
#define ADCEVENT_V01_CHMASKACCEPTED_POS 0
#define ADCEVENT_V01_CHMASKACCEPTED_BIT 0xFFFFFFFF

// Group header structure

#define ADCEVENT_V01_GRHEAD_LEN 1

#define ADCEVENT_V01_STARTIDXCELL_LIN 0
#define ADCEVENT_V01_STARTIDXCELL_POS 16
#define ADCEVENT_V01_STARTIDXCELL_BIT 0xFFFF0000

#define ADCEVENT_V01_FREQUENCY_LIN 0
#define ADCEVENT_V01_FREQUENCY_POS 14
#define ADCEVENT_V01_FREQUENCY_BIT 0x0000C000

#define ADCEVENT_V01_TRHASDATA_LIN 0
#define ADCEVENT_V01_TRHASDATA_POS 12
#define ADCEVENT_V01_TRHASDATA_BIT 0x00003000

#define ADCEVENT_V01_GRSIZE_LIN 0
#define ADCEVENT_V01_GRSIZE_POS 0
#define ADCEVENT_V01_GRSIZE_BIT 0x00000FFF

// Group tail structure

#define ADCEVENT_V01_GRTAIL_LEN 1

#define ADCEVENT_V01_TTAGTRIG_LIN 0
#define ADCEVENT_V01_TTAGTRIG_POS 0
#define ADCEVENT_V01_TTAGTRIG_BIT 0xFFFFFFFF

  ////////////////
 // VERSION 02 //
////////////////

#define ADCEVENT_V02_FORMAT_VERSION 2

// File header structure

#define ADCEVENT_V02_FHEAD_LEN 3

#define ADCEVENT_V02_FILEINDEX_LIN 0
#define ADCEVENT_V02_FILEINDEX_POS 0
#define ADCEVENT_V02_FILEINDEX_BIT 0x0000FFFF

#define ADCEVENT_V02_RUNNUMBER_LIN 1
#define ADCEVENT_V02_RUNNUMBER_POS 0
#define ADCEVENT_V02_RUNNUMBER_BIT 0xFFFFFFFF

#define ADCEVENT_V02_TTAGSOF_LIN 2
#define ADCEVENT_V02_TTAGSOF_POS 0
#define ADCEVENT_V02_TTAGSOF_BIT 0xFFFFFFFF

// File tail structure

#define ADCEVENT_V02_FTAIL_LEN 4

#define ADCEVENT_V02_NEVENTS_LIN 0
#define ADCEVENT_V02_NEVENTS_POS 0
#define ADCEVENT_V02_NEVENTS_BIT 0x0FFFFFFF

#define ADCEVENT_V02_FILESIZELO_LIN 1
#define ADCEVENT_V02_FILESIZELO_POS 0
#define ADCEVENT_V02_FILESIZELO_BIT 0xFFFFFFFF

#define ADCEVENT_V02_FILESIZEHI_LIN 2
#define ADCEVENT_V02_FILESIZEHI_POS 0
#define ADCEVENT_V02_FILESIZEHI_BIT 0xFFFFFFFF

#define ADCEVENT_V02_TTAGEOF_LIN 3
#define ADCEVENT_V02_TTAGEOF_POS 0
#define ADCEVENT_V02_TTAGEOF_BIT 0xFFFFFFFF

// Event header structure

#define ADCEVENT_V02_EVENTHEAD_LEN 6

#define ADCEVENT_V02_BOARDID_LIN 1
#define ADCEVENT_V02_BOARDID_POS 24
#define ADCEVENT_V02_BOARDID_BIT 0xFF000000

#define ADCEVENT_V02_PATTERN_LIN 1
#define ADCEVENT_V02_PATTERN_POS 8
#define ADCEVENT_V02_PATTERN_BIT 0x00FFFF00

#define ADCEVENT_V02_STATUS_LIN 1
#define ADCEVENT_V02_STATUS_POS 4
#define ADCEVENT_V02_STATUS_BIT 0x000000F0

#define ADCEVENT_V02_STATUS_HASDATA_BIT 0
#define ADCEVENT_V02_STATUS_DRS4COR_BIT 1
#define ADCEVENT_V02_STATUS_ZEROSUP_BIT 2

#define ADCEVENT_V02_GRMASK_LIN 1
#define ADCEVENT_V02_GRMASK_POS 0
#define ADCEVENT_V02_GRMASK_BIT 0x0000000F

#define ADCEVENT_V02_EVENTNUMBER_LIN 2
#define ADCEVENT_V02_EVENTNUMBER_POS 0
#define ADCEVENT_V02_EVENTNUMBER_BIT 0x003FFFFF

#define ADCEVENT_V02_TTAGEVENT_LIN 3
#define ADCEVENT_V02_TTAGEVENT_POS 0
#define ADCEVENT_V02_TTAGEVENT_BIT 0xFFFFFFFF

#define ADCEVENT_V02_CHMASKACTIVE_LIN 4
#define ADCEVENT_V02_CHMASKACTIVE_POS 0
#define ADCEVENT_V02_CHMASKACTIVE_BIT 0xFFFFFFFF

#define ADCEVENT_V02_CHMASKACCEPTED_LIN 5
#define ADCEVENT_V02_CHMASKACCEPTED_POS 0
#define ADCEVENT_V02_CHMASKACCEPTED_BIT 0xFFFFFFFF

// Group header structure

#define ADCEVENT_V02_GRHEAD_LEN 1

#define ADCEVENT_V02_STARTIDXCELL_LIN 0
#define ADCEVENT_V02_STARTIDXCELL_POS 16
#define ADCEVENT_V02_STARTIDXCELL_BIT 0xFFFF0000

#define ADCEVENT_V02_FREQUENCY_LIN 0
#define ADCEVENT_V02_FREQUENCY_POS 14
#define ADCEVENT_V02_FREQUENCY_BIT 0x0000C000

#define ADCEVENT_V02_TRHASDATA_LIN 0
#define ADCEVENT_V02_TRHASDATA_POS 12
#define ADCEVENT_V02_TRHASDATA_BIT 0x00003000

#define ADCEVENT_V02_GRSIZE_LIN 0
#define ADCEVENT_V02_GRSIZE_POS 0
#define ADCEVENT_V02_GRSIZE_BIT 0x00000FFF

// Group tail structure

#define ADCEVENT_V02_GRTAIL_LEN 1

#define ADCEVENT_V02_TTAGTRIG_LIN 0
#define ADCEVENT_V02_TTAGTRIG_POS 0
#define ADCEVENT_V02_TTAGTRIG_BIT 0xFFFFFFFF

  ////////////////
 // VERSION 03 //
////////////////

#define ADCEVENT_V03_FORMAT_VERSION 3

// File header structure

#define ADCEVENT_V03_FHEAD_LEN 4

#define ADCEVENT_V03_FILEINDEX_LIN 0
#define ADCEVENT_V03_FILEINDEX_POS 0
#define ADCEVENT_V03_FILEINDEX_BIT 0x0000FFFF

#define ADCEVENT_V03_RUNNUMBER_LIN 1
#define ADCEVENT_V03_RUNNUMBER_POS 0
#define ADCEVENT_V03_RUNNUMBER_BIT 0xFFFFFFFF

#define ADCEVENT_V03_BOARDSN_LIN 2
#define ADCEVENT_V03_BOARDSN_POS 0
#define ADCEVENT_V03_BOARDSN_BIT 0xFFFFFFFF

#define ADCEVENT_V03_TTAGSOF_LIN 3
#define ADCEVENT_V03_TTAGSOF_POS 0
#define ADCEVENT_V03_TTAGSOF_BIT 0xFFFFFFFF

// File tail structure

#define ADCEVENT_V03_FTAIL_LEN 4

#define ADCEVENT_V03_NEVENTS_LIN 0
#define ADCEVENT_V03_NEVENTS_POS 0
#define ADCEVENT_V03_NEVENTS_BIT 0x0FFFFFFF

#define ADCEVENT_V03_FILESIZELO_LIN 1
#define ADCEVENT_V03_FILESIZELO_POS 0
#define ADCEVENT_V03_FILESIZELO_BIT 0xFFFFFFFF

#define ADCEVENT_V03_FILESIZEHI_LIN 2
#define ADCEVENT_V03_FILESIZEHI_POS 0
#define ADCEVENT_V03_FILESIZEHI_BIT 0xFFFFFFFF

#define ADCEVENT_V03_TTAGEOF_LIN 3
#define ADCEVENT_V03_TTAGEOF_POS 0
#define ADCEVENT_V03_TTAGEOF_BIT 0xFFFFFFFF

// Event header structure

#define ADCEVENT_V03_EVENTHEAD_LEN 6

#define ADCEVENT_V03_BOARDID_LIN 1
#define ADCEVENT_V03_BOARDID_POS 24
#define ADCEVENT_V03_BOARDID_BIT 0xFF000000

#define ADCEVENT_V03_PATTERN_LIN 1
#define ADCEVENT_V03_PATTERN_POS 8
#define ADCEVENT_V03_PATTERN_BIT 0x00FFFF00

#define ADCEVENT_V03_0SUPALG_LIN 1
#define ADCEVENT_V03_0SUPALG_POS 4
#define ADCEVENT_V03_0SUPALG_BIT 0x000000F0

#define ADCEVENT_V03_GRMASK_LIN 1
#define ADCEVENT_V03_GRMASK_POS 0
#define ADCEVENT_V03_GRMASK_BIT 0x0000000F

#define ADCEVENT_V03_STATUS_LIN 2
#define ADCEVENT_V03_STATUS_POS 22
#define ADCEVENT_V03_STATUS_BIT 0xFFC00000

#define ADCEVENT_V03_STATUS_HASDATA_BIT  0
#define ADCEVENT_V03_STATUS_DRS4CORR_BIT 1
#define ADCEVENT_V03_STATUS_ZEROSUPP_BIT 2
#define ADCEVENT_V03_STATUS_BRDFAIL_BIT  3
#define ADCEVENT_V03_STATUS_AUTOPASS_BIT 4

#define ADCEVENT_V03_EVENTNUMBER_LIN 2
#define ADCEVENT_V03_EVENTNUMBER_POS 0
#define ADCEVENT_V03_EVENTNUMBER_BIT 0x003FFFFF

#define ADCEVENT_V03_TTAGEVENT_LIN 3
#define ADCEVENT_V03_TTAGEVENT_POS 0
#define ADCEVENT_V03_TTAGEVENT_BIT 0xFFFFFFFF

#define ADCEVENT_V03_CHMASKACTIVE_LIN 4
#define ADCEVENT_V03_CHMASKACTIVE_POS 0
#define ADCEVENT_V03_CHMASKACTIVE_BIT 0xFFFFFFFF

#define ADCEVENT_V03_CHMASKACCEPTED_LIN 5
#define ADCEVENT_V03_CHMASKACCEPTED_POS 0
#define ADCEVENT_V03_CHMASKACCEPTED_BIT 0xFFFFFFFF

// Group header structure

#define ADCEVENT_V03_GRHEAD_LEN 1

#define ADCEVENT_V03_STARTIDXCELL_LIN 0
#define ADCEVENT_V03_STARTIDXCELL_POS 22
#define ADCEVENT_V03_STARTIDXCELL_BIT 0xFFC00000

#define ADCEVENT_V03_FREQUENCY_LIN 0
#define ADCEVENT_V03_FREQUENCY_POS 20
#define ADCEVENT_V03_FREQUENCY_BIT 0x00300000

#define ADCEVENT_V03_TRHASDATA_LIN 0
#define ADCEVENT_V03_TRHASDATA_POS 19
#define ADCEVENT_V03_TRHASDATA_BIT 0x00008000

#define ADCEVENT_V03_GRSIZE_LIN 0
#define ADCEVENT_V03_GRSIZE_POS 0
#define ADCEVENT_V03_GRSIZE_BIT 0x00000FFF

// Group tail structure

#define ADCEVENT_V03_GRTAIL_LEN 1

#define ADCEVENT_V03_TTAGTRIG_LIN 0
#define ADCEVENT_V03_TTAGTRIG_POS 0
#define ADCEVENT_V03_TTAGTRIG_BIT 0xFFFFFFFF

#endif
