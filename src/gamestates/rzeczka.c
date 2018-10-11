/*! \file empty.c
 *  \brief Empty gamestate.
 */
/*
 * Copyright (c) Sebastian Krzyszkowiak <dos@dosowisko.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"
#include <libsuperderpy.h>

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.
	struct Character* rzeczka;
	ALLEGRO_AUDIO_STREAM* music;
	ALLEGRO_SAMPLE_INSTANCE* sound;
	ALLEGRO_SAMPLE* sample;

	int state;

	int counter;

	int oldpos;

	unsigned char fade;
};

int Gamestate_ProgressCount = 5; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->state) {
		AnimateCharacter(game, data->rzeczka, delta, 1.0);
	}
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->state) {
		data->counter++;
		if (data->rzeczka->pos != data->oldpos) {
			if (data->rzeczka->pos < 4) {
				if (data->fade < 20) {
					data->fade = 0;
				} else {
					data->fade -= 20;
				}
			}
			data->oldpos = data->rzeczka->pos;
		}
	}

	if (data->counter > 5 * 60) {
		al_set_audio_stream_gain(data->music, al_get_audio_stream_gain(data->music) - 0.001);
		al_set_sample_instance_gain(data->sound, al_get_sample_instance_gain(data->sound) - 0.001);
	}
	if (al_get_audio_stream_gain(data->music) <= 0) {
		UnloadAllGamestates(game);
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	data->rzeczka->tint = al_map_rgba(data->fade, data->fade, data->fade, data->fade);
	DrawCharacter(game, data->rzeczka);
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
		data->state++;
		al_play_sample_instance(data->sound);
	}
}

void* Gamestate_Load(struct Game* game, void (*progress)(struct Game*)) {
	// Called once, when the gamestate library is being loaded.
	// Good place for allocating memory, loading bitmaps etc.
	//
	// NOTE: There's no OpenGL context available here. If you want to prerender something,
	// create VBOs, etc. do it in Gamestate_PostLoad.

	struct GamestateResources* data = calloc(1, sizeof(struct GamestateResources));
	progress(game); // report that we progressed with the loading, so the engine can move a progress bar

	data->music = al_load_audio_stream(GetDataFilePath(game, "rzeczka.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_gain(data->music, 0.9);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);

	data->sample = al_load_sample(GetDataFilePath(game, "odlot.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.music);
	al_set_sample_instance_playmode(data->sound, ALLEGRO_PLAYMODE_LOOP);

	data->rzeczka = CreateCharacter(game, "rzeczka");
	RegisterSpritesheet(game, data->rzeczka, "animacja_rzeka");
	LoadSpritesheets(game, data->rzeczka, progress);
	SelectSpritesheet(game, data->rzeczka, "-animacja_rzeka");

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->music);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	al_show_mouse_cursor(game->display);
	al_set_audio_stream_playing(data->music, true);
	SetCharacterPosition(game, data->rzeczka, 1920 / 2.0, 1080 / 2.0, 0);
	data->counter = 0;
	data->state = 0;
	data->fade = 255;
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->music, false);
}

// Optional endpoints:

void Gamestate_PostLoad(struct Game* game, struct GamestateResources* data) {
	// This is called in the main thread after Gamestate_Load has ended.
	// Use it to prerender bitmaps, create VBOs, etc.
}

void Gamestate_Pause(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets paused (so only Draw is being called, no Logic nor ProcessEvent)
	// Pause your timers and/or sounds here.
}

void Gamestate_Resume(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets resumed. Resume your timers and/or sounds here.
}

void Gamestate_Reload(struct Game* game, struct GamestateResources* data) {
	// Called when the display gets lost and not preserved bitmaps need to be recreated.
	// Unless you want to support mobile platforms, you should be able to ignore it.
}