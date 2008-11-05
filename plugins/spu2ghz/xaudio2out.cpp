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
#include <windows.h>
#include <xaudio2.h>
#include <strsafe.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <conio.h>

class XAudio2Mod: public SndOutModule
{
private:
	static const int PacketsPerBuffer = 1;
	static const int BufferSize = SndOutPacketSize * PacketsPerBuffer;
	static const int BufferSizeBytes = BufferSize * 2;

	s16* qbuffer;
	s32 out_num;

#define MAX_BUFFER_COUNT 3

	//--------------------------------------------------------------------------------------
	// Callback structure
	//--------------------------------------------------------------------------------------
	class StreamingVoiceContext : public IXAudio2VoiceCallback
	{
	public:
		SndBuffer* sndout;
		IXAudio2SourceVoice* pSourceVoice;

	protected:
		STDMETHOD_(void, OnVoiceProcessingPassStart) () {}
		STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32) { };
		STDMETHOD_(void, OnVoiceProcessingPassEnd) () {}
		STDMETHOD_(void, OnStreamEnd) () {}
		STDMETHOD_(void, OnBufferStart) ( void* ) {}
		STDMETHOD_(void, OnBufferEnd) ( void* context )
		{
			// All of these checks are necessary because XAudio2 is wonky shizat.
			if( pSourceVoice == NULL || context == NULL || sndout == NULL ) return;

			s16* qb = (s16*)context;

			for(int p=0; p<PacketsPerBuffer; p++, qb+=SndOutPacketSize )
				sndout->ReadSamples( qb );

			XAUDIO2_BUFFER buf = {0};
			buf.AudioBytes = BufferSizeBytes;
			buf.pAudioData=(const BYTE*)context;
			buf.pContext=context;

			pSourceVoice->SubmitSourceBuffer( &buf );
		}
		STDMETHOD_(void, OnLoopEnd) ( void* ) {}   
		STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) { };

	} voiceContext;

	IXAudio2* pXAudio2;
	IXAudio2MasteringVoice* pMasteringVoice;
	IXAudio2SourceVoice* pSourceVoice;

	WAVEFORMATEX wfx;

public:

	s32  Init(SndBuffer *sb)
	{
		HRESULT hr;

		//
		// Initialize XAudio2
		//
		CoInitializeEx( NULL, COINIT_MULTITHREADED );

		UINT32 flags = 0;
#ifdef _DEBUG
		flags |= XAUDIO2_DEBUG_ENGINE;
#endif

		if ( FAILED(hr = XAudio2Create( &pXAudio2, flags ) ) )
		{
			SysMessage( "Failed to init XAudio2 engine: %#X\n", hr );
			CoUninitialize();
			return -1;
		}

		//
		// Create a mastering voice
		//
		if ( FAILED(hr = pXAudio2->CreateMasteringVoice( &pMasteringVoice, 0, SampleRate ) ) )
		{
			SysMessage( "Failed creating mastering voice: %#X\n", hr );
			SAFE_RELEASE( pXAudio2 );
			CoUninitialize();
			return -1;
		}

		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec = SampleRate;
		wfx.nChannels=2;
		wfx.wBitsPerSample = 16;
		wfx.nBlockAlign = 2*2;
		wfx.nAvgBytesPerSec = SampleRate * wfx.nBlockAlign;
		wfx.cbSize=0;

		// Egh.. not sure if the initializer for these values should be before
		// or after CreateSourceVoice.  Some drivers could do evil things to
		// our structure whn we create the voice.

		//
		// Create an XAudio2 voice to stream this wave
		//
		if( FAILED(hr = pXAudio2->CreateSourceVoice( &pSourceVoice, &wfx,
			XAUDIO2_VOICE_NOSRC, 1.0f, &voiceContext ) ) )
		{
			SysMessage( "Error %#X creating source voice\n", hr );
			SAFE_RELEASE( pXAudio2 );
			return -1;
		}

		// See comment above.  Leave this redundant code in to protect against
		// potentially stupid drivers.
		voiceContext.pSourceVoice = pSourceVoice;
		pSourceVoice->FlushSourceBuffers();
		pSourceVoice->Start( 0, 0 );

		qbuffer = new s16[BufferSize*MAX_BUFFER_COUNT];
		ZeroMemory(qbuffer,BufferSizeBytes*MAX_BUFFER_COUNT);

		// Start two buffers.
		// Frankly two buffers is all we should ever need since the buffer fill code
		// is tied directly to the XAudio2 engine.

		{
			XAUDIO2_BUFFER buf = {0};
			buf.AudioBytes = BufferSizeBytes;
			buf.pContext=qbuffer;
			buf.pAudioData=(BYTE*)buf.pContext;
			pSourceVoice->SubmitSourceBuffer( &buf );
		}

		{
			XAUDIO2_BUFFER buf = {0};
			buf.AudioBytes = BufferSizeBytes;
			buf.pContext=&qbuffer[BufferSize];
			buf.pAudioData=(BYTE*)buf.pContext;
			pSourceVoice->SubmitSourceBuffer( &buf );
		}
		voiceContext.sndout = sb;

		return 0;
	}

	void Close()
	{
		//
		// Clean up?
		// Apparently XA2 would just rather we NOT clean up...

		/*if( pSourceVoice != NULL )
		{
			//pSourceVoice->FlushSourceBuffers();
			//pSourceVoice->Stop( 0 );
			//Sleep(10);	// give the engine some time to flush voices
			//pSourceVoice->DestroyVoice();
		}*/

		//Sleep(10);	// give the engine some more time, because I don't trust it.

		//
		// Cleanup XAudio2
		//

		// All XAudio2 interfaces are released when the engine is destroyed,
		// but being tidy never hurt.

		if( pMasteringVoice != NULL )
			pMasteringVoice->DestroyVoice();

		SAFE_RELEASE( pXAudio2 );
		SAFE_DELETE_ARRAY( qbuffer );

		pMasteringVoice = NULL;
		voiceContext.sndout = NULL;
		voiceContext.pSourceVoice = NULL;
		pSourceVoice = NULL;

		CoUninitialize();
	}

	virtual void Configure(HWND parent)
	{
	}

	virtual bool Is51Out() const { return false; }

	s32  Test() const
	{
		return 0;
	}

	int GetEmptySampleCount() const
	{
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState( &state );
		return state.SamplesPlayed & (BufferSize-1);
	}

	const char* GetIdent() const
	{
		return "xaudio2";
	}

	const char* GetLongName() const
	{
		return "XAudio 2 (Experimental)";
	}

} XA2;

SndOutModule *XAudio2Out=&XA2;
