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
	ALLEGRO_BITMAP *bg, *gradient;
	ALLEGRO_AUDIO_STREAM* music;
	ALLEGRO_AUDIO_STREAM* taniec;

	struct Character *niebieski, *sowka, *grzebien;
	int counter;
};

int Gamestate_ProgressCount = 5; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	AnimateCharacter(game, data->niebieski, delta, 1.0);
	AnimateCharacter(game, data->sowka, delta, 1.0);
	AnimateCharacter(game, data->grzebien, delta, 2.0);

	data->grzebien->scaleX = 0.333;
	data->grzebien->scaleY = 0.333;
	float pos = al_get_audio_stream_position_secs(data->taniec) / al_get_audio_stream_length_secs(data->taniec);
	SetCharacterPosition(game, data->grzebien, 1920 * 2 * (1.0 - pos) - 1920 / 2.0, 1080 * 0.4 + sin(game->time * 3.0) * 40, 0);
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	data->counter++;
	float pos = al_get_audio_stream_position_secs(data->taniec) / al_get_audio_stream_length_secs(data->taniec);
	if (pos >= 1.0) {
		game->data->next = strdup("domek");
		SwitchCurrentGamestate(game, "myszka");
	}
	if (data->counter % 20 == 0) {
		SetCharacterPosition(game, data->niebieski, rand() / (double)RAND_MAX * 1920, rand() / (double)RAND_MAX * 1080, rand());
		data->niebieski->scaleX = (0.5 + rand() / (double)RAND_MAX) / 3.0;
		data->niebieski->scaleY = data->niebieski->scaleX;
	}
	if (data->counter % 25 == 0) {
		SetCharacterPosition(game, data->sowka, rand() / (double)RAND_MAX * 1920, rand() / (double)RAND_MAX * 1080, rand());
		data->sowka->scaleX = (0.5 + rand() / (double)RAND_MAX) / 3.0;
		data->sowka->scaleY = data->sowka->scaleX;
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	al_draw_bitmap(data->bg, 0, 0, 0);
	DrawCharacter(game, data->grzebien);
	DrawCharacter(game, data->niebieski);
	DrawCharacter(game, data->sowka);
	al_draw_bitmap(data->gradient, 0, 0, 0);
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

	data->bg = al_load_bitmap(GetDataFilePath(game, "bongo.png"));
	data->gradient = al_load_bitmap(GetDataFilePath(game, "gradient.png"));

	data->music = al_load_audio_stream(GetDataFilePath(game, "bongobg.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_gain(data->music, 1.0);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);

	data->taniec = al_load_audio_stream(GetDataFilePath(game, "taniec.flac"), 4, 2048);
	al_set_audio_stream_playing(data->taniec, false);
	al_set_audio_stream_playmode(data->taniec, ALLEGRO_PLAYMODE_ONCE);
	al_attach_audio_stream_to_mixer(data->taniec, game->audio.music);

	data->niebieski = CreateCharacter(game, "niebieski");
	RegisterSpritesheet(game, data->niebieski, "niebieski_przod");
	RegisterSpritesheet(game, data->niebieski, "niebieski_tyl");
	LoadSpritesheets(game, data->niebieski, progress);

	data->sowka = CreateCharacter(game, "sowka");
	RegisterSpritesheet(game, data->sowka, "sowka_przod");
	RegisterSpritesheet(game, data->sowka, "sowka_tyl");
	LoadSpritesheets(game, data->sowka, progress);

	data->grzebien = CreateCharacter(game, "grzebien");
	RegisterSpritesheet(game, data->grzebien, "grzebien_macha");
	LoadSpritesheets(game, data->grzebien, progress);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->music);
	al_destroy_audio_stream(data->taniec);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	al_hide_mouse_cursor(game->display);
	al_set_audio_stream_playing(data->music, true);
	al_set_audio_stream_playing(data->taniec, true);
	data->counter = 0;
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playing(data->taniec, false);
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
