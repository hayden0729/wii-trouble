#ifndef TANK_CURSOR_H
#define TANK_CURSOR_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <wiiuse/wpad.h>

#include "cursors_png.h"

#include "button.h"
#include "collision.h"

using namespace wsp;

class Cursor : public Sprite {
	public:
		int Update(LayerManager* buttonManager);
		Cursor(int player);
	private:
		int player;
};

#endif