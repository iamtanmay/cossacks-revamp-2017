// DeviceCD.cpp : implementation file
//

//#include "stdafx.h"
#include "windows.h"
#pragma pack(1)
#include "DeviceCD.h"
#include "TMixer.h"
#include <stdio.h>
#include "ResFile.h"
#include "gFile.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDeviceCD
int StartTrack = 2;
int NTracks = 19;
byte TracksMask[32] = { 2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20 };
CDeviceCD::CDeviceCD()
{
	GFILE* f = Gopen("Tracks.cd", "r");
	if (f) {
		Gscanf(f, "%d%d", &StartTrack, &NTracks);
		for (int i = 0; i < NTracks; i++)Gscanf(f, "%d", TracksMask + i);
		Gclose(f);
	};
	Open();
}

CDeviceCD::~CDeviceCD()
{
	Close();
}


// CDeviceCD message handlers
MCIDEVICEID glFDeviceID;
byte currentTrack = 2;
bool CDeviceCD::Open()
{
	/* TODO: Find out why there are 19 track according to Tracks.cd,
	   when there are only 18 tracks included in the Steam version,
	   so maybe hardcoding 18 is not needed */
	for (int i = 0; i < 18; ++i) {
		char filename[32];
		sprintf(filename, "tracks/Track %d.wav", i+1);
		char commandString[128];
		sprintf(commandString, "open \"%s\" type waveaudio alias %d", filename, TracksMask[i]);

		DWORD currentError = mciSendString(commandString, NULL, 0, NULL);
		if (currentError) {
			char errorMessage[100];
			mciGetErrorString(currentError, errorMessage, 100);
			char ccc[200];
			sprintf(ccc, "Error code: %u\nError Message: %s\nFile: %s", currentError, errorMessage, filename);
			MessageBox(NULL, ccc, "CDeviceCD::Open", 0);
		}
		FError = FError | currentError;
	}


	if (FError) {
		FOpened = FALSE;
		return FALSE;
	}

	FOpened = TRUE;
	return TRUE;
}

bool CDeviceCD::Close()
{
	if (FOpened)
	{
		/* TODO: Find out why there are 19 track according to Tracks.cd,
		   when there are only 18 tracks included in the Steam version,
		   so maybe hardcoding 18 is not needed */
		for (int i = 0; i < 18; ++i) {
			char commandString[128];
			sprintf(commandString, "close \"tracks/Track %d.wav\"", i + 1);

			FError = FError | mciSendString(commandString, NULL, 0, NULL);
		}
		return TRUE;
	}

	return FALSE;
}

bool CDeviceCD::Pause()
{
	if (FOpened)
	{
		char commandString[32];
		sprintf(commandString, "Pause %d", currentTrack);
		FError = mciSendString(commandString, NULL, 0, NULL);

		if (!FError)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

bool CDeviceCD::Resume()
{
	if (FOpened)
	{
		char commandString[32];
		sprintf(commandString, "Resume %d",currentTrack);
		FError = mciSendString(commandString, NULL, 0, NULL);

		if (!FError)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

bool CDeviceCD::Stop()
{
	if (FOpened)
	{
		char commandString[32];
		sprintf(commandString, "Stop %d", currentTrack);
		FError = mciSendString(commandString, NULL, 0, NULL);

		if (!FError)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

DWORD CDeviceCD::GetVolume()
{
	CMixer Mixer(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
		MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
		MIXERCONTROL_CONTROLTYPE_VOLUME);


	return Mixer.GetControlValue();
}

bool CDeviceCD::SetVolume(DWORD Volume)
{
	CMixer Mixer(MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
		MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,
		MIXERCONTROL_CONTROLTYPE_VOLUME);


	Mixer.SetControlValue(Volume);

	return 1;
}
extern HWND hwnd;
bool CDeviceCD::Play(DWORD Track)
{
	if (FOpened)
	{
		char commandString[32];
		currentTrack = Track;
		sprintf(commandString, "Play %d notify", currentTrack);
		FError = mciSendString(commandString, NULL, 0, hwnd);
		if (FError) {
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}
int PrevTrack3 = -1;
int PrevTrack2 = -1;
int PrevTrack1 = -1;
int NextCommand = -1;
void PlayCDTrack(int Id);
extern int srando();
void PlayRandomTrack();
LRESULT CD_MCINotify(WPARAM wFlags, LONG lDevId)
{
	if (wFlags == MCI_NOTIFY_SUCCESSFUL)
	{
		//insert here
		if (NextCommand == -1) {
			PlayRandomTrack();
		}
		else if (NextCommand >= 1000) {
			PlayCDTrack(NextCommand - 1000);
			PrevTrack1 = NextCommand - 1000;
			NextCommand = -1;
		}
		else {
			PlayCDTrack(NextCommand);
			PrevTrack1 = NextCommand;
		};
		return 1;
	}
	return 0;

}
CDeviceCD CDPLAY;
void PlayCDTrack(int Id) {
	CDPLAY.Play(Id);
};
extern int CurrentNation;
extern int PlayMode;
void PlayRandomTrack()
{
	if (PlayMode == 1 && CurrentNation != -1) {
		PlayCDTrack(TracksMask[CurrentNation]);
		return;
	};
	int Track = -1;
	do {
		Track = (((GetTickCount() & 4095)*NTracks) >> 12) + StartTrack;
		if (Track == PrevTrack1 || Track == PrevTrack2 || Track == PrevTrack3)Track = -1;
	} while (Track == -1);
	PrevTrack3 = PrevTrack2;
	PrevTrack2 = PrevTrack1;
	PrevTrack1 = Track;
	PlayCDTrack(Track);
}

void StopPlayCD()
{
	CDPLAY.Stop();
}

// TODO: make volume settings work
int GetCDVolume()
{
	return (int(CDPLAY.GetVolume()) * 100) >> 16;
}

// TODO: make volume settings work, find out why it is called even if nothing is changed
void SetCDVolume(int Vol)
{
	CDPLAY.SetVolume(((Vol) * 65535) / 100);
}