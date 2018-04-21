
typedef sts_mixer_sample_t MixerSample;
typedef sts_mixer_t StsMixer;
typedef f32 AudioType;
i32 AudioForSDL = AUDIO_F32SYS;
i32 AudioForSTS = STS_MIXER_SAMPLE_FORMAT_FLOAT;
#define drWavReadProc drwav_read_f32
f32 globalVolume = 0.5;

SDL_AudioDeviceID globalAudioDevice;

static 
void audioCallback(void* user, u8* stream, i32 len)
{
	sts_mixer_mix_audio((StsMixer*)user, stream, len / (sizeof(AudioType) * 2));
}

static 
void audioRefillStream(MixerSample* sample, void* user)
{

}

StsMixer* globalMixer;
void wbInitAudio(MemoryArena* arena)
{
	globalMixer = arenaPush(arena, sizeof(StsMixer));
	sts_mixer_init(globalMixer, 44100, AudioForSTS);

	SDL_AudioSpec want = {0}, have = {0};
	want.format = AUDIO_F32SYS;
	want.freq = 44100;
	want.channels = 2;
	want.userdata = globalMixer;
	want.samples = 4096;
	want.callback = audioCallback;
	globalAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	//Actually sets the device to play
	SDL_PauseAudioDevice(globalAudioDevice, 0);
}

#define GlobalSampleCapacity 256
MixerSample globalSamples[GlobalSampleCapacity];
AudioType* globalSampleData[GlobalSampleCapacity];
i32 globalSampleCount = 0;
MixerSample* loadSample(string filename, MemoryArena* arena)
{
	if(globalSampleCount >= GlobalSampleCapacity) {
		fprintf(stderr, "Error: ran out of sample storage.\n");
		return NULL;
	}
	char buf[4096];
	i32 buflen = snprintf(buf, 4096, "%s%s", globalBasePath, filename);
	drwav wav;
	if(!drwav_init_file(&wav, buf)) {
		fprintf(stderr, "Error: could not find audio sample %s\n", buf);
		return NULL;
	}

	i32 index = globalSampleCount++;
	AudioType* data = arenaPush(arena, 
		sizeof(AudioType) * wav.totalSampleCount);
	usize samplesDecoded = drWavReadProc(&wav, wav.totalSampleCount, data);
	drwav_uninit(&wav);
	MixerSample s = {
		(u32)samplesDecoded,
		44100,
		AudioForSTS,
		data
	};

	globalSampleData[index] = data;
	globalSamples[index] = s;
	return globalSamples + index;
}

void playSample(MixerSample* sample, f32 gain, f32 pitch, f32 pan)
{
	if(gain > 5) gain = 5;
	//printf("Sfx Gain(%.2f) Pitch(%.2f) Pan(%.2f)\n", gain, pitch, pan);
	SDL_LockAudioDevice(globalAudioDevice);
	i32 ret = sts_mixer_play_sample(globalMixer, sample, gain * globalVolume, pitch, pan);
	SDL_UnlockAudioDevice(globalAudioDevice);
	//printf("played on %d\n", ret);
}
