#ifndef TANK_TANK_H
#define TANK_TANK_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <wiiuse/wpad.h>
#include <math.h>
#include <vector>
#include <asndlib.h>
#include <mp3player.h>

#include "tanks_png.h"
#include "shoot_pcm.h"

#include "collision.h"
#include "bullet.h"
#include "explosion.h"

using namespace wsp;

class Tank : public Sprite {
	public:
		// updates tank given player inputs
		void Update(LayerManager* tankManager, LayerManager* wallManager, LayerManager* bulletManager, LayerManager* explosionManager);
        void Destroy(LayerManager* tankManager, LayerManager* explosionManager = NULL);
		void SetMoveSpeed(f32 moveSpeed);
		void SetTurnSpeed(f32 turnSpeed);
		f32 GetInitialMoveSpeed();
		f32 GetInitialTurnSpeed();
		Tank(int player, int ammo);
	private:
		int player;
		int animFrame;
		f32 moveSpeed;
		f32 turnSpeed;
		f32 initialMoveSpeed;
		f32 initialTurnSpeed;
		int ammo;
        int life;
		// returns true if the tank has fewer than (ammo) shots on the map
		bool HasAmmo(LayerManager* bulletManager);
		// shoots a bullet
		void Shoot(LayerManager* wallManager, LayerManager* bulletManager);
		// animates the tank, moving its treads forwards or backwards
		void Animate(bool forwards);
};

#endif