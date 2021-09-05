#ifndef TANK_BUTTON_H
#define TANK_BUTTON_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>

using namespace wsp;

class Button : public Sprite {
	public:
		void Select();
		void Deselect();
		int GetID();
		Button(int id, const unsigned char* normalImgData, const unsigned char* overImgData);
	private:
		int id;
		Image* normalImg;
		Image* overImg;
};

#endif