#pragma once

#include "stdafx.h"
#include "PlayerStateInfo.h"

class Player;
class Slide;
class SlidesAutoTargetAsync;

class PlayerSliding
{
	friend class Player;
	friend class PlayerAbilities;

	Player* p;
	Slide* currentSlide = nullptr;
	SlidesAutoTargetAsync* slidesAutoTarget;

	Ogre::BillboardSet* targetBillboardSet;
	Ogre::SceneNode* billboardNode;

public:

	PlayerSliding(Player* player);
	~PlayerSliding();

	float autoAttachUnavailableTimer = 0;
	void slideStarted(Slide* slide);
	void addLoadedSlide(Slide* slide);

	void update(float tslf, bool readyToSlide);

	void pressedKey(const OIS::KeyEvent &arg);
	void releasedKey(const OIS::KeyEvent &arg);
	void movedMouse(const OIS::MouseEvent &e);

	void hideSlideTarget();
	bool showPossibleSlideTarget();

	bool foundTarget();
	void portToTarget();
};