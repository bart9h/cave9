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
#include "util.h"

static float audio_index_value(signed short *data, int size, float *i, float freq)
{
        unsigned int samples = size/2; // 16bit
        float i2 = fmodf(*i, 1), i1 = 1 - i2;
        float v =
                i1 * data[(int)(*i)] +
                i2 * data[(int)(*i + 1) % samples];
        *i = fmodf(*i + freq, samples);
        return v;
}

static void audio_mix(void *data, Uint8 *stream, int len)
{
        Audio *audio = (Audio*)data;

        float max_vel[3] = { MAX_VEL_X, MAX_VEL_Y, MAX_VEL_Z };
        float intensity = log(1+
                10000*(1-CLAMP(audio->ship->dist / MIN_CAVE_RADIUS,0,1)) +
                10000*MIN(1, (LEN(audio->ship->vel)-MAX_VEL_Z) / (LEN(max_vel)-MAX_VEL_Z))
                ) / log(1+20000);
        float low_freq  =  .2 +  .6 * intensity;
        float high_freq = 1.0 + 2.0 * intensity;

		float collision = ship_hit(audio->ship);
		if(collision > audio->hit)
			audio->hit = collision;
        float hit_freq  = audio->hit != 0;

        unsigned int buffer_samples = len/2; // 16bit
        signed short *buffer = (signed short *)stream;
        for(unsigned i = 0; i < buffer_samples; ) {
                // FIXME use audio->fmt-> sample rate to make a transition time in seconds:
                audio->right = audio->right * .999 + .001 * audio->ship->righton;
                audio->left  = audio->left  * .999 + .001 * audio->ship->lefton;
                audio->hit  *= .9996;

                float high = audio_index_value(audio->thrust_data, audio->thrust_size, &audio->high_index, high_freq);
                float low  = audio_index_value(audio->thrust_data, audio->thrust_size, &audio->low_index,  low_freq);
                float hit  = audio_index_value(audio->hit_data,    audio->hit_size,    &audio->hit_index,  hit_freq);
                buffer[i++] = .3 * low + .1 * high * audio->right + .5 * hit * audio->hit;
                buffer[i++] = .3 * low + .1 * high * audio->left  + .5 * hit * audio->hit;
        }
}

void audio_start (Audio* audio, Ship* ship)
{
	if(!audio->enabled)
		return;
	audio->ship  = ship;
	audio->hit  =
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

void audio_init (Audio* audio, bool enabled)
{
	audio->enabled = false;
	if (!enabled)
		return;

	const char* audio_file = FIND (AUDIO_FILE);
	if (SDL_LoadWAV(audio_file, &audio->fmt,
				(Uint8**)&(audio->thrust_data), &audio->thrust_size) == NULL)
	{
		fprintf(stderr, "SDL_LoadWAV(%s): %s\n", audio_file, SDL_GetError());
		return;
	}
	if(audio->fmt.format != AUDIO_S16 && audio->fmt.channels != 1) {
		fprintf(stderr, "audio file '%s' expected to be 1ch and 16bit\n", audio_file);
		return;

	}

	const char* audio_hit_file = FIND (AUDIO_HIT_FILE);
	if (SDL_LoadWAV(audio_hit_file, &audio->hit_fmt,
				(Uint8**)&(audio->hit_data), &audio->hit_size) == NULL)
	{
		fprintf(stderr, "SDL_LoadWAV(%s): %s\n", audio_hit_file, SDL_GetError());
		return;
	}
	if(audio->hit_fmt.format != AUDIO_S16 && audio->hit_fmt.channels != 1) {
		fprintf(stderr, "audio file '%s' expected to be 1ch and 16bit\n", audio_hit_file);
		return;
	}

	audio->hit_index =
	audio->low_index =
	audio->high_index = 0;


	audio->fmt.channels = 2;
	audio->fmt.samples = 512;
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
