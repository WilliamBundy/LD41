// sts_mixer.h - v0.01
// written 2016 by Sebastian Steinhauer
// editied 2018 by William Bundy for his "wpl" project
//
// Changelog:
// 	- Reformatted code
// 	- We now only do this stuff in floating point format


// A sample is a *MONO* piece of audio which is loaded fully to memory.
// It can be played with various gains, pitches and pannings.

typedef struct wplMixerSample wplMixerSample;
struct wplMixerSample
{
	//in samples (4 bytes)
	u32 length;
	u32 frequency;        
	void* data;
};


// A stream is *STEREO* audio which will be decoded/loaded as needed.
// It can be played with various gains. No panning or pitching.

// The callback which will be called when the stream needs more data.
typedef void (*wplMixerStreamProc)(wplMixerSample* sample, void* userdata);

typedef struct wplMixerStream wplMixerStream;
struct wplMixerStream 
{
	void* userdata;         
	wplMixerStreamProc callback;         

	// the current stream "sample" which holds the current piece of audio
	wplMixerSample sample;           
};


// A voice is an audio source which will be used during mixing.
// It can play nothing, a sample or a stream.
// Most of those fields are considered "private" and you should not play around with those.
typedef struct wplMixerVoice wplMixerVoice;
struct wplMixerVoice 
{
	//TODO(will) better organize this struct for padding
	i32 state;
	wplMixerSample* sample;
	wplMixerStream* stream;
	f32 position;
	f32 gain;
	f32 pitch;
	f32 pan;
};


// The mixer state.
typedef struct wplMixer wplMixer;
struct wplMixer
{
	f32 gain; 
	u32 frequency;
	
	isize voiceCount;
	wplMixerVoice* voices;
};

// "Initializes" a new sts_mixer state.
void wplMixerInit(wplMixer* mixer, unsigned int frequency, int audio_format);

// "Shutdown" the mixer state. It will simply reset all fields.
//void wplMixerShutdown(wplMixer* mixer);

// Return the number of active voices. Active voices are voices that play either a stream or a sample.
int wplMixerGetActiveVoices(wplMixer* mixer);

// Play the given sample with the gain, pitch and panning.
// Panning can be something between -1.0f (fully left) ...  +1.0f (fully right)
// Please note that pitch will be clamped so it cannot reach 0.0f (would be useless).
// Returns the number of the voice where this sample will be played or -1 if no voice was free.
int wplMixerPlaySample(wplMixer* mixer, wplMixerSample* sample, float gain, float pitch, float pan);

// Plays the given stream with the gain.
// Returns the number of the voice where this stream will be played or -1 if no voice was free.
int wplMixerPlayStream(wplMixer* mixer, wplMixerStream* stream, float gain);

// Stops voice with the given voice no. You can pass the returned number of wplMixerPlaySample / wplMixerPlayStream here.
void wplMixerStopVoice(wplMixer* mixer, int voice);

// Stops all voices playing the given sample. Useful when you want to delete the sample and make sure it is not used anymore.
void wplMixerStopSample(wplMixer* mixer, wplMixerSample* sample);

// Stops all voices playing the given stream. Useful when you want to delete the stream and make sure it is not used anymore.
void wplMixerStopStream(wplMixer* mixer, wplMixerStream* stream);

// The mixing function. You should call the function if you need to pass more audio data to the audio device.
// Typically this function is called in a separate thread or something like that.
// It will write audio data in the specified format and frequency of the mixer state.
void wplMixerMixAudio(wplMixer* mixer, void* output, unsigned int samples);


enum {
  wplMixer_VoiceStopped,
  wplMixer_VoicePlaying,
  wplMixer_VoiceStreaming
};


static float stmClamp(const float value, const float min, const float max)
{
	if (value < min) return min;
	else if (value > max) return max;
	else return value;
}

static float stmClamp1(const float sample) 
{
	if (sample < -1.0f) return -1.0f;
	else if (sample > 1.0f) return 1.0f;
	else return sample;
}


static float stmGetSample(wplMixerSample* sample, size_t position)
{
	return ((float*)sample->data)[position];
}


static void stmResetVoice(wplMixer* mixer, const int i) 
{
	wplMixerVoice*  voice = &mixer->voices[i];
	voice->state = wplMixer_VoiceStopped;
	voice->sample = 0;
	voice->stream = 0;
	voice->position = voice->gain = voice->pitch = voice->pan = 0.0f;
}


static int stmFindFreeVoice(wplMixer* mixer) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if(mixer->voices[i].state == wplMixer_VoiceStopped) {
			return i;
		}
	}
	return -1;
}


void wplMixerInit(wplMixer* mixer, isize voiceCount, wplMixerVoice* voices)
{
	mixer->frequency = 44100;
	mixer->gain = 1.0f;

	mixer->voiceCount = voiceCount;
	mixer->voices = voices;
	for (isize i = 0; i < voiceCount; ++i) {
		stmResetVoice(mixer, i);
	}
}

int wplMixerGetActiveVoices(wplMixer* mixer) 
{
	isize i, active;
	for (i = 0, active = 0; i < mixer->voiceCount; ++i) {
		if (mixer->voices[i].state != wplMixer_VoiceStopped) {
			++active;
		}
	}
	return active;
}


int wplMixerPlaySample(wplMixer* mixer,
		wplMixerSample* sample,
		f32 gain, f32 pitch, f32 pan)
{
	int                 i;
	wplMixerVoice*  voice;

	i = stmFindFreeVoice(mixer);
	if (i >= 0) {
		voice = &mixer->voices[i];
		voice->gain = gain;
		voice->pitch = stmClamp(pitch, 0.1f, 10.0f);
		voice->pan = stmClamp(pan * 0.5f, -0.5f, 0.5f);
		voice->position = 0.0f;
		voice->sample = sample;
		voice->stream = 0;
		voice->state = wplMixer_VoicePlaying;
	}
	return i;
}


int wplMixerPlayStream(wplMixer* mixer, wplMixerStream* stream, float gain) 
{
	i32 i = stmFindFreeVoice(mixer);
	if (i >= 0) {
		wplMixerVoice* voice = mixer->voices + i;
		voice->gain = gain;
		voice->position = 0.0f;
		voice->sample = 0;
		voice->stream = stream;
		voice->state = wplMixer_VoiceStreaming;
	}
	return i;
}


void wplMixerStopVoice(wplMixer* mixer, int voice) 
{
	if (voice >= 0 && voice < mixer->voiceCount) {
		stmResetVoice(mixer, voice);
	}
}


void wplMixerStopSample(wplMixer* mixer, wplMixerSample* sample) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if(mixer->voices[i].sample == sample) {
			stmResetVoice(mixer, i);
		}
	}
}


void wplMixerStopStream(wplMixer* mixer, wplMixerStream* stream) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if (mixer->voices[i].stream == stream) {
			stmResetVoice(mixer, i);
		}
	}
}

void wplMixerMixAudio(wplMixer* mixer, void* output, u32 samples) 
{
	wplMixerVoice*  voice;
	u32 i, position;
	f32 left, right, advance, sample;
	f32* out_float = (f32*)output;

	// mix all voices
	advance = 1.0f / (float)mixer->frequency;
	for (; samples > 0; --samples) {
		left = 0.0f;
		right = 0.0f;

		for (i = 0; i < mixer->voiceCount; ++i) {
			voice = mixer->voices + i;

			if (voice->state == wplMixer_VoicePlaying) {
				wplMixerSample* vsample = &voice->sample;
				position = (int)voice->position;

				if (position < vsample->length) {
					sample = stmClamp1(stmGetSample(vsample, position) * voice->gain);
					left += stmClamp1(sample * (0.5f - voice->pan));
					right += stmClamp1(sample * (0.5f + voice->pan));
					voice->position += (float)vsample->frequency * advance * voice->pitch;
				} else {
					stmResetVoice(mixer, i);
				}

			} else if (voice->state == wplMixer_VoiceStreaming) {
				wplMixerSample* vsample = &voice->stream->sample;
				position = ((int)voice->position) * 2;

				if (position >= vsample->length) {
					// buffer empty...refill
					voice->stream->callback(vsample, voice->stream->userdata);
					voice->position = 0.0f;
					position = 0;
				}

				left += stmClamp1(stmGetSample(vsample, position) * voice->gain);
				right += stmClamp1(stmGetSample(vsample, position + 1) * voice->gain);
				voice->position += (f32)vsample->frequency * advance;
			}
		}

		// write to buffer
		left = stmClamp1(left);
		right = stmClamp1(right);
		*out_float++ = left;
		*out_float++ = right;
	}
}
