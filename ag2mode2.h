
#define MAX_SEGS   8            /* max. number of segments to take in events */
#define MAX_PARS   (8*MAX_SEGS+1)       /* max. number of parameters to fit */
#define MAX_INTPTS (2*MAX_SEGS) /* max. number of interaction points */
#define MAXDATASIZE 3000
#define PMODE 0644
#define GEB_TYPE_DECOMP         1

#define KEYLEN 20


/* key version (4,0) */

typedef struct key {
  unsigned int FrameLength; 
  unsigned int type;
  unsigned int eventNumber;
  unsigned long long int TS;
} KEY;
/* ^^^^^ also used for composite frame read */

/* Global */

typedef struct global {
 unsigned short int ID;
 unsigned short int status;
 float e0;
 float e1;
 float t0;
 float t1;
 unsigned short int PSAStatus;
 unsigned short int NbHit;
} GLOBAL;

/* we have to split the PSAHit because of the */
/* of 32 bit alignment problems, Arghhhh!!!! */
/* The structures above are OK on 32  bit platforms */
/* tl 20140627 */
/* could also have used '#pragma pack(1)' compiler option? */

typedef struct PSAHit1 {
 unsigned short int status;
 short int seg;
 short int crys;
} PSAHIT1;

typedef struct PSAHit2 {
 float X;
 float Y;
 float Z;
 float E;
 float T;
 float tCCe;
 float tCCh;
 float tSGe;
 float tSGh;
} PSAHIT2;




















/* mode2 GRETINA formats */




typedef struct DCR_INTPTS3
{
  float x, y, z, e;             /* here e refers to the fraction */
  int seg;                      /* segment hit */
  float seg_ener;               /* energy of hit segment */
} DCR_INTPTS3;


struct crys_intpts3
{
  int type;                     /* defined as abcd5678 */
  int crystal_id;
  int num;                      /* # of int pts from decomp, or # of nets on decomp error */
  float tot_e;                  /* dnl corrected */
  int core_e[4];                /* 4 raw core energies from FPGA filter (no shift) */
  long long int timestamp;
  long long trig_time;          /* not yet impl */
  float t0;
  float cfd;
  float chisq;
  float norm_chisq;
  float baseline;
  float prestep;                /* avg trace value before step */
  float poststep;               /* avg trace value following step */
  int pad;                      /* non-0 on decomp error, value gives error type */
  DCR_INTPTS3 intpts[MAX_INTPTS];
};



struct gebData
{
  int type;                     /* type of data following */
  int length;
  long long timestamp;
};

typedef struct crys_intpts3 CRYS_INTPTS3;
typedef struct gebData GEBDATA;


