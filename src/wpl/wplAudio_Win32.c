
/* Thanks to StrangeZak for the base for this code 
 * It's heavily modified, but I wouldn't have 
 * gotten it done on time without it.
 * */

static 
void wInitWasapi(void)
{
	win32_audio *Result = 0;
	Result = (win32_audio *) Win32AllocateMemory(sizeof(win32_audio));
	ZeroMemory(Result, sizeof(win32_audio));

	IMMDeviceEnumerator *DeviceEnumerator = 0;
	HRESULT ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), 0, 
			CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
			(void **)&DeviceEnumerator);
	if(ret != S_OK) {
		wLogError(0, "Error: MMDeviceEnumerator creation failed! %x\n", ret);
		return;
	}

	IMMDevice *Device = 0;
	ret = DeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device);
	if(ret != S_OK) {
		wLogError(0, "GetDefaultAudioEndpoint failed! %x\n", ret);
		return;
	}

	ret = Device->Activate(
			__uuidof(IAudioClient), 
			CLSCTX_ALL, 
			0, 
			(void **)&Result->WriteAudioClient);

	if(ret != S_OK) {
		wLogError(0, "Audio device activation failed! %x\n", ret);
		return;
	}

	// NOTE(zak): This is just a default normal PCM format
	// nothing special really. We might wanna change the SamplesPerSec,
	// and BitsPerSample when we get real sound effects and music
	// into the game
	WAVEFORMATEX WaveFormat = {0};
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.nChannels = 2;
	WaveFormat.nSamplesPerSec = 44100;
	WaveFormat.wBitsPerSample = 16;
	WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;

	Result->SamplesPerSecond = WaveFormat.nSamplesPerSec;
	Result->ChannelCount = WaveFormat.nChannels;

	REFERENCE_TIME TimeRequested = 10000000;
	ret = Result->WriteAudioClient->Initialize(
			AUDCLNT_SHAREMODE_SHARED,
			AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
			AUDCLNT_STREAMFLAGS_RATEADJUST | 
			AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, 

			(WaveFormat.nSamplesPerSec * (sizeof(s16) * 2)),
			0,
			&WaveFormat,
			0);

	if(ret != S_OK) {
		wLogError(0, "Error: WriteAudioClient init failed! %x\n", ret);
		return;
	}

	ret = Result->WriteAudioClient->GetService(
			__uuidof(IAudioRenderClient),
			(void **)&Result->AudioRenderClient);

	if(ret != S_OK) {
		wLogError(0, "Error: GetService for Audio client failed! %x\n", ret);                        
		return;
	}

	SAFE_RELEASE(DeviceEnumerator);

	CreateThread(0, 0, Win32AudioThreadProc, Result, 0, 0);                            
}

static 
u32 wWasapiAudioThreadProc(void *Parameter)
{
	win32_audio *Audio = (win32_audio *)Parameter;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);    

	HANDLE BufferReadyEvent = CreateEvent(0, 0, 0, 0);
	HRESULT ret = Audio->WriteAudioClient->SetEventHandle(BufferReadyEvent);
	if(ret != S_OK) {
		wLogError(0, "Error: SetEventHandle failed for audio client! %x\n", ret);
		return 0;
	}

	ret = Audio->WriteAudioClient->GetBufferSize(&Audio->BufferSize);
	if(ret != S_OK) {
		wLogError(0, "Error: Failed to get buffer size for audio! %x\n", ret);
		return 0;

	}

	ret = Audio->WriteAudioClient->Start();
	if(ret != S_OK) {
		wLogError(0, "Error: Failed to start audio client! %x\n", ret);
		return 0;
	}

	while(1) {
		if(WaitForSingleObject(BufferReadyEvent, INFINITE) == WAIT_OBJECT_0) {
			u32 PaddingFrameCount;
			ret = Audio->WriteAudioClient->GetCurrentPadding(&PaddingFrameCount); 
			if(!SUCCEEDED(ret)) continue;
			i32 WriteAmmount = Audio->BufferSize - PaddingFrameCount;

			if(WriteAmmount <= 0) {
				continue;
			}

			BYTE *Buffer = 0;
			ret = Audio->AudioRenderClient->GetBuffer(WriteAmmount, &Buffer); 
			if(!SUCCEEDED(ret)) continue;

			ZeroMemory(Buffer, WriteAmmount * sizeof(s16) * Audio->ChannelCount);
			game_audio_buffer AudioBuffer = {0};
			AudioBuffer.Buffer = Buffer;
			AudioBuffer.BufferSize = WriteAmmount;
			AudioBuffer.ChannelCount = Audio->ChannelCount;
			AudioBuffer.SamplesPerSecond = Audio->SamplesPerSecond;  

			if(Game.GenerateAudioSamples) {
				if(GameMemory.PermanentStorage && GameMemory.TransientStorage) {
					Game.GenerateAudioSamples(&GameMemory, &AudioBuffer);
				}
			}

			Audio->AudioRenderClient->ReleaseBuffer(WriteAmmount, 0);
		}
	}
	return 0;
}


static 
void OutputPlayingSound(game_state *GameState, game_audio_buffer *AudioBuffer)
{        
	if(!GameState->IsInitialized) return;

	local_persist memory_arena *Arena;
	if(!Arena)
	{
		Arena = &GameState->WorldArena;
	}

	StartTemporarySection(Arena);
	r32 *RealChannel0 = (r32 *)Platform.AllocateMemory(AudioBuffer->BufferSize * sizeof(r32));
	r32 *RealChannel1 = (r32 *)Platform.AllocateMemory(AudioBuffer->BufferSize * sizeof(r32));

	// Clear mixer channels        
	{
		r32 *Dest0 = RealChannel0;
		r32 *Dest1 = RealChannel1;

		for(int SampleIndex = 0;
				SampleIndex < AudioBuffer->BufferSize;
				++SampleIndex)
		{
			*Dest0++ = 0.0f;
			*Dest1++ = 0.0f;
		}        
	}    

	b32 SoundFinished = false;

	// Sum all the sounds
	for(playing_sound **PlayingSoundPtr = &GameState->FirstPlayingSound; *PlayingSoundPtr;) {
		playing_sound *PlayingSound = *PlayingSoundPtr;

		if(!PlayingSound->Sound) 
		{
			// TODO(zak) Handle Stereo
			r32 Volume0 = PlayingSound->Volume[0];
			r32 Volume1 = PlayingSound->Volume[1];            
			r32 *Dest0 = RealChannel0;
			r32 *Dest1 = RealChannel1;

			Assert(PlayingSound->SamplesPlayed >= 0);

			u32 SamplesToMix = AudioBuffer->BufferSize;
			u32 SampleRemainingInSound = PlayingSound->Sound->SampleCount - PlayingSound->SamplesPlayed;

			if(SamplesToMix > SampleRemainingInSound)
			{
				SamplesToMix = SampleRemainingInSound;
			}

			for(u32 SampleIndex = PlayingSound->SamplesPlayed;
					SampleIndex <  (PlayingSound->SamplesPlayed + SamplesToMix);
					++SampleIndex)
			{
				r32 SampleValue = PlayingSound->Sound->Samples[0][SampleIndex];
				*Dest0++ += Volume0 * SampleValue;
				*Dest1++ += Volume1 * SampleValue;
			}

			SoundFinished = ((u32)PlayingSound->SamplesPlayed == PlayingSound->Sound->SampleCount);

			PlayingSound->SamplesPlayed += SamplesToMix;
		}
		else
		{
			// TODO(zak): We should never just call LoadWAV
			// for when we support multiple audio formats
			// TODO(zak)" Move this allocation outside of this function...
			PlayingSound->Sound = (loaded_sound *)Platform.AllocateMemory(sizeof(loaded_sound));
			*PlayingSound->Sound = LoadWAV(PlayingSound->FilePath);
		}


		if(SoundFinished)
		{
			// put freed sounds into another linked list
			if(PlayingSound->ShouldLoop)
			{
				PlayingSound->SamplesPlayed = 0;
			}
			else
			{
				*PlayingSoundPtr = PlayingSound->Next;
				PlayingSound->Next = GameState->FirstFreePlayingSound;
				GameState->FirstFreePlayingSound = PlayingSound;
			}
		}
		else
		{
			PlayingSoundPtr = &PlayingSound->Next;
		}
	}    

	// Convert to 16 bit
	{
		r32 *Source0 = RealChannel0;
		r32 *Source1 = RealChannel1;

			s16 *SampleOut = (s16 *)AudioBuffer->Buffer;    
			for(int SampleIndex = 0;
					SampleIndex < AudioBuffer->BufferSize;
					++SampleIndex)
			{        
				*SampleOut++ = (s16)(*Source0++ + 0.5f);
				*SampleOut++ = (s16)(*Source1++ + 0.5f);        
			}        

			EndTemporarySection(Arena);
		}

		Platform.FreeMemory(RealChannel1);
		Platform.FreeMemory(RealChannel0);
	}
}
