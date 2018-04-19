// wMixer.c
// editied 2018 by William Bundy
//
// sts_mixer.h - v0.01
// written 2016 by Sebastian Steinhauer
// sts_mixer was committed to the public domain.
//
// Changelog:
// 	- Reformatted code
// 	- Floating point only; removed other input/output modes
// To do:
//  - SIMD-ize


enum {
	wMixer_VoiceStopped,
	wMixer_VoicePlaying,
	wMixer_VoiceStreaming
};


static 
float stmClamp(const float value, const float min, const float max)
{
	if (value < min) return min;
	else if (value > max) return max;
	else return value;
}

static 
float stmClamp1(const float sample) 
{
	if (sample < -1.0f) return -1.0f;
	else if (sample > 1.0f) return 1.0f;
	else return sample;
}


static 
float stmGetSample(wMixerSample* sample, size_t position)
{
	return ((float*)sample->data)[position];
}


static
void stmResetVoice(wMixer* mixer, const int i) 
{
	wMixerVoice*  voice = &mixer->voices[i];
	voice->state = wMixer_VoiceStopped;
	voice->sample = 0;
	voice->stream = 0;
	voice->position = voice->gain = voice->pitch = voice->pan = 0.0f;
}


static 
int stmFindFreeVoice(wMixer* mixer) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if(mixer->voices[i].state == wMixer_VoiceStopped) {
			return i;
		}
	}
	return -1;
}


void wMixerInit(wMixer* mixer, isize voiceCount, wMixerVoice* voices)
{
	mixer->frequency = 44100;
	mixer->gain = 1.0f;

	mixer->voiceCount = voiceCount;
	mixer->voices = voices;
	for (isize i = 0; i < voiceCount; ++i) {
		stmResetVoice(mixer, i);
	}
}

int wMixerGetActiveVoices(wMixer* mixer) 
{
	isize i, active;
	for (i = 0, active = 0; i < mixer->voiceCount; ++i) {
		if (mixer->voices[i].state != wMixer_VoiceStopped) {
			++active;
		}
	}
	return active;
}


int wMixerPlaySample(wMixer* mixer,
		wMixerSample* sample,
		f32 gain, f32 pitch, f32 pan)
{
	int                 i;
	wMixerVoice*  voice;

	i = stmFindFreeVoice(mixer);
	if (i >= 0) {
		voice = &mixer->voices[i];
		voice->gain = gain;
		voice->pitch = stmClamp(pitch, 0.1f, 10.0f);
		voice->pan = stmClamp(pan * 0.5f, -0.5f, 0.5f);
		voice->position = 0.0f;
		voice->sample = sample;
		voice->stream = 0;
		voice->state = wMixer_VoicePlaying;
	}
	return i;
}


int wMixerPlayStream(wMixer* mixer, wMixerStream* stream, float gain) 
{
	i32 i = stmFindFreeVoice(mixer);
	if (i >= 0) {
		wMixerVoice* voice = mixer->voices + i;
		voice->gain = gain;
		voice->position = 0.0f;
		voice->sample = 0;
		voice->stream = stream;
		voice->state = wMixer_VoiceStreaming;
	}
	return i;
}


void wMixerStopVoice(wMixer* mixer, int voice) 
{
	if (voice >= 0 && voice < mixer->voiceCount) {
		stmResetVoice(mixer, voice);
	}
}


void wMixerStopSample(wMixer* mixer, wMixerSample* sample) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if(mixer->voices[i].sample == sample) {
			stmResetVoice(mixer, i);
		}
	}
}


void wMixerStopStream(wMixer* mixer, wMixerStream* stream) 
{
	for(isize i = 0; i < mixer->voiceCount; ++i) {
		if (mixer->voices[i].stream == stream) {
			stmResetVoice(mixer, i);
		}
	}
}

void wMixerMixAudio(wMixer* mixer, void* output, u32 samples) 
{
	wMixerVoice*  voice;
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

			if (voice->state == wMixer_VoicePlaying) {
				wMixerSample* vsample = &voice->sample;
				position = (int)voice->position;

				if (position < vsample->length) {
					sample = stmClamp1(stmGetSample(vsample, position) * voice->gain);
					left += stmClamp1(sample * (0.5f - voice->pan));
					right += stmClamp1(sample * (0.5f + voice->pan));
					voice->position += (float)vsample->frequency * advance * voice->pitch;
				} else {
					stmResetVoice(mixer, i);
				}

			} else if (voice->state == wMixer_VoiceStreaming) {
				wMixerSample* vsample = &voice->stream->sample;
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
