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
	ALLEGRO_AUDIO_STREAM* music;

	ALLEGRO_SAMPLE* sample;
	ALLEGRO_SAMPLE_INSTANCE* pac;

	ALLEGRO_VIDEO* video;

	int state;

	int counter;
};

int Gamestate_ProgressCount = 3; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	//AnimateCharacter(game, data->pienki, delta, 1.0);
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	data->counter++;
	if (data->counter == (int)(2.0 * 60)) {
		al_play_sample_instance(data->pac);
	}
	if (data->counter == (int)(3.5 * 60)) {
		SwitchScene(game, "rzeczka");
	}
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.
	if (al_get_video_frame(data->video)) {
		al_draw_bitmap(al_get_video_frame(data->video), 0, 0, 0);
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

	data->music = al_load_audio_stream(GetDataFilePath(game, "wrona.flac"), 4, 2048);
	al_set_audio_stream_playing(data->music, false);
	al_set_audio_stream_playmode(data->music, ALLEGRO_PLAYMODE_LOOP);
	al_set_audio_stream_gain(data->music, 1.0);
	al_attach_audio_stream_to_mixer(data->music, game->audio.music);
	progress(game);

	data->sample = al_load_sample(GetDataFilePath(game, "alarm.flac"));
	data->pac = al_create_sample_instance(data->sample);
	al_attach_sample_instance_to_mixer(data->pac, game->audio.fx);
	al_set_sample_instance_gain(data->pac, 1.0);
	al_set_sample_instance_playmode(data->pac, ALLEGRO_PLAYMODE_ONCE);
	progress(game);

	data->video = al_open_video(GetDataFilePath(game, "wrona.ogv"));

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->music);
	al_close_video(data->video);
	al_destroy_sample_instance(data->pac);
	al_destroy_sample(data->sample);
	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	HideMouse(game);
	al_set_audio_stream_playing(data->music, true);
	data->counter = 0;
	data->state = 0;
	al_start_video(data->video, game->audio.fx);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->music, false);
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
