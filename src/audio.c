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

void audio_mix (void* data, Uint8* stream, int len)
{
	Audio *audio = (Audio*)data;

	float low_freq = .4 - .3 * audio->ship->dist / MAX_CAVE_RADIUS;

	unsigned int data_samples = audio->size/2; // 16bit
	unsigned int buffer_samples = len/2; // 16bit
	signed short *buffer = (signed short*)stream;
	for (unsigned i = 0; i < buffer_samples; ) {
		float p2 = fmodf (audio->low_index, 1);
		float p1 = 1 - p2;
		float low =
			p1 * audio->data[(int)(audio->low_index)] +
			p2 * audio->data[(int)(audio->low_index + 1) % data_samples];

		float high = audio->data[audio->index];

		// FIXME use audio->fmt-> sample rate to make a transition time in seconds:
		audio->right = audio->right * .999 + .001 * audio->ship->righton;
		audio->left  = audio->left  * .999 + .001 * audio->ship->lefton;

		buffer[i++] = low * .3 + .1 * high * audio->right;
		buffer[i++] = low * .3 + .1 * high * audio->left;

		audio->index = (audio->index + 1) % data_samples;
		audio->low_index = fmodf(audio->low_index + low_freq, data_samples);
	}
}

void audio_start (Audio* audio, Ship* ship)
{
	if(!audio->enabled)
		return;
	audio->ship  = ship;
	audio->left  = 0;
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
	audio->index = 0;
	audio->low_index = 0;

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
