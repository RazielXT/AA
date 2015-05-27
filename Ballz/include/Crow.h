#pragma once

#include "stdafx.h"

class CrowsPack;


class Crow
{
	friend class CrowFlight;
	friend class CrowLanding;

public:

	Crow(Ogre::SceneNode* node, Ogre::Entity* entity) : mNode(node), mEntity(entity) {};
	~Crow();

	bool update(Ogre::Real tslf);

	bool readyToFly() const;
	bool readyToLand() const;

	void flyTo(Ogre::Animation* flightAnim);
	void landTo(Ogre::Vector3 pos);

protected:

	void createLandingAnim();
	void createLiftingAnim();

	enum {	OnGround, Flying, Landing, Lifting, None	} state = None;
	
	Ogre::SceneNode* mNode;
	Ogre::Entity* mEntity;
};