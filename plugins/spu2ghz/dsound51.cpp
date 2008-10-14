//GiGaHeRz's SPU2 Driver
//Copyright (c) 2003-2008, David Quintana <gigaherz@gmail.com>
//
//This library is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, write to the Free Software
//Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#define _WIN32_DCOM
#include "spu2.h"
#include "dialogs.h"
#include <initguid.h>
#include <windows.h>
#include <dsound.h>
#include <strsafe.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <conio.h>
#include <assert.h>
#include <Mmreg.h>
#include <ks.h>
#include <KSMEDIA.H>

#include "lowpass.h"

#define ENABLE_DPLII

struct ds_device_data {
	char name[256];
	GUID guid;
	bool hasGuid;
} extern devices[];
extern int ndevs;
extern GUID DevGuid;
extern bool haveGuid;

extern HRESULT GUIDFromString(const char *str, LPGUID guid);

class DSound51: public SndOutModule
{
private:
#	define PI 3.14159265f

#	define BufferSize      (CurBufferSize*6)
#	define BufferSizeBytes (BufferSize<<1)
#	define TBufferSize     (BufferSize*CurBufferCount)

	s32* tbuffer;

	FILE *voicelog;

	int channel;

	bool dsound_running;
	HANDLE thread;
	DWORD tid;

	#define MAX_BUFFER_COUNT 3

	IDirectSound8* dsound;
	IDirectSoundBuffer8* buffer;
	IDirectSoundNotify8* buffer_notify;
	HANDLE buffer_events[MAX_BUFFER_COUNT];

	WAVEFORMATEXTENSIBLE wfx;

	SndBuffer *buff;

	s32 LAccum;
	s32 RAccum;
	s32 ANum;

	s32 LBuff[128];
	s32 RBuff[128];

	LPF_data lpf_l;
	LPF_data lpf_r;

	void Convert(s16 *obuffer, s32 ValL, s32 ValR)
	{
		static u8 bufdone=1;
		static s32 Gfl=0,Gfr=0;

		static s32 spdif_data[6];
		static s32 LMax=0,RMax=0;

		if(PlayMode&4)
		{
			spdif_get_samples(spdif_data); 
		}
		else
		{
			spdif_data[0]=0;
			spdif_data[1]=0;
			spdif_data[2]=0;
			spdif_data[3]=0;
			spdif_data[4]=0;
			spdif_data[5]=0;
		}

#ifdef ENABLE_DPLII
#ifdef USE_AVERAGING
		LAccum+=abs(ValL);
		RAccum+=abs(ValR);
		ANum++;
		
		if(ANum>=512)
		{
			LMax=0;RMax=0;

			LAccum/=ANum;
			RAccum/=ANum;
			ANum=0;

			for(int i=0;i<127;i++)
			{
				LMax+=LBuff[i];
				RMax+=RBuff[i];
				LBuff[i]=LBuff[i+1];
				RBuff[i]=RBuff[i+1];
			}
			LBuff[127]=LAccum;
			RBuff[127]=RAccum;
			LMax+=LAccum;
			RMax+=RAccum;

			s32 TL = (LMax>>15)+1;
			s32 TR = (RMax>>15)+1;

			Gfl=(RMax)/(TL);
			Gfr=(LMax)/(TR);

			if(Gfl>255) Gfl=255;
			if(Gfr>255) Gfr=255;
		}
#else
		
		if((ValL>>8)>LMax) LMax = (ValL>>8);
		if(-(ValL>>8)>LMax) LMax = -(ValL>>8);
		if((ValR>>8)>RMax) RMax = (ValR>>8);
		if(-(ValR>>8)>RMax) RMax = -(ValR>>8);
		ANum++;
		if(ANum>=128)
		{
			ANum=0;
			LAccum = (LAccum * 224 + LMax * 32)>>8;
			RAccum = (RAccum * 224 + RMax * 32)>>8;

			LMax=0;
			RMax=0;

			if(LAccum<1) LAccum=1;
			if(RAccum<1) RAccum=1;

			Gfl=(RAccum)*255/(LAccum);
			Gfr=(LAccum)*255/(RAccum);

			int gMax = max(Gfl,Gfr);

			Gfl=Gfl*255/gMax;
			Gfr=Gfr*255/gMax;

			if(Gfl>255) Gfl=255;
			if(Gfr>255) Gfr=255;
			if(Gfl<1) Gfl=1;
			if(Gfr<1) Gfr=1;
		}
#endif

		s32 L,R,C,LFE,SL,SR,LL,LR;

		extern double pow_2_31;
		LL = (s32)(LPF(&lpf_l,(ValL>>4)/pow_2_31)*pow_2_31);
		LR = (s32)(LPF(&lpf_r,(ValR>>4)/pow_2_31)*pow_2_31);
		LFE = (LL + LR)>>4;

		C=(ValL+ValR)>>1; //16.8

		ValL-=C;//16.8
		ValR-=C;//16.8

		L=ValL>>8; //16.0
		R=ValR>>8; //16.0
		C=C>>8;    //16.0

		s32 VL=(ValL>>4) * Gfl; //16.12
		s32 VR=(ValR>>4) * Gfr;

		SL=(VL/209 - VR/148)>>4; //16.0 (?)
		SR=(VL/148 - VR/209)>>4; //16.0 (?)

		// increase surround stereo separation
		int SC = (SL+SR)>>1; //16.0
		int SLd = SL - SC;   //16.0
		int SRd = SL - SC;   //16.0

		SL = (SLd * 209 + SC * 148)>>8; //16.0
		SR = (SRd * 209 + SC * 148)>>8; //16.0

		obuffer[0]=spdif_data[0] + (((L   * GainL  ) + (C * AddCLR))>>8);
		obuffer[1]=spdif_data[1] + (((R   * GainR  ) + (C * AddCLR))>>8);
		obuffer[2]=spdif_data[2] + (((C   * GainC  ))>>8);
		obuffer[3]=spdif_data[3] + (((LFE * GainLFE))>>8);
		obuffer[4]=spdif_data[4] + (((SL  * GainSL ))>>8);
		obuffer[5]=spdif_data[5] + (((SR  * GainSR ))>>8);
#else
		obuffer[0]=spdif_data[0]+(ValL>>8);
		obuffer[1]=spdif_data[1]+(ValR>>8);
		obuffer[2]=spdif_data[2];
		obuffer[3]=spdif_data[3];
		obuffer[4]=spdif_data[4];
		obuffer[5]=spdif_data[5];
#endif
	}

#	define STRFY(x) #x

#	define verifyc(x) if(Verifyc(x,STRFY(x))) return -1;

	static DWORD CALLBACK RThread(DSound51*obj)
	{
		return obj->Thread();
	}

	int __forceinline Verifyc(HRESULT hr,const char* fn)
	{
		if(FAILED(hr))
		{
			SysMessage("ERROR: Call to %s Failed.",fn);
			return -1;
		}
		return 0;
	}

	DWORD Thread()
	{
		while( dsound_running )
		{
			u32 rv = WaitForMultipleObjects(MAX_BUFFER_COUNT,buffer_events,FALSE,400);
	 
			LPVOID p1,p2;
			DWORD s1,s2;
	 
			for(int i=0;i<MAX_BUFFER_COUNT;i++)
			{
				if (rv==WAIT_OBJECT_0+i)
				{
					u32 poffset=BufferSizeBytes * i;

					buff->ReadSamples(tbuffer,CurBufferSize*2);

					verifyc(buffer->Lock(poffset,BufferSizeBytes,&p1,&s1,&p2,&s2,0));
					s16 *t = (s16*)p1;
					s32 *s = tbuffer;
					for(int j=0;j<CurBufferSize;j++)
					{
						// DPL2 code here: inputs s[0] and s[1]. outputs t[0] to t[5]
						Convert(t,s[0],s[1]);

						t+=6;
						s+=2;
					}
					verifyc(buffer->Unlock(p1,s1,p2,s2));
				}
			}
		}
		return 0;
	}

public:
	s32  Init(SndBuffer *sb)
	{
		if(CurBufferSize<1024) CurBufferSize=1024;

		buff = sb;

		//
		// Initialize DSound
		//
		verifyc(DirectSoundCreate8(NULL,&dsound,NULL));
	 
		verifyc(dsound->SetCooperativeLevel(GetDesktopWindow(),DSSCL_PRIORITY));
		IDirectSoundBuffer* buffer_;
 		DSBUFFERDESC desc; 
	 
		// Set up WAV format structure. 
	 
		memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
		wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfx.Format.nSamplesPerSec = SampleRate;
		wfx.Format.nChannels=6;
		wfx.Format.wBitsPerSample = 16;
		wfx.Format.nBlockAlign = wfx.Format.nChannels*wfx.Format.wBitsPerSample/8;
		wfx.Format.nAvgBytesPerSec = SampleRate * wfx.Format.nBlockAlign;
		wfx.Format.cbSize=22;
		wfx.Samples.wValidBitsPerSample=0;
		wfx.dwChannelMask=SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
		wfx.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

		verifyc(dsound->SetSpeakerConfig(DSSPEAKER_5POINT1));

		// Set up DSBUFFERDESC structure. 
	 
		memset(&desc, 0, sizeof(DSBUFFERDESC)); 
		desc.dwSize = sizeof(DSBUFFERDESC); 
		desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY;// _CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; 
		desc.dwBufferBytes = BufferSizeBytes * MAX_BUFFER_COUNT; 
		desc.lpwfxFormat = &wfx.Format; 
	 
		desc.dwFlags |=DSBCAPS_LOCSOFTWARE;
		desc.dwFlags|=DSBCAPS_GLOBALFOCUS;
	 
		verifyc(dsound->CreateSoundBuffer(&desc,&buffer_,0));
		verifyc(buffer_->QueryInterface(IID_IDirectSoundBuffer8,(void**)&buffer));
		buffer_->Release();
	 
		verifyc(buffer->QueryInterface(IID_IDirectSoundNotify8,(void**)&buffer_notify));

		DSBPOSITIONNOTIFY not[MAX_BUFFER_COUNT];
	 
		for(int i=0;i<MAX_BUFFER_COUNT;i++)
		{
			buffer_events[i]=CreateEvent(NULL,FALSE,FALSE,NULL);
			not[i].dwOffset=(wfx.Format.nBlockAlign*10 + BufferSizeBytes*(i+1))%desc.dwBufferBytes;
			not[i].hEventNotify=buffer_events[i];
		}
	 
		buffer_notify->SetNotificationPositions(MAX_BUFFER_COUNT,not);
	 
		LPVOID p1=0,p2=0;
		DWORD s1=0,s2=0;
	 
		verifyc(buffer->Lock(0,desc.dwBufferBytes,&p1,&s1,&p2,&s2,0));
		assert(p2==0);
		memset(p1,0,s1);
		verifyc(buffer->Unlock(p1,s1,p2,s2));
	 
		LPF_init(&lpf_l,LowpassLFE,SampleRate);
		LPF_init(&lpf_r,LowpassLFE,SampleRate);

		//Play the buffer !
		verifyc(buffer->Play(0,0,DSBPLAY_LOOPING));

		tbuffer = new s32[BufferSize];

		// Start Thread
		dsound_running=true;
		thread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RThread,this,0,&tid);
		SetThreadPriority(thread,THREAD_PRIORITY_TIME_CRITICAL);

		return 0;
	}

	void Close()
	{
		// Stop Thread
		fprintf(stderr," * SPU2: Waiting for DSound thread to finish...");
		dsound_running=false;
			
		WaitForSingleObject(thread,INFINITE);
		CloseHandle(thread);

		fprintf(stderr," Done.\n");

		//
		// Clean up
		//
		buffer->Stop();
	 
		for(int i=0;i<MAX_BUFFER_COUNT;i++)
			CloseHandle(buffer_events[i]);
	 
		buffer_notify->Release();
		buffer->Release();
		dsound->Release();

		delete tbuffer;
	}

private:

	static BOOL CALLBACK DSEnumCallback( LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext )
	{
		//strcpy(devices[ndevs].name,lpcstrDescription);
		_snprintf_s(devices[ndevs].name,256,255,"%s",lpcstrDescription);

		if(lpGuid)
		{
			devices[ndevs].guid=*lpGuid;
			devices[ndevs].hasGuid = true;
		}
		else
		{
		devices[ndevs].hasGuid = false;
		}
		ndevs++;

		if(ndevs<32) return TRUE;
		return FALSE;
	}

	static BOOL DoHandleScrollMessage(WPARAM wParam, LPARAM lParam, int vmin, int vmax, HWND hwndDisplay)
	{
		int wmId    = LOWORD(wParam); 
		int wmEvent = HIWORD(wParam); 
		static char temp[1024];
		switch(wmId) {
			//case TB_ENDTRACK:
			//case TB_THUMBPOSITION:
			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN:
				wmEvent=(int)SendMessage((HWND)lParam,TBM_GETPOS,0,0);
			case TB_THUMBTRACK:
				if(wmEvent<vmin) wmEvent=vmin;
				if(wmEvent>vmax) wmEvent=vmax;
				SendMessage((HWND)lParam,TBM_SETPOS,TRUE,wmEvent);
				sprintf(temp,"%d",vmax-wmEvent);
				SetWindowText(hwndDisplay,temp);
				break;
			default:
				return FALSE;
		}
		return TRUE;
	}
#define HANDLE_SCROLL_MESSAGE(idc,vmin,vmax,idcDisplay) \
			if((HWND)lParam == GetDlgItem(hWnd,idc)) return DoHandleScrollMessage(wParam,lParam,vmin,vmax,GetDlgItem(hWnd,idcDisplay))

	static BOOL CALLBACK ConfigProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
	{
		int wmId,wmEvent;
		int tSel=0;

		switch(uMsg)
		{
			case WM_INITDIALOG:

				haveGuid = ! FAILED(GUIDFromString(DSoundDevice,&DevGuid));
				SendMessage(GetDlgItem(hWnd,IDC_DS_DEVICE),CB_RESETCONTENT,0,0); 

				ndevs=0;
				DirectSoundEnumerate(DSEnumCallback,NULL);

				tSel=-1;
				for(int i=0;i<ndevs;i++)
				{
					SendMessage(GetDlgItem(hWnd,IDC_DS_DEVICE),CB_ADDSTRING,0,(LPARAM)devices[i].name);
					if(haveGuid && IsEqualGUID(devices[i].guid,DevGuid))
					{
						tSel=i;
					}
				}

				if(tSel>=0)
				{
					SendMessage(GetDlgItem(hWnd,IDC_DS_DEVICE),CB_SETCURSEL,tSel,0);
				}

				INIT_SLIDER(IDC_SLIDER1,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER2,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER3,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER4,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER5,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER6,0,512,64,16,8);
				INIT_SLIDER(IDC_SLIDER7,0,512,64,16,8);

				break;
			case WM_COMMAND:
				wmId    = LOWORD(wParam); 
				wmEvent = HIWORD(wParam); 
				// Parse the menu selections:
				switch (wmId)
				{
					case IDOK:
						{
							int i = (int)SendMessage(GetDlgItem(hWnd,IDC_DS_DEVICE),CB_GETCURSEL,0,0);
							
							if(!devices[i].hasGuid)
							{
								DSoundDevice[0]=0; // clear device name to ""
							}
							else
							{
								sprintf_s(DSoundDevice,256,"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
									devices[i].guid.Data1,
									devices[i].guid.Data2,
									devices[i].guid.Data3,
									devices[i].guid.Data4[0],
									devices[i].guid.Data4[1],
									devices[i].guid.Data4[2],
									devices[i].guid.Data4[3],
									devices[i].guid.Data4[4],
									devices[i].guid.Data4[5],
									devices[i].guid.Data4[6],
									devices[i].guid.Data4[7]
									);
							}
						}
						EndDialog(hWnd,0);
						break;
					case IDCANCEL:
						EndDialog(hWnd,0);
						break;
					default:
						return FALSE;
				}
				break;
			case WM_VSCROLL:
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER1,0,512,IDC_EDIT1);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER2,0,512,IDC_EDIT2);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER3,0,512,IDC_EDIT3);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER4,0,512,IDC_EDIT4);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER5,0,512,IDC_EDIT5);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER6,0,512,IDC_EDIT6);
				HANDLE_SCROLL_MESSAGE(IDC_SLIDER7,0,512,IDC_EDIT7);

			default:
				return FALSE;
		}
		return TRUE;
	}

public:
	virtual void Configure(HWND parent)
	{
		INT_PTR ret;
		ret=DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_DSOUND51),GetActiveWindow(),(DLGPROC)ConfigProc,1);
		if(ret==-1)
		{
			MessageBoxEx(GetActiveWindow(),"Error Opening the config dialog.","OMG ERROR!",MB_OK,0);
			return;
		}
	}

	s32  Test()
	{
		return 0;
	}

	bool Is51Out() { return true; }

} DS51;

SndOutModule *DSound51Out=&DS51;