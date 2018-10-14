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

enum Myszol {
	MYSZOL_LEFT = 0,
	MYSZOL_RIGHT = 1,
	MYSZOL_BOTTOM = 2,
	MYSZOL_BOTTOM_LEFT = 3,
	MYSZOL_TOP = 4,
	MYSZOL_TOP_RIGHT = 5
};

struct GamestateResources {
	// This struct is for every resource allocated and used by your gamestate.
	// It gets created on load and then gets passed around to all other function calls.
	ALLEGRO_BITMAP* myszka;
	ALLEGRO_AUDIO_STREAM* music;

	enum Myszol myszol;
	int counter;
	double pos;
	double angle;
	double rand;
	int con;
};

int Gamestate_ProgressCount = 1; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	data->counter++;
	if (data->counter % 6 == 0) {
		data->pos = al_get_audio_stream_position_secs(data->music) / al_get_audio_stream_length_secs(data->music);
		data->angle = rand() / (double)RAND_MAX;
	}

	if (data->pos >= 1.0) {
		data->con++;
		if (data->con > 10) {
			if (!game->data->next) {
				return;
			}
			SwitchCurrentGamestate(game, game->data->next);
			free(game->data->next);
			game->data->next = NULL;
		}
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	if (data->con) {
		return;
	}
	switch (data->myszol) {
		case MYSZOL_RIGHT:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				2500 * data->pos, 1080 / 2.0 + data->rand * 1080, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
		case MYSZOL_LEFT:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				2500 * (1.0 - data->pos) - 500, 1080 / 2.0 + data->rand * 1080, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
		case MYSZOL_TOP:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				1920 / 2.0 + 1920 * data->rand, 1300 * (1.0 - data->pos) - 200, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
		case MYSZOL_BOTTOM:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				1920 / 2.0 + 1920 * data->rand, 1300 * data->pos, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
		case MYSZOL_BOTTOM_LEFT:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				2500 * (1.0 - data->pos) - 500, 1300 * data->pos, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
		case MYSZOL_TOP_RIGHT:
			al_draw_scaled_rotated_bitmap(data->myszka, al_get_bitmap_width(data->myszka) / 2.0, al_get_bitmap_height(data->myszka) / 2.0,
				2500 * data->pos, 1300 * (1.0 - data->pos) - 200, 0.5, 0.5, data->angle * 0.2 - 0.1, 0);
			break;
	}
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
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

	data->music = al_load_audio_stream(GetDataFilePath(game, rand() % 2 ? "przejscie.flac" : "przejscie2.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);
	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_bitmap(data->myszka);
	al_destroy_audio_stream(data->music);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	HideMouse(game);
	al_set_audio_stream_playing(data->music, true);
	data->counter = 0;
	data->con = 0;

	data->myszol = rand() % 6;

	data->rand = (rand() / (double)RAND_MAX) * 0.5 - 0.25;

	switch (data->myszol) {
		case MYSZOL_LEFT:
			data->myszka = al_load_bitmap(GetDataFilePath(game, PunchNumber(game, "myszki/lewoX.webp", 'X', rand() % 2)));
			break;
		case MYSZOL_RIGHT:
			data->myszka = al_load_bitmap(GetDataFilePath(game, PunchNumber(game, "myszki/prawoX.webp", 'X', rand() % 3)));
			break;
		case MYSZOL_TOP:
			data->myszka = al_load_bitmap(GetDataFilePath(game, PunchNumber(game, "myszki/goraX.webp", 'X', rand() % 3)));
			break;
		case MYSZOL_BOTTOM:
			data->myszka = al_load_bitmap(GetDataFilePath(game, PunchNumber(game, "myszki/dolX.webp", 'X', rand() % 4)));
			break;
		case MYSZOL_BOTTOM_LEFT:
			data->myszka = al_load_bitmap(GetDataFilePath(game, "myszki/lewodol.webp"));
			break;
		case MYSZOL_TOP_RIGHT:
			data->myszka = al_load_bitmap(GetDataFilePath(game, "myszki/prawogora.webp"));
			break;
	}
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
