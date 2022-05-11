#include "explosion.h"
using namespace wsp;

void Explosion::Update(LayerManager* explosionManager) {
    life--;
	if (!life) {
		Destroy(explosionManager);
		return;
	}
    if (!(life % frameLength)) SetFrame(GetFrame() + 1); // move to the next frame after each frame length
}
void Explosion::Destroy(LayerManager* explosionManager) {
    explosionManager->Remove(this);
	delete this->GetImage();
    delete this;
}
Explosion::Explosion(f32 x, f32 y) {
	Image* explosionImg = new Image();
	explosionImg->LoadImage(explosion_png); // explosion_png is an image that comes from an image include
	SetImage(explosionImg, explosionImg->GetWidth()/5, explosionImg->GetHeight()/5); // image is a 5x5 of explosions
    SetPosition(x - GetWidth() / 2, y - GetWidth() / 2);
    frameLength = 5; // each frame of animation lasts for frameLength frames of the game
    life = 5 * 5 * frameLength;
	// play explosion sound
	SND_SetVoice(SND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT_LE, 44100, 0, (char*) explode_pcm, explode_pcm_size, 255, 255, NULL);
}