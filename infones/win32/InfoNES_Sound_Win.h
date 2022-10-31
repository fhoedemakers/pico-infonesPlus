/*===================================================================*/
/*                                                                   */
/*  InfoNES_Sound_Win.h : Implementation of DIRSOUND Class           */
/*                                                                   */
/*  2000/05/12  InfoNES Project                                      */
/*                                                                   */
/*===================================================================*/

#ifndef ds_INCLUDE
#define ds_INCLUDE

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include <mmsystem.h>
#include "dsound.h"
#include "../InfoNES_pAPU.h"

/*-------------------------------------------------------------------*/
/*  Constants for DirectSound                                        */
/*-------------------------------------------------------------------*/

#define ds_NUMCHANNELS      8
#define ds_CHANSPERSAMPLE   1
#define ds_BITSPERSAMPLE	  8
#define Loops               20

#if ( pAPU_QUALITY == 1 )
#define ds_SAMPLERATE 		  11025
#define rec_freq            183
#elif ( pAPU_QUALITY == 2 )
#define ds_SAMPLERATE 		  22050
#define rec_freq            367
#else
#define ds_SAMPLERATE 		  44100
#define rec_freq            735
#endif

/*-------------------------------------------------------------------*/
/*  Class Definitions                                                */
/*-------------------------------------------------------------------*/
class DIRSOUND
{
	public:
    /*-------------------------------------------------------------------*/
    /*  Constructor/Destructor                                           */
    /*-------------------------------------------------------------------*/
		DIRSOUND(HWND hwnd);
		~DIRSOUND();
    
    /*-------------------------------------------------------------------*/
    /*  Global Functions for NES Emulation                               */
    /*-------------------------------------------------------------------*/
    BOOL SoundOutput( int samples, BYTE* wave );
		void SoundClose( void );
		BOOL SoundOpen( int samples_per_sync, int sample_rate );
    BOOL SoundMute( BOOL flag );

    /*-------------------------------------------------------------------*/
    /*  Global Functions                                                 */
    /*-------------------------------------------------------------------*/
		void UnLoadWave(WORD channel);
		void Start(WORD channel, BOOL looping);
		void Stop(WORD channel);

  private:
    /*-------------------------------------------------------------------*/
    /*  Local Functions                                                  */
    /*-------------------------------------------------------------------*/
	  WORD AllocChannel(void);
		void CreateBuffer(WORD channel);
		void DestroyBuffer(WORD channel);
		void FillBuffer(WORD channel);

    /*-------------------------------------------------------------------*/
    /*  Local Variables                                                  */
    /*-------------------------------------------------------------------*/
		HWND					hwnd; 			/* Window handle to application */
	  LPDIRECTSOUND lpdirsnd;

		/* Used for management of each sound channel  */
		BYTE								*sound[ds_NUMCHANNELS];
		DWORD 							 len[ds_NUMCHANNELS];
		LPDIRECTSOUNDBUFFER  lpdsb[ds_NUMCHANNELS];

    /* Used for Sound Buffer */
    WORD                iCnt;
    WORD                ch1;
};
#endif /* ds_INCLUDE */
