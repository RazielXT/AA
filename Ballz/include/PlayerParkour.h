#pragma once

#include "stdafx.h"

class Player;

class PlayerParkour
{
    friend class Player;

    Player* p;
    OgreNewt::Body* body;

    bool possibleWalljump = false;
    bool allowWalljump = true;
    bool tryWallJump();

    float rolling;

    OgreNewt::BallAndSocket* wallrunJoint;
    Ogre::Vector3 wall_normal, wallrunCurrentDir;
    float wallrunSide, wallrunTimer, wallrunSpeed;
    bool getWallrunInfo(float side, Ogre::Vector3 frontDir, float testDegree = 90);

    void doWalljump();

public:

    void wallrun_callback(OgreNewt::Body* me, float timeStep, int threadIndex)
    {
        me->setVelocity(wallrunCurrentDir*wallrunSpeed*wallrunTimer - wall_normal - Ogre::Vector3(0, 1, 0)*wallrunTimer);
    }

    PlayerParkour(Player* player);

    bool spacePressed();
    void hitGround();

    inline bool isRolling()
    {
        return rolling > 0;
    }
    void doRoll();
    void updateRolling(float tslf);

    bool tryWallrun();
    void updateWallrunning();
    void stopWallrun();
    bool updateParkourPossibility();
};