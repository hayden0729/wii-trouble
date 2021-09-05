#ifndef TANK_BULLET_H
#define TANK_BULLET_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <math.h>
#include <vector>
#include <asndlib.h>
#include <mp3player.h>

#include "bullet_png.h"
#include "hit_pcm.h"

#include "collision.h"

using namespace wsp;

class Bullet : public Sprite {
	public:
		Bullet(int player, f32 radius, f32 speed, int life = 60 * 5);
		void Destroy(LayerManager* manager);
		int GetPlayer();
		int GetInitialSpeed();
		void Update(LayerManager* bulletManager, LayerManager* wallManager);
		void SetSpeed(f32 speed);
	private:
		int player;
		int life;
		f32 speed;
		f32 initialSpeed;
		f32 radius;
		void SetRadius(f32 radius);
};

#endif