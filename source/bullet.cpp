#include "bullet.h"
using namespace wsp;

Bullet::Bullet(int player, f32 radius, f32 speed, int life) { // default lifespan is 8 seconds (60 fps)
	Image* bulletImg = new Image();
	bulletImg->LoadImage(bullet_png); // bullet_png is an image that comes from an image include
	SetImage(bulletImg);
	this->player = player;
	this->life = life;
	this->speed = speed;
	this->initialSpeed = speed;
	SetRadius(radius);
}
void Bullet::Destroy(LayerManager* manager) { // deletes the bullet and removes it from the specified manager
	manager->Remove(this);
	delete this->GetImage();
	delete this;
}
void Bullet::SetSpeed(f32 speed) { this->speed = speed; }
int Bullet::GetPlayer() { return player; }
int Bullet::GetInitialSpeed() { return initialSpeed; }
void Bullet::Update(LayerManager* bulletManager, LayerManager* wallManager) { // update life, movement, and collision (returns 0 if bullet dies, 1 otherwise)
	// decrement life
	life--;
	// out of bounds check (kill if out of bounds)
	bool outLeft = GetX() + GetWidth() / 2 + GetWidth() * GetStretchWidth() / 2 < 0;
	bool outRight = GetX() > 640;
	bool outTop = GetY() + GetHeight() / 2 + GetHeight() * GetStretchHeight() / 2 < 0;
	bool outBot = GetY() > 480;
	if (outLeft || outRight || outTop || outBot) life = 0;
	// if out of life, destroy
	if (!life) {
		Destroy(bulletManager);
		return;
	}
	// movement
	f32 radRotation = GetRotation() * 2.0 * (M_PI / 180.0); // convert rotation (in degrees/2) to radians
	Move(speed * cos(radRotation), speed * sin(radRotation));
	// collision check
	for (int wallNum = 0; wallNum < (int) wallManager->GetSize(); wallNum++) {
		Quad* wall = (Quad*) wallManager->GetLayerAt(wallNum);
		if (CollisionPossible(this, wall)) {
			std::vector<f32> collision = Collision(this, wall);
			if (collision[2] != 0) {
				// move bullet out of wall
				f32 axisMultiplier = collision[2] / sqrt(pow(collision[0], 2) + pow(collision[1], 2));
				Move(collision[0] * axisMultiplier, collision[1] * axisMultiplier);
				// get the penetration vector's angle (divided by 2 as that is the standard in libwiisprite) and flip the bullet's angle around that angle
				f32 penetrationAngle = atan2(collision[1], collision[0]) * (180.0 / M_PI) / 2;
				SetRotation(fmod(-GetRotation() + 2 * penetrationAngle + 90, 180)); // add 90 to reverse direction
				// make hit sound
				SND_SetVoice(SND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT_LE, 44100, 0, (char*) hit_pcm, hit_pcm_size, 255, 255, NULL);
			}
		}
	}
}
void Bullet::SetRadius(f32 radius) { // set radius and change stretch/collision to adapt
	this->radius = radius;
	SetStretchWidth(radius * 2 / GetImage()->GetWidth());
	SetStretchHeight(radius * 2 / GetImage()->GetHeight());
	DefineCollisionRectangle(0, 0, radius * 2, radius * 2);
}