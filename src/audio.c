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
#include "audio.h"


float audio_index_value(Audio *audio, float *i, float freq)
{
        unsigned int samples = audio->size/2; // 16bit
        float i2 = fmodf(*i, 1), i1 = 1 - i2;
        float v =
                i1 * audio->data[(int)(*i)] +
                i2 * audio->data[(int)(*i + 1) % samples];
        *i = fmodf(*i + freq, samples);
        return v;
}

void audio_mix(void *data, Uint8 *stream, int len)
{
        Audio *audio = (Audio*)data;

        float max_vel[3] = { MAX_VEL_X, MAX_VEL_Y, MAX_VEL_Z };
        float intensity = log(1+
                10000*(1-CLAMP(audio->ship->dist / MIN_CAVE_RADIUS,0,1)) +
                10000*MIN(1, (LEN(audio->ship->vel)-MAX_VEL_Z) / (LEN(max_vel)-MAX_VEL_Z))
                ) / log(1+20000);

        float hit_freq  = 6 + 10 * RAND;
        float low_freq  =  .2 +  .6 * intensity;
        float high_freq = 1.0 + 2.0 * intensity;

		float collision = ship_hit(audio->ship);

        unsigned int buffer_samples = len/2; // 16bit
        signed short *buffer = (signed short *)stream;
        for(unsigned i = 0; i < buffer_samples; ) {
                // FIXME use audio->fmt-> sample rate to make a transition time in seconds:
                audio->right = audio->right * .999 + .001 * audio->ship->righton;
                audio->left  = audio->left  * .999 + .001 * audio->ship->lefton;

                float high = audio_index_value(audio, &audio->high, high_freq);
                float low  = audio_index_value(audio, &audio->low,  low_freq);
                float hit  = audio_index_value(audio, &audio->hit,  hit_freq);
                buffer[i++] = .3 * low + .1 * high * audio->right + .2 * hit * collision;
                buffer[i++] = .3 * low + .1 * high * audio->left  + .2 * hit * collision;
        }
}

void audio_start (Audio* audio, Ship* ship)
{
	if(!audio->enabled)
		return;
	audio->ship  = ship;
	audio->left  =
	audio->right = 0;
	SDL_PauseAudio(0);
}

void audio_stop (Audio* audio)
{
	if(!audio->enabled)
		return;
	SDL_PauseAudio(1);
}

void audio_init (Audio* audio)
{
	audio->enabled = false;

	if (SDL_LoadWAV(AUDIO_FILE, &audio->fmt,
				(Uint8**)&(audio->data), &audio->size) == NULL)
	{
		fprintf(stderr, "SDL_LoadWAV(%s): %s\n", AUDIO_FILE, SDL_GetError());
		return;
	}

	audio->hit =
	audio->low =
	audio->high = 0;


	//audio->fmt.freq = 8000;
	if(audio->fmt.format != AUDIO_S16) {
		fprintf(stderr, "audio file '%s' must be 16bit (its %d) and signed (%08x, not %08x)\n",
				AUDIO_FILE, audio->fmt.format & 0xff, AUDIO_S16, audio->fmt.format);
		return;
	}
	if(audio->fmt.channels != 1) {
		fprintf(stderr, "audio file '%s' must be 1 channel (not %d)\n",
				AUDIO_FILE, audio->fmt.channels);
		return;

	}
	audio->fmt.channels = 2;
	audio->fmt.samples = 128;
	audio->fmt.callback = audio_mix;
	audio->fmt.userdata = audio;

	if(SDL_OpenAudio(&audio->fmt, NULL) != 0) {
		fprintf(stderr, "SDL_OpenAudio(): %s\n", SDL_GetError());
		return;
	}

	atexit(SDL_CloseAudio);
	audio->enabled = true;
}

// vim600:fdm=syntax:fdn=1:
