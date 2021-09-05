#include "button.h"
using namespace wsp;

void Button::Select() {
	SetImage(overImg);
	SetStretchWidth(1.05);
	SetStretchHeight(1.05);
}

void Button::Deselect() {
	SetImage(normalImg);
	SetStretchWidth(1.0);
	SetStretchHeight(1.0);
}

int Button::GetID() { return id; }

Button::Button(int id, const unsigned char* normalImgData, const unsigned char* overImgData) {
	this->id = id;
	normalImg = new Image();
	normalImg->LoadImage(normalImgData);
	overImg = new Image();
	overImg->LoadImage(overImgData);
}