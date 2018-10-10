/*! \file common.c
 *  \brief Common stuff that can be used by all gamestates.
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

#include "common.h"
#include <libsuperderpy.h>

void Compositor(struct Game* game, struct Gamestate* gamestates) {
	struct Gamestate* tmp = gamestates;
	int counter = 0;
	while (tmp) {
		if ((tmp->loaded) && (tmp->started)) {
			counter++;
		}
		tmp = tmp->next;
	}
	tmp = gamestates;
	while (tmp) {
		al_use_shader(game->data->grain);
		al_set_shader_float("time", al_get_time()); //data->blink_counter/3600.0);
		if ((tmp->loaded) && (tmp->started)) {
			float randx = (rand() / (double)RAND_MAX) * 3.0;
			float randy = (rand() / (double)RAND_MAX) * 3.0;
			if (rand() % 200) {
				randx = 0;
				randy = 0;
			}

			float color = 1.0 + (rand() / (double)RAND_MAX) * 0.01 - 0.005;

			al_draw_tinted_scaled_rotated_bitmap(tmp->fb, al_map_rgba_f(color, color, color, color), al_get_bitmap_width(tmp->fb) / 2.0, al_get_bitmap_height(tmp->fb) / 2.0,
				al_get_display_width(game->display) / 2.0 + randx, al_get_display_height(game->display) / 2.0 + randy, 1.05, 1.05, 0.0, 0);
		}
		al_use_shader(NULL);
		tmp = tmp->next;
	}
}

bool GlobalEventHandler(struct Game* game, ALLEGRO_EVENT* ev) {
	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_M)) {
		game->config.mute = !game->config.mute;
		al_set_mixer_gain(game->audio.mixer, game->config.mute ? 0.0 : 1.0);
		SetConfigOption(game, "SuperDerpy", "mute", game->config.mute ? "1" : "0");
		PrintConsole(game, "Mute: %d", game->config.mute);
	}

	if ((ev->type == ALLEGRO_EVENT_KEY_DOWN) && (ev->keyboard.keycode == ALLEGRO_KEY_F)) {
		game->config.fullscreen = !game->config.fullscreen;
		if (game->config.fullscreen) {
			SetConfigOption(game, "SuperDerpy", "fullscreen", "1");
			al_hide_mouse_cursor(game->display);
		} else {
			SetConfigOption(game, "SuperDerpy", "fullscreen", "0");
			al_show_mouse_cursor(game->display);
		}
#ifdef ALLEGRO_ANDROID
		al_set_display_flag(game->display, ALLEGRO_FRAMELESS, game->config.fullscreen);
#endif
		al_set_display_flag(game->display, ALLEGRO_FULLSCREEN_WINDOW, game->config.fullscreen);
		SetupViewport(game, game->viewport_config);
		PrintConsole(game, "Fullscreen toggled");
	}

	return false;
}

struct CommonResources* CreateGameData(struct Game* game) {
	struct CommonResources* data = calloc(1, sizeof(struct CommonResources));
	data->grain = CreateShader(game, GetDataFilePath(game, "shaders/vertex.glsl"), GetDataFilePath(game, "shaders/grain.glsl"));
	return data;
}

void DestroyGameData(struct Game* game) {
	DestroyShader(game, game->data->grain);
	free(game->data);
}
