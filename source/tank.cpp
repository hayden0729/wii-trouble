#include "tank.h"
using namespace wsp;

// updates tank given player inputs (returns 0 if tank dies, 1 otherwise)
void Tank::Update(LayerManager* tankManager, LayerManager* wallManager, LayerManager* bulletManager, LayerManager* explosionManager) {
	// get inputs
	u16 buttonsHeld = WPAD_ButtonsHeld(player);
	u16 buttonsDown = WPAD_ButtonsDown(player);
	// variables for button holding for the sake of conciseness/readability (directions corrected for sideways wiimote)
	u16 upHeld = buttonsHeld & WPAD_BUTTON_RIGHT;
	u16 downHeld = buttonsHeld & WPAD_BUTTON_LEFT;
	u16 leftHeld = buttonsHeld & WPAD_BUTTON_UP;
	u16 rightHeld = buttonsHeld & WPAD_BUTTON_DOWN;
	// some other variables for animation and movement
	bool tankMoved = (upHeld || downHeld) && !(upHeld && downHeld); // true if tank will attempt to move forwards/backwards this frame (up or down is held, but not both)
	f32 radRotation = GetRotation() * 2.0 * (M_PI / 180.0); // convert rotation (in degrees/2) to radians
	// movement
	if (rightHeld) SetRotation( fmod((GetRotation() + turnSpeed),  180.0)); // go clockwise but stay in 0-180 (0-360 degrees)
	if (leftHeld) SetRotation( fmod((GetRotation() - turnSpeed),  180.0)); // go counterclockwise but stay in 0-180 (0-360 degrees)
	if (upHeld) Move(moveSpeed * cos(radRotation), moveSpeed * sin(radRotation));
	if (downHeld) Move(-moveSpeed * cos(radRotation), -moveSpeed * sin(radRotation));
	// animation
	animFrame = (animFrame + 1) % 3; // this is set to 0 every 3 calls; this way, the tank animates every 3 frames
	if (animFrame == 0) {
		if (tankMoved) Animate(upHeld); // will go forwards if upHeld, backwards if not (which implies downHeld because tankMoved is true)
		if (!tankMoved && rightHeld) Animate(true); // animate forwards for clockwise, backwards for counterclockwise (if tank isn't moving already)
		if (!tankMoved && leftHeld) Animate(false);
	}
	// wall collision check
	for (int i = 0; i < (int) wallManager->GetSize(); i++) {
		Quad* wall = (Quad*) wallManager->GetLayerAt(i);
		if (CollisionPossible((Sprite*) this, wall)) {
			std::vector<f32> collision = Collision((Sprite*) this, wall);
			if (collision[2] != 0) {
				f32 axisMultiplier = collision[2] / sqrt(pow(collision[0], 2) + pow(collision[1], 2));
				Move(collision[0] * axisMultiplier, collision[1] * axisMultiplier);
			};
		};
	};
	// bullet collision check
	for (int i = 0; i < (int) bulletManager->GetSize(); i++) {
		Bullet* bullet = (Bullet*) bulletManager->GetLayerAt(i);
		if (CollisionPossible((Sprite*) this, (Sprite*) bullet)) {
			std::vector<f32> collision = Collision((Sprite*) this, (Sprite*) bullet);
			if (collision[2] != 0) {
				bullet->Destroy(bulletManager);
                life--;
				if (!life) {
					Destroy(tankManager, explosionManager);
					return;
				}
			};
		};
	};
	// 2 (shoot bullet)
	if (buttonsDown & WPAD_BUTTON_2 && HasAmmo(bulletManager)) Shoot(wallManager, bulletManager);
}
void Tank::SetMoveSpeed(f32 moveSpeed) { this->moveSpeed = moveSpeed; }
void Tank::SetTurnSpeed(f32 turnSpeed) { this->turnSpeed = turnSpeed; }
f32 Tank::GetInitialMoveSpeed() { return initialMoveSpeed; }
f32 Tank::GetInitialTurnSpeed() { return initialTurnSpeed; }
// deletes the tank and removes it from the specified manager
void Tank::Destroy(LayerManager* tankManager, LayerManager* explosionManager) {
	if (explosionManager) explosionManager->Append(new Explosion(GetX() + GetWidth() / 2, GetY() + GetHeight() / 2));
    tankManager->Remove(this);
	delete this->GetImage();
    delete this;
}
// constructor
Tank::Tank(int player, int ammo) {
	this->player = player;
	this->ammo = ammo;
	Image* tankImg = new Image();
	tankImg->LoadImage(tanks_png); // tanks_png is an image that comes from an image include
	SetImage(tankImg, tankImg->GetWidth()/8, tankImg->GetHeight()/4); // image is an 8x4 grid
	SetFrame(player * 8); // 8 frames per player
	SetStretchWidth(.75);
	SetStretchHeight(.75);
	// tanks fit in a ~28x24 rectangle on their image (note: only width/height are relevant, offset is a libwiisprite feature that is not used)
	// note: this is pixels on image, not on ingame sprite, which is smaller due to its .75 multiplier above
	DefineCollisionRectangle(0, 0, 28, 24);
	// speeds default to these values, currently there is no reason for them to vary
	moveSpeed = 2.0;
	turnSpeed = 1.5;
	initialMoveSpeed = moveSpeed;
	initialTurnSpeed = turnSpeed;
    life = 1;
}
// returns true if the tank has fewer than (ammo) shots on the map
bool Tank::HasAmmo(LayerManager* bulletManager) {
	int activeBullets = 0;
	for (int i = 0; i < (int) bulletManager->GetSize(); i++) {
		Bullet* bullet = (Bullet*) bulletManager->GetLayerAt(i);
		if (bullet->GetPlayer() == player) activeBullets++;
	}
	return activeBullets < ammo;
}
// shoots a bullet
void Tank::Shoot(LayerManager* wallManager, LayerManager* bulletManager) {
	// spawn bullet at the front of the tank, subtracting speed to spawn it inside initially (it'll move before collision detection)
	f32 bulletRadius = 2.0;
	f32 bulletSpeed = 4.0;
	f32 radRotation = GetRotation() * 2.0 * (M_PI / 180.0); // convert rotation (in degrees/2) to radians
	f32 bulletX = GetX() + GetWidth() / 2 + cos(radRotation) * (12 + bulletRadius) - cos(radRotation) * bulletSpeed;
	f32 bulletY = GetY() + GetHeight() / 2 + sin(radRotation) * (12 + bulletRadius) - sin(radRotation) * bulletSpeed;
	Bullet* bullet = new Bullet(player, bulletRadius, bulletSpeed);
	bullet->SetPosition(bulletX - bullet->GetWidth() / 2, bulletY - bullet->GetHeight() / 2); // center bullet on bulletX and bulletY
	bullet->SetRotation(GetRotation());
	bulletManager->Append(bullet);
	// play sound
	SND_SetVoice(SND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT_LE, 44100, 0, (char*) shoot_pcm, shoot_pcm_size, 255, 255, NULL);
}
// animates the tank, moving its treads forwards or backwards
void Tank::Animate(bool forwards) {
	// for forwards, go backwards in the animation (add 7 frames each time so it loops back around to 1 behind)
	if (forwards) SetFrame(player * 8 + (GetFrame() + 7) % 8); // player * 8 sets tank color, getFrame % 8 sets movement frame
	else SetFrame(player * 8 + (GetFrame() + 1) % 8); // player * 8 sets tank color, getFrame % 8 sets movement frame
}