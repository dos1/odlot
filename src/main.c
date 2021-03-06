/*! \file main.c
 *  \brief Main file of Super Examples.
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
#include "defines.h"
#include <libsuperderpy.h>
#include <signal.h>
#include <stdio.h>

static _Noreturn void derp(int sig) {
	ssize_t __attribute__((unused)) n = write(STDERR_FILENO, "Segmentation fault\nI just don't know what went wrong!\n", 54);
	abort();
}

int main(int argc, char** argv) {
	signal(SIGSEGV, derp);

	srand(time(NULL));

	al_set_org_name("Holy Pangolin");
	al_set_app_name(LIBSUPERDERPY_GAMENAME_PRETTY);

	struct Game* game = libsuperderpy_init(argc, argv, LIBSUPERDERPY_GAMENAME, (struct Viewport){1920, 1080});
	if (!game) { return 1; }
	al_set_window_title(game->display, LIBSUPERDERPY_GAMENAME_PRETTY);

	LoadGamestate(game, "myszka");

	LoadGamestate(game, "altanka");
	LoadGamestate(game, "bongo");
	LoadGamestate(game, "but");
	LoadGamestate(game, "ciuchcia");
	LoadGamestate(game, "domek");
	LoadGamestate(game, "gaski");
	LoadGamestate(game, "intro");
	LoadGamestate(game, "logo");
	LoadGamestate(game, "pienki");
	LoadGamestate(game, "pudelko");
	LoadGamestate(game, "rave");
	LoadGamestate(game, "rzeczka");
	LoadGamestate(game, "taniec");
	LoadGamestate(game, "wrona");

	StartGamestate(game, "intro");

	game->data = CreateGameData(game);

	game->show_loading_on_launch = true;
	game->handlers.event = GlobalEventHandler;
	game->handlers.destroy = DestroyGameData;
	game->handlers.prelogic = PreLogic;

	EnableCompositor(game, Compositor);

	al_hide_mouse_cursor(game->display);

	return libsuperderpy_run(game);
}
