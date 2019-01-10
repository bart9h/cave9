/*
	This file is part of cave9.

	cave9 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cave9 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cave9.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <SDL.h>
#include <math.h>
#include <stdlib.h>
#include "audio.h"
#include "util.h"

static short int audio_soft_clamp(float v)
{
	float hard = 32767;
	float soft = 30000;
	float abs = fabs(v);
	float sign = v < 0 ? -1 : 1;
	return abs < soft ? v : 
		sign * ( soft + (hard-soft)*log(1-1/(1+abs-soft)) );
}

static float audio_index_value(Wave *wave, float on)
{
	if(!wave->enabled)
		return 0;
	unsigned int samples = wave->size/2; // 16bit
	float w2 = fmodf(wave->index, 1), w1 = 1 - w2;
	int i1 = floorf(wave->index); 
	int i2 = fmodf(wave->index + 1, samples);

	float v =
			(w1 * wave->data[i1] + w2 * wave->data[i2])
			* wave->volume * wave->max;
	wave->index = fmodf(wave->index + wave->step, samples);

	wave->volume = wave->volume * (1-wave->decay) + wave->decay * on;

	return v;
}

static void audio_mix(void *data, Uint8 *stream, int len)
{
        Audio *audio = (Audio*)data;

		// step (frequency)
        float max_vel[3] = { MAX_VEL_X, MAX_VEL_Y, MAX_VEL_Z };
        float intensity = log(1+
                10000*(1-CLAMP(audio->ship->dist / MIN_CAVE_RADIUS,0,1)) +
                10000*MIN(1, (LEN(audio->ship->vel)-MAX_VEL_Z) / (LEN(max_vel)-MAX_VEL_Z))
                ) / log(1+20000);
        
		audio->back.step =
			.2 +  .6 * intensity;

        audio->left.step = 
        audio->right.step = 
			1.0 + 2.0 * intensity;

		// volume
		float collision = ship_hit(audio->ship);
		if(audio->ship->dist == -1) {
			if(audio->crash.volume == 0) {
				audio->crash.volume = 1;
				audio->hit.volume = 0;
			}
		} else {
			if(collision > audio->hit.volume)
				audio->hit.volume = collision;
		}


        unsigned int buffer_samples = len/2; // 16bit
        signed short *buffer = (signed short *)stream;
        for(unsigned i = 0; i < buffer_samples; ) {
                float both  = audio_index_value(&audio->back,  0) +
                              audio_index_value(&audio->hit,   0) +
                              audio_index_value(&audio->crash, 0);
                float left  = audio_index_value(&audio->left,  audio->ship->lefton || audio->ship->upon || audio->ship->downon);
                float right = audio_index_value(&audio->right, audio->ship->righton || audio->ship->upon || audio->ship->downon);

                buffer[i++] = audio_soft_clamp(right + both);
                buffer[i++] = audio_soft_clamp(left  + both);
        }
}

void audio_start (Audio* audio, Ship* ship)
{
	if(!audio->enabled)
		return;
	
	audio->ship  = ship;

	audio->back.volume   = 1;
	audio->left.volume   =
	audio->right.volume  = 0;
	audio->hit.volume    = 0;
	audio->crash.volume  = 0;

	float freq_factor = 22050. / audio->fmt.freq;
	audio->back.decay   = 0 * freq_factor;
	audio->left.decay   = 
	audio->right.decay  = 0.001 * freq_factor;
	audio->hit.decay    = 0.0004 * freq_factor;
	audio->crash.decay  = 0.0002 * freq_factor;

	audio->back.max   = 0.3;
	audio->left.max   =
	audio->right.max  = 0.1;
	audio->hit.max    = 0.5;
	audio->crash.max  = 1;

	audio->hit.step   = 1;
	audio->crash.step = 1;

	audio->crash.index = 0;

	SDL_PauseAudio(0);
}

void audio_stop (Audio* audio)
{
	if(!audio->enabled)
		return;
	SDL_PauseAudio(1);
}

bool audio_load(Audio *audio, Wave *wave, const char *filename)
{
	wave->enabled = false;

	filename = FIND (filename);
	if (SDL_LoadWAV(filename, 
			         & wave->fmt,
			(Uint8**)&(wave->data), 
			         & wave->size
		) == NULL)
	{
		fprintf(stderr, "SDL_LoadWAV(%s): %s\n", filename, SDL_GetError());
		return false;
	}
	
	if (                   wave->fmt.format   != AUDIO_S16 
	||                     wave->fmt.channels != 1
	|| (audio->fmt.freq && wave->fmt.freq     != audio->fmt.freq))
	{
		fprintf(stderr, "Audio format innapropriate for '%s', "
				"all files should be of the same format\n", filename);

		return false;
	}

	if(!audio->fmt.freq) // set base freq if not set already
		audio->fmt.freq = wave->fmt.freq;

	wave->index = rand()%wave->size;
	wave->enabled = true;

	fprintf(stderr, "Audio Wave '%s' %dHz 1ch 16bit-signed\n", 
			filename, wave->fmt.freq);

	return true;
}

void audio_init (Audio* audio, bool enabled)
{
	audio->enabled = false;
	if (!enabled)
		return;

	audio->fmt.format = AUDIO_S16;
	audio->fmt.freq = 0;
	audio->fmt.channels = 2;
	audio->fmt.samples = 512;
	audio->fmt.callback = audio_mix;
	audio->fmt.userdata = audio;

	audio_load(audio, &audio->back,  AUDIO_THRUST_FILE);
	audio_load(audio, &audio->left,  AUDIO_THRUST_FILE);
	audio_load(audio, &audio->right, AUDIO_THRUST_FILE);
	audio_load(audio, &audio->hit,   AUDIO_HIT_FILE);
	audio_load(audio, &audio->crash, AUDIO_CRASH_FILE);

	if(audio->fmt.freq == 0) {
		fprintf(stderr, "no audio file loaded\n");
		return;
	}

	printf("Audio %dHz %dch 16bit-signed\n",
			audio->fmt.freq,
			audio->fmt.channels);

	if(SDL_OpenAudio(&audio->fmt, NULL) != 0) {
		fprintf(stderr, "SDL_OpenAudio(): %s\n", SDL_GetError());
		return;
	}

	atexit(SDL_CloseAudio);
	audio->enabled = true;
}

// vim600:fdm=syntax:fdn=1:
