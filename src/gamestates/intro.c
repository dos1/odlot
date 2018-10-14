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
	struct Character* grzebien;
	ALLEGRO_AUDIO_STREAM *spada, *rosnie, *odlot, *jeden, *dwa, *trzy;
	bool unlocked;
	int counter;
	bool odlatuje;

	int distance;

	ALLEGRO_SHADER* circ;
};

int Gamestate_ProgressCount = 10; // number of loading steps as reported by Gamestate_Load; 0 when missing

void Gamestate_Logic(struct Game* game, struct GamestateResources* data, double delta) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	SetCharacterPosition(game, data->grzebien, 1920 * 0.7, 1080 * Interpolate(fmin(data->counter, 100) / 100.0, TWEEN_STYLE_BACK_IN_OUT) * 1.4 - 800, 0);

	if (!data->unlocked) {
		int distance = (int)(sqrt(pow(game->data->mouseX * 1920 - GetCharacterX(game, data->grzebien), 2) +
													 pow(game->data->mouseY * 1080 - GetCharacterY(game, data->grzebien), 2)) /
			250.0);
		if (distance > 2) {
			distance = 2;
		}

		if (distance != data->distance) {
			PrintConsole(game, "dist %d", distance);
			al_set_audio_stream_playing(data->jeden, false);
			al_set_audio_stream_playing(data->dwa, false);
			al_set_audio_stream_playing(data->trzy, false);

			if (distance == 2) {
				al_rewind_audio_stream(data->jeden);
				al_set_audio_stream_playing(data->jeden, true);
			}
			if (distance == 1) {
				al_rewind_audio_stream(data->dwa);
				al_set_audio_stream_playing(data->dwa, true);
			}
			if (distance == 0) {
				al_rewind_audio_stream(data->trzy);
				al_set_audio_stream_playing(data->trzy, true);
			}

			data->distance = distance;
		}
	}

	if (data->odlatuje) {
		float pos = al_get_audio_stream_position_secs(data->odlot) / al_get_audio_stream_length_secs(data->odlot);
		SetCharacterPosition(game, data->grzebien, 1920 * 0.7 - 1920 * (pos - 0.02), 1080 * 1.4 - 800 - 1080 * (pos - 0.02), 0);

		if (pos >= 0.98) {
			SwitchCurrentGamestate(game, "logo");
		}
	}

	if (data->unlocked) {
		AnimateCharacter(game, data->grzebien, delta, 2.2);
	}
}

void Gamestate_Tick(struct Game* game, struct GamestateResources* data) {
	// Here you should do all your game logic as if <delta> seconds have passed.
	data->counter++;
}

void Gamestate_Draw(struct Game* game, struct GamestateResources* data) {
	// Draw everything to the screen here.

	if (data->counter > 100) {
		al_use_shader(data->circ);
		DrawTexturedRectangle(GetCharacterX(game, data->grzebien) - 100 * (3 - data->distance), GetCharacterY(game, data->grzebien) - 100 * (3 - data->distance),
			GetCharacterX(game, data->grzebien) + 100 * (3 - data->distance), GetCharacterY(game, data->grzebien) + 100 * (3 - data->distance),
			al_map_rgba(64, 64, 64, 64));
		al_use_shader(NULL);
	}

	DrawCharacter(game, data->grzebien);
}

static CharacterCallback(Grzebien) {
	struct GamestateResources* d = data;
	if (new&& new != old && strcmp(new->name, "grzebien_macha") == 0) {
		d->odlatuje = true;
		al_rewind_audio_stream(d->odlot);
		al_set_audio_stream_playing(d->odlot, true);
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
		if (data->unlocked) {
			return;
		}
		al_set_audio_stream_playing(data->rosnie, true);
		data->unlocked = true;
		HideMouse(game);
		SelectSpritesheet(game, data->grzebien, "grzebien_rosnie");
		data->grzebien->callback = Grzebien;
		data->grzebien->callbackData = data;
		al_set_audio_stream_playing(data->jeden, false);
		al_set_audio_stream_playing(data->dwa, false);
		al_set_audio_stream_playmode(data->trzy, ALLEGRO_PLAYMODE_ONCE);
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

	data->spada = al_load_audio_stream(GetDataFilePath(game, "grzebien.flac"), 4, 2048);
	al_set_audio_stream_playing(data->spada, false);
	al_attach_audio_stream_to_mixer(data->spada, game->audio.fx);
	progress(game);

	data->rosnie = al_load_audio_stream(GetDataFilePath(game, "grzebienrosnie.flac"), 4, 2048);
	al_set_audio_stream_playing(data->rosnie, false);
	al_attach_audio_stream_to_mixer(data->rosnie, game->audio.fx);
	progress(game);

	data->odlot = al_load_audio_stream(GetDataFilePath(game, "grzebienodlot.flac"), 4, 2048);
	al_set_audio_stream_playing(data->odlot, false);
	al_attach_audio_stream_to_mixer(data->odlot, game->audio.fx);
	progress(game);

	data->jeden = al_load_audio_stream(GetDataFilePath(game, "1.flac"), 4, 2048);
	al_set_audio_stream_playing(data->jeden, false);
	al_set_audio_stream_playmode(data->jeden, ALLEGRO_PLAYMODE_LOOP);
	al_attach_audio_stream_to_mixer(data->jeden, game->audio.music);
	progress(game);

	data->dwa = al_load_audio_stream(GetDataFilePath(game, "2.flac"), 4, 2048);
	al_set_audio_stream_playing(data->dwa, false);
	al_set_audio_stream_playmode(data->dwa, ALLEGRO_PLAYMODE_LOOP);
	al_attach_audio_stream_to_mixer(data->dwa, game->audio.music);
	progress(game);

	data->trzy = al_load_audio_stream(GetDataFilePath(game, "3.flac"), 4, 2048);
	al_set_audio_stream_playing(data->trzy, false);
	al_set_audio_stream_playmode(data->trzy, ALLEGRO_PLAYMODE_LOOP);
	al_attach_audio_stream_to_mixer(data->trzy, game->audio.music);
	progress(game);

	data->grzebien = CreateCharacter(game, "grzebien");
	RegisterSpritesheet(game, data->grzebien, "grzebien_rosnie");
	RegisterSpritesheet(game, data->grzebien, "grzebien_rosnie_skrzydelka");
	RegisterSpritesheet(game, data->grzebien, "grzebien_macha");
	LoadSpritesheets(game, data->grzebien, progress);
	SelectSpritesheet(game, data->grzebien, "grzebien_rosnie");

	data->circ = CreateShader(game, GetDataFilePath(game, "shaders/vertex.glsl"), GetDataFilePath(game, "shaders/circular_gradient.glsl"));

	data->grzebien->scaleX = 0.666;
	data->grzebien->scaleY = 0.666;

	return data;
}

void Gamestate_Unload(struct Game* game, struct GamestateResources* data) {
	// Called when the gamestate library is being unloaded.
	// Good place for freeing all allocated memory and resources.
	al_destroy_audio_stream(data->spada);
	al_destroy_audio_stream(data->rosnie);
	al_destroy_audio_stream(data->odlot);
	al_destroy_audio_stream(data->jeden);
	al_destroy_audio_stream(data->dwa);
	al_destroy_audio_stream(data->trzy);

	DestroyCharacter(game, data->grzebien);
	DestroyShader(game, data->circ);

	free(data);
}

void Gamestate_Start(struct Game* game, struct GamestateResources* data) {
	// Called when this gamestate gets control. Good place for initializing state,
	// playing music etc.
	al_set_audio_stream_playing(data->spada, true);
	data->counter = 0;
	data->unlocked = false;
	data->odlatuje = false;
	data->distance = -1;
	game->data->first_load = false;
	ShowMouse(game);
}

void Gamestate_Stop(struct Game* game, struct GamestateResources* data) {
	// Called when gamestate gets stopped. Stop timers, music etc. here.
	al_set_audio_stream_playing(data->spada, false);
	al_set_audio_stream_playing(data->rosnie, false);
	al_set_audio_stream_playing(data->odlot, false);
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
