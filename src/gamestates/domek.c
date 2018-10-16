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
	ALLEGRO_BITMAP *domek, *mask;
	ALLEGRO_SAMPLE_INSTANCE* sound;
	ALLEGRO_SAMPLE* sample;
	ALLEGRO_VIDEO* video;
	int counter;
	bool playing;
};

int Gamestate_ProgressCount = 4; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	CheckMask(game, data->mask);
	if (al_get_video_position(data->video, ALLEGRO_VIDEO_POSITION_ACTUAL) >= 15) {
		game->data->next = strdup("rave");
		SwitchCurrentGamestate(game, "myszka");
	}
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	al_draw_bitmap(data->domek, 0, 0, 0);
	if (data->playing) {
		ALLEGRO_BITMAP* bmp = al_get_video_frame(data->video);
		if (bmp) {
			al_draw_bitmap(bmp, 0, 0, 0);
		}
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
		if (!game->data->hover) { return; }
		al_set_video_playing(data->video, true);
		data->playing = true;
		HideMouse(game);
		al_set_sample_instance_gain(data->sound, 2.0);
	}
	if (ev->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
		if (!data->playing) { return; }
		game->data->next = strdup("rave");
		SwitchCurrentGamestate(game, "myszka");
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
	data->sample = al_load_sample(GetDataFilePath(game, "domek.flac"));
	data->sound = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->sound, game->audio.music);
	al_set_sample_instance_playmode(data->sound, ALLEGRO_PLAYMODE_LOOP);
	progress(game);

	data->domek = al_load_bitmap(GetDataFilePath(game, "domek.jpg"));
	progress(game);

	data->mask = al_load_bitmap(GetDataFilePath(game, "domekmask.webp"));
	progress(game);

	data->video = al_open_video(GetDataFilePath(game, "domek.ogv"));

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_bitmap(data->domek);
	al_destroy_bitmap(data->mask);
	al_destroy_sample_instance(data->sound);
	al_destroy_sample(data->sample);
	al_close_video(data->video);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	ShowMouse(game);
	data->counter = 0;
	data->playing = false;
	al_play_sample_instance(data->sound);
	al_start_video(data->video, game->audio.fx);
	al_set_video_playing(data->video, false);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_stop_sample_instance(data->sound);
	al_set_video_playing(data->video, false);
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
