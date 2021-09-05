#include "cursor.h"
using namespace wsp;

int Cursor::Update(LayerManager* buttonManager) { // updates cursor and returns id of button pressed (0 if none)
    ir_t ir;
	WPAD_IR(player, &ir);
	SetPosition(ir.sx-WSP_POINTER_CORRECTION_X, ir.sy-WSP_POINTER_CORRECTION_Y); // use sx and sy (s for smoothed) for best ir detection; sx/sy require offsets
	Move(-((f32)GetWidth()/2), -((f32)GetHeight()/2)); // center by moving up/left by half cursor height/width
	SetRotation(ir.angle/2); // set angle, must be divided by 2 to translate correctly
	// check button selection
	for (int i = 0; i < (int) buttonManager->GetSize(); i++) {
		Button* button = (Button*) buttonManager->GetLayerAt(i);
		if (CollisionPossible(this, button)) {
			std::vector<f32> collision = Collision(this, button);
			if (collision[2] != 0) {
				// there is a collision between cursor and button, so select button
				button->Select();
				if (WPAD_ButtonsDown(player) & WPAD_BUTTON_A) { // a is pressed, so press button
					return button->GetID();
				}
			}
		}
	}
	return 0;
}

Cursor::Cursor(int player) {
	this->player = player;
	// set image; the image contains 4 cursors, so use setframe to set it to the appropriate one for this player
	Image* cursorImg = new Image();
	cursorImg->LoadImage(cursors_png); // cursors_png is an image that comes from an image include
	SetImage(cursorImg, cursorImg->GetWidth()/4, cursorImg->GetHeight()); // image is a 4x1 of cursors
	SetFrame(player);
	DefineCollisionRectangle(0, 0, 4, 4); // the collision rectangle for cursors is small, as it's just at the fingertip
}