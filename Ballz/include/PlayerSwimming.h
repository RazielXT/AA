#pragma once

#include "stdafx.h"

class Player;

class PlayerSwimming
{
    friend class Player;

    Player* p;
    Ogre::RenderTarget *rttTex;
    Ogre::TexturePtr texture;
    Ogre::Camera* mWaterCam;
    Ogre::SceneNode* mWaterCamNode;

    void initWaterDepthReading();
    void readWaterDepth();

    Ogre::ParticleSystem* bubbles;
    Ogre::SceneNode* bubblesNode;

    Ogre::ParticleSystem* splash;
    Ogre::SceneNode* splashNode;

    Ogre::ParticleSystem* dust;
    Ogre::SceneNode* dustNode;

    void enteredWater();
    void leftWater();

public:

    PlayerSwimming(Player* player);

    void update();
};