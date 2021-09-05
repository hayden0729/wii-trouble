#include <stdlib.h>
#include <gccore.h>
#include <fat.h>
#include <wiisprite.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>
#include <mp3player.h>

#include "button.h"
#include "cursor.h"
#include "bullet.h"
#include "tank.h"
#include "explosion.h"
#include "map.h"

#include "background_png.h"
#include "logo_png.h"
#include "btn_2_players_png.h"
#include "btn_2_players_over_png.h"
#include "btn_3_players_png.h"
#include "btn_3_players_over_png.h"
#include "btn_4_players_png.h"
#include "btn_4_players_over_png.h"
#include "btn_exit_png.h"
#include "btn_exit_over_png.h"

#include "music_mp3.h"
#include "shoot_pcm.h"
#include "explode_pcm.h"

// libwisprite namespace
using namespace wsp;

// log file for debugging
//FILE* logFile;

void ClearLayerManager(LayerManager* manager) {
	while (true) {
		Layer* layer = manager->GetLayerAt(0);
		if (layer) {
			manager->Remove(layer);
			delete layer;
		}
		else break;
	}
}

int main(int argc, char** argv) {
	
	// video initialization
	GameWindow* gwd = new GameWindow();
	gwd->InitVideo();
	gwd->SetBackground((GXColor){ 0, 0, 0, 255 });

	// audio initialization
	ASND_Init();
	MP3Player_Init();

	// file initialization
	fatInitDefault();
	//logFile = fopen("wiitrouble.log", "w");

	// wiimote initialization
	WPAD_Init();
	WPAD_SetVRes(WPAD_CHAN_ALL, gwd->GetWidth(), gwd->GetHeight());
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);

	// a few constants for manager sizes/map creation
	const int tankAmmo = 6; // tanks have 6 shots
	const int mapWidth = 8;
	const int mapHeight = 6;

	// create layer managers & map
	LayerManager* cursorManager = new LayerManager(4);
	LayerManager* tankManager = new LayerManager(4);
	LayerManager* bulletManager = new LayerManager(4 * tankAmmo + 1); // max of (number of tanks)*ammo bullets on the map at once, plus one decorative bullet in the menu
	LayerManager* explosionManager = new LayerManager(4);
	LayerManager* wallManager = new LayerManager(mapWidth * mapHeight * 2 + mapWidth + mapHeight); // north/west side of each cell (2wh), plus east/bottom borders (w+h)
	LayerManager* buttonManager = new LayerManager(4);
	Map* map = NULL;

	// create background & logo
	Image* backgroundImg = new Image();
	backgroundImg->LoadImage(background_png);
	Sprite* background = new Sprite();
	background->SetImage(backgroundImg);

	Image* logoImg = new Image();
	logoImg->LoadImage(logo_png);
	Sprite* logo = new Sprite();
	logo->SetImage(logoImg);
	logo->SetPosition((gwd->GetWidth() - logo->GetWidth()) / 2, 64); // arbitrary numbers for logo positioning

	// create buttons
	for (int i = 0; i < 4; i++) {
		Button* button;
		if (i == 0) button = new Button(i + 1, btn_2_players_png, btn_2_players_over_png); // these names have "btn" in front so that c++ doesn't think they're 
		if (i == 1) button = new Button(i + 1, btn_3_players_png, btn_3_players_over_png);
		if (i == 2) button = new Button(i + 1, btn_4_players_png, btn_4_players_over_png);
		if (i == 3) button = new Button(i + 1, btn_exit_png, btn_exit_over_png);
		button->SetPosition(166, 192 + 68 * i); // arbitrary numbers for button positioning
		buttonManager->Append(button);
	}

	// create cursors
	for (int player = 0; player < 4; player++) {
		cursorManager->Append(new Cursor(player));
	}

	// initialize a few variables that will need to be kept between frames
	bool inMenu = true; // indicates whether or not the menu is active (false if game is being played)
	int tankCount = 0; // number of tanks to be spawned at the beginning of each game (defaults to 0 but must be selected on the menu before any games are started)

	// main loop
	while (1) {

		// music
		if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(music_mp3, music_mp3_size, NULL);
		
		// this is set to true when an exit condition is met to indicate that this will be the final frame
		bool lastFrame = false;

		// player inputs
		WPAD_ScanPads();
		for (int player = 0; player < 4; player++) {
			// exit (home)
			if (WPAD_ButtonsDown(player) & WPAD_BUTTON_HOME) lastFrame = true;
			// go to menu (+)
			if (!inMenu && WPAD_ButtonsDown(player) & WPAD_BUTTON_PLUS) {
				// play explosion sound and go to menu
				SND_SetVoice(SND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT_LE, 44100, 0, (char*) explode_pcm, explode_pcm_size, 255, 255, NULL);
				inMenu = !inMenu;
				// delete all tanks, bullets, and explosions & reset map
				ClearLayerManager(tankManager);
				ClearLayerManager(bulletManager);
				ClearLayerManager(explosionManager);
				if (map) {
					map->Destroy(wallManager);
					map = NULL;
				};
			}
		}

		// menu bullet spawning
		if (inMenu && !bulletManager->GetSize()) { // spawn a bullet when there are no more
			f32 bulletRadius = 2.0;
			f32 bulletSpeed = 2.0;
			Bullet* bullet = new Bullet(0, bulletRadius, bulletSpeed); // 0 for no player
			bullet->SetPosition(logo->GetX() + 258, logo->GetY() + 16); // center bullet on turret in logo (arbitrary position)
			bullet->SetRotation(135); // facing up
			bulletManager->Append(bullet);
		}

		// new game if 1 or fewer tanks remain and all explosions have died
		if (!inMenu && tankManager->GetSize() <= 1 && !explosionManager->GetSize()) {
			// delete all tanks, bullets, and explosions
			ClearLayerManager(tankManager);
			ClearLayerManager(bulletManager);
			ClearLayerManager(explosionManager);
			// reset map & spawn new tanks
			if (map) map->Destroy(wallManager);
			map = new Map(gwd->GetWidth(), gwd->GetHeight(), 8, 6, 8); // 8x6 map w/ 8-pixel-thick walls that takes up the whole screen
			map->GenerateWalls(wallManager);
			map->SpawnTanks(tankCount, tankManager, tankAmmo);
		}

		// update rotating walls
		if (map && wallManager->GetSize()) for (int i = 0; i < map->GetSpinningWalls(); i++) {
			Quad* spinningWall = (Quad*) wallManager->GetLayerAt(i);
			f32 spinningWallSpeed = .5;
			if (explosionManager->GetSize()) spinningWallSpeed /= 2; // slow mo if explosions exist
			spinningWall->SetRotation(fmod(spinningWall->GetRotation() + spinningWallSpeed,  180.0));
		}

		// update bullets
		for (int i = 0; i < (int) bulletManager->GetSize(); i++) {
			Bullet* bullet = (Bullet*) bulletManager->GetLayerAt(i);
			if (explosionManager->GetSize()) { // slow mo if explosions exist
				bullet->SetSpeed(bullet->GetInitialSpeed() / 2);
			}
			else {
				bullet->SetSpeed(bullet->GetInitialSpeed());
			}
			bullet->Update(bulletManager, wallManager);
			if (!bullet) i--; // repeat index because object died
		}

		// update tanks
		for (int i = 0; i < (int) tankManager->GetSize(); i++) {
			Tank* tank = (Tank*) tankManager->GetLayerAt(i);
			if (explosionManager->GetSize()) { // slow mo if explosions exist
				tank->SetMoveSpeed(tank->GetInitialMoveSpeed() / 2);
				tank->SetTurnSpeed(tank->GetInitialTurnSpeed() / 2);
			}
			else {
				tank->SetMoveSpeed(tank->GetInitialMoveSpeed());
				tank->SetTurnSpeed(tank->GetInitialTurnSpeed());
			}
			tank->Update(tankManager, wallManager, bulletManager, explosionManager);
			if (!tank) i--; // repeat index because object died
		}

		// update explosions
		for (int i = 0; i < (int) explosionManager->GetSize(); i++) {
			Explosion* explosion = (Explosion*) explosionManager->GetLayerAt(i);
			explosion->Update(explosionManager);
			if (!explosion) i--; // repeat index because object died
		}

		// deselect buttons (re-selected if cursors are still hovering in cursor update)
		for (int i = 0; i < (int) buttonManager->GetSize(); i++) {
			((Button*) buttonManager->GetLayerAt(i))->Deselect();
		}

		// update cursors
		for (int i = 0; i < (int) cursorManager->GetSize(); i++) {
			int buttonPressed = ((Cursor*) cursorManager->GetLayerAt(i))->Update(buttonManager);
			// press buttons on menu
			if (inMenu) {
				if (buttonPressed) SND_SetVoice(SND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT_LE, 44100, 0, (char*) shoot_pcm, shoot_pcm_size, 255, 255, NULL); // any: shoot sound
				if (buttonPressed >= 1 && buttonPressed <= 3) { // buttons 1-3: start game
					tankCount = buttonPressed + 1;
					inMenu = !inMenu;
				}
				if (buttonPressed == 4) { // button 4: exit
					lastFrame = true;
				}
			}
		}

		// render this frame and move on to the next
		background->Draw(0, 5); // background is drawn at y=5 because for some reason if it's drawn at (0, 0) it's 5 px above everything else
		bulletManager->Draw(0, 0);
		if (inMenu) {
			logo->Draw(0, 0);
			buttonManager->Draw(0, 0);
			cursorManager->Draw(0, 0);
		}
		else  {
			wallManager->Draw(0, 0);
			tankManager->Draw(0, 0);
			explosionManager->Draw(0, 0);
		}
		gwd->Flush();

		// exit if that was the final frame
		if (lastFrame) {
			fatUnmount(0);
			gwd->StopVideo();
			exit(0);
		}

	}

	return 0;

}