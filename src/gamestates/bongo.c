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
	ALLEGRO_BITMAP* bg;
	ALLEGRO_SAMPLE_INSTANCE* bongo[5];
	ALLEGRO_SAMPLE* sample[5];
	ALLEGRO_AUDIO_STREAM* music;
	int counter;

	int seq[4];

	int current;
	int user[4];
};

int Gamestate_ProgressCount = 7; // number of loading steps as reported by Gamestate_Load; 0 when missing

static int GetBongo(struct Game* game, struct GamestateResources* data) {
	int x = (int)(game->data->mouseX * 1920);
	int y = (int)(game->data->mouseY * 1080);
	int b = -1;
	if ((x > 537) && (y > 150) && (x < 537 + 219) && (y < 160 + 76)) {
		b = 0;
	} else if ((x > 537) && (y > 313) && (x < 537 + 301) && (y < 313 + 125)) {
		b = 1;
	} else if ((x > 845) && (y > 441) && (x < 845 + 321) && (y < 441 + 130)) {
		b = 2;
	} else if ((x > 1135) && (y > 361) && (x < 1135 + 285) && (y < 361 + 111)) {
		b = 3;
	} else if ((x > 1101) && (y > 269) && (x < 1101 + 272) && (y < 269 + 103)) {
		b = 4;
	}
	return b;
}

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (GetBongo(game, data) > -1) {
		game->data->hover = true;
	}
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	if (data->counter == 40) {
		if ((data->user[0] == data->seq[0]) &&
			(data->user[1] == data->seq[1]) &&
			(data->user[2] == data->seq[2]) &&
			(data->user[3] == data->seq[3])) {
			SwitchCurrentGamestate(game, "taniec");
			return;
		}

		data->seq[0] = rand() % 5;
		data->seq[1] = rand() % 5;
		data->seq[2] = rand() % 5;
		data->seq[3] = rand() % 5;

		al_stop_sample_instance(data->bongo[data->seq[0]]);
		al_play_sample_instance(data->bongo[data->seq[0]]);
	}
	if (data->counter == 70) {
		al_stop_sample_instance(data->bongo[data->seq[1]]);
		al_play_sample_instance(data->bongo[data->seq[1]]);
	}
	if (data->counter == 100) {
		al_stop_sample_instance(data->bongo[data->seq[2]]);
		al_play_sample_instance(data->bongo[data->seq[2]]);
	}
	if (data->counter == 130) {
		al_stop_sample_instance(data->bongo[data->seq[3]]);
		al_play_sample_instance(data->bongo[data->seq[3]]);
		ShowMouse(game);
	}
	data->counter++;
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	if (data->counter > 130) {
		al_draw_bitmap(data->bg, 0, 0, 0);
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
		if (data->counter < 130) {
			return;
		}
		int b = GetBongo(game, data);
		if (b >= 0) {
			al_stop_sample_instance(data->bongo[b]);
			al_play_sample_instance(data->bongo[b]);

			data->user[data->current] = b;
			data->current++;
			if (data->current == 4) {
				data->current = 0;
				data->counter = -20;
				HideMouse(game);
			}
		}
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

	data->bg = al_load_bitmap(GetDataFilePath(game, "bongo.webp"));
	progress(game);

	for (int i = 0; i < 5; i++) {
		data->sample[i] = al_load_sample(GetDataFilePath(game, PunchNumber(game, "bongoX.flac", 'X', i + 1)));
		data->bongo[i] = al_create_sample_instance(data->sample[i]);
		al_attach_sample_instance_to_mixer(data->bongo[i], game->audio.fx);
		al_set_sample_instance_playmode(data->bongo[i], ALLEGRO_PLAYMODE_ONCE);
		progress(game);
	}

	data->music = al_load_audio_stream(GetDataFilePath(game, "bongobg.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_gain(data->music, 0.5);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->music);
	al_destroy_bitmap(data->bg);
	for (int i = 0; i < 5; i++) {
		al_destroy_sample_instance(data->bongo[i]);
		al_destroy_sample(data->sample[i]);
	}
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	HideMouse(game);
	al_set_audio_stream_playing(data->music, true);
	data->counter = 0;
	data->seq[0] = rand() % 5;
	data->seq[1] = rand() % 5;
	data->seq[2] = rand() % 5;
	data->seq[3] = rand() % 5;
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
