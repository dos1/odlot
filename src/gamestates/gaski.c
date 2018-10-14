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
	struct Character* bg;
	ALLEGRO_AUDIO_STREAM* music;
	ALLEGRO_SAMPLE* sample;

	struct Character *gaski[64], *gaska;

	int counter;
};

int Gamestate_ProgressCount = 74; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	AnimateCharacter(game, data->bg, delta, 1.0);

	for (int i = 0; i < 64; i++) {
		if (data->gaski[i]->reversing) {
			MoveCharacter(game, data->gaski[i], -300 * delta, 0, 0);
		} else {
			MoveCharacter(game, data->gaski[i], 300 * delta, 0, 0);
		}
	}
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	data->gaski[rand() % 64]->angle = rand() / (double)RAND_MAX * 0.4 - 0.2;
	data->gaski[rand() % 64]->angle = rand() / (double)RAND_MAX * 0.3 - 0.15;
	data->counter++;

	if (data->counter == 25) {
		al_play_sample(data->sample, 0.4, -0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}
	if (data->counter == 80) {
		al_play_sample(data->sample, 0.4, 0.5, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}
	if (data->counter == 200) {
		al_play_sample(data->sample, 0.45, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
	}
	if (data->counter > 6 * 60) {
		if (rand() % 10 == 0) {
			al_play_sample(data->sample, 0.3, rand() / (double)RAND_MAX * 2 - 1.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
		}
	}
	if (data->counter >= 16 * 60) {
		game->data->next = strdup("but");
		SwitchCurrentGamestate(game, "myszka");
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	DrawCharacter(game, data->bg);
	for (int i = 0; i < 64; i++) {
		DrawCharacter(game, data->gaski[i]);
	}
}

void Gamestate_ProcessEvent(struct Game* game, struct GamestateResources* data, ALLEGRO_EVENT* ev) {
	// Called for each event in Allegro event queue.
	// Here you can handle user input, expiring timers etc.
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)) {
		UnloadCurrentGamestate(game); // mark this gamestate to be stopped and unloaded
		// When there are no active gamestates, the engine will quit.
	}

	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
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

	al_reserve_samples(64);
	progress(game);

	data->gaska = CreateCharacter(game, "gaski");
	RegisterSpritesheet(game, data->gaska, "przod1");
	RegisterSpritesheet(game, data->gaska, "przod2");
	RegisterSpritesheet(game, data->gaska, "tyl1");
	RegisterSpritesheet(game, data->gaska, "tyl2");
	LoadSpritesheets(game, data->gaska, progress);

	for (int i = 0; i < 64; i++) {
		data->gaski[i] = CreateCharacter(game, "gaski");
		data->gaski[i]->shared = true;
		data->gaski[i]->spritesheets = data->gaska->spritesheets;
		bool ktora = rand() % 2;
		SelectSpritesheet(game, data->gaski[i], ktora ? "przod1" : "przod2");
		bool flip = rand() % 2;
		data->gaski[i]->flipX = flip;
		double x;
		if ((ktora && !flip) || (!ktora && flip)) {
			x = -1920 * (rand() / (double)RAND_MAX) - 1920;
		} else {
			x = 1920 * (rand() / (double)RAND_MAX) + 1920 + 1920;
		}
		SetCharacterPosition(game, data->gaski[i], x, 1080 * ((rand() / (double)RAND_MAX) / 3.0 + 0.6), 0);
		data->gaski[i]->reversing = x > 0;
		data->gaski[i]->scaleX = 0.25 + i * 0.005;
		data->gaski[i]->scaleY = 0.25 + i * 0.005;
		progress(game);
	}

	SelectSpritesheet(game, data->gaski[0], "przod1");
	SelectSpritesheet(game, data->gaski[1], "przod2");
	SetCharacterPosition(game, data->gaski[0], -200, GetCharacterY(game, data->gaski[0]), 0);
	SetCharacterPosition(game, data->gaski[1], 1920 + 200, GetCharacterY(game, data->gaski[0]), 0);
	data->gaski[0]->reversing = false;
	data->gaski[1]->reversing = true;
	data->gaski[0]->flipX = false;
	data->gaski[1]->flipX = false;
	data->gaski[0]->scaleX = 0.25 + 64 * 0.005;
	data->gaski[0]->scaleY = 0.25 + 64 * 0.005;
	data->gaski[1]->scaleX = 0.25 + 64 * 0.005;
	data->gaski[1]->scaleY = 0.25 + 64 * 0.005;
	progress(game);

	data->music = al_load_audio_stream(GetDataFilePath(game, "niepokoj.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_gain(data->music, 0.5);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "gaska.flac"));
	progress(game);

	data->bg = CreateCharacter(game, "bgs");
	RegisterSpritesheet(game, data->bg, "bgs");
	LoadSpritesheets(game, data->bg, progress);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->music);

	DestroyCharacter(game, data->bg);
	al_destroy_sample(data->sample);

	for (int i = 0; i < 64; i++) {
		DestroyCharacter(game, data->gaski[i]);
	}
	DestroyCharacter(game, data->gaska);

	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	HideMouse(game);
	al_set_audio_stream_playing(data->music, true);
	SetCharacterPosition(game, data->bg, 1920 / 2.0, 1080 / 2.0, 0);
	data->counter = 0;
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->music, false);
	al_stop_samples();
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
