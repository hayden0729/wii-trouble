#ifndef TANK_EXPLOSION_H
#define TANK_EXPLOSION_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <asndlib.h>
#include <mp3player.h>

#include "explosion_png.h"
#include "explode_pcm.h"

using namespace wsp;

class Explosion : public Sprite {
	public:
		void Update(LayerManager* explosionManager);
        void Destroy(LayerManager* explosionManager);
		Explosion(f32 x, f32 y);
	private:
		int life;
		int frameLength; // length of time in game frames for which each frame of the explosion animation should be played
};

#endif