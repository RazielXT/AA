#include "stdafx.h"
#include "CrowPath.h"
#include "MathUtils.h"
#include "Player.h"

using namespace Ogre;

void CrowPathAnimations::refresh()
{
    if (mFlightTrack == nullptr)
    {
        //onground
    }
    else
    {
        //flight
        if (mTempTrack == nullptr)
        {
            Ogre::TransformKeyFrame key(0, 0);
            mFlightTrack->getInterpolatedKeyFrame(mFlightPos, &key);

            lastOr = key.getRotation();
            lastPos = key.getTranslate();
        }
        //lift/land/switch
        else
        {
            Ogre::TransformKeyFrame key0(0, 0);
            mTempTrack->getInterpolatedKeyFrame(mTempPos, &key0);

            Ogre::TransformKeyFrame key1(0, 0);

            //check if start of switching
            if (mOldFlightTrack)
                mOldFlightTrack->getInterpolatedKeyFrame(mOldFlightPos, &key1);
            else
                mFlightTrack->getInterpolatedKeyFrame(mFlightPos, &key1);

            lastOr = Quaternion::Slerp(animWeight, key0.getRotation(), key1.getRotation(), true);
            lastPos = MathUtils::lerp(key0.getTranslate(), key1.getTranslate(), animWeight);
        }
    }
}

void CrowPathAnimations::clearOldFlight()
{
    if (mOldFlightTrack)
    {
        if (mOldFlightTrack)
            delete mOldFlightTrack->getParent();

        mOldFlightTrack = nullptr;
        mOldFlightLenght = -2;
    }
}

void CrowPathAnimations::clearManuallyFlight()
{
    if (mFlightTrack)
    {
        auto name = mFlightTrack->getParent()->getName();
        Global::mSceneMgr->destroyAnimation(name);

        mFlightTrack = nullptr;
        mFlightLenght = -2;
    }
}

void CrowPathAnimations::clearTemp()
{
    if (mTempTrack)
    {
        auto name = mTempTrack->getParent()->getName();
        Global::mSceneMgr->destroyAnimation(name);

        mTempTrack = nullptr;
        mTempLenght = -2;
    }
}

void CrowPathAnimations::clear()
{
    if (mFlightTrack)
        delete mFlightTrack->getParent();

    mFlightTrack = nullptr;
    mFlightLenght = -2;

    clearOldFlight();

    clearTemp();
}

CrowPath::CrowPath()
{
}

CrowPath::~CrowPath()
{
}

bool CrowPath::update(Ogre::Real tslf, Ogre::SceneNode* mNode, Ogre::Quaternion& qOffset, Ogre::Vector3& pOffset)
{
    //update path/paths
    updateAnimState(tslf);

    animState.refresh();

    mNode->setPosition(getPosition() + pOffset);
    mNode->setOrientation(getOrientation()*qOffset);

    //update state
    if (state == Lifting)
    {
        if (animState.mTempPos == animState.mTempLenght)
        {
            animState.clearTemp();
            Global::DebugPrint("fly");
            state = Flying;
            return true;
        }
    }
    else if (state == Landing)
    {
        if (animState.mTempPos == animState.mTempLenght)
        {
            animState.clear();
            Global::DebugPrint("ground");
            state = Standing;
            return true;
        }
    }
    else if (state == SwitchFlying)
    {
        if (animState.mTempPos == animState.mTempLenght)
        {
            animState.clearTemp();
            Global::DebugPrint("fly");
            state = Flying;
            return true;
        }
    }
    else if (state == Walking)
    {
        if (animState.mFlightPos == animState.mFlightLenght)
        {
            animState.clearManuallyFlight();
            Global::DebugPrint("endWalk");
            state = Standing;
            return true;
        }
    }

    return false;
}

Ogre::Vector3 CrowPath::getPosition() const
{
    //current +blending
    return animState.lastPos;
}

Ogre::Quaternion CrowPath::getOrientation() const
{
    //current +blending
    return animState.lastOr;
}

void CrowPath::updateAnimState(Ogre::Real tslf)
{
    if (animState.mOldFlightTrack)
        animState.mOldFlightPos = std::fmod(animState.mOldFlightPos + tslf, animState.mOldFlightLenght);
    else if (animState.mFlightPos >= 0 && animState.animWeight > 0)
    {
        if (state == Walking)
            animState.mFlightPos = std::min(animState.mFlightPos + tslf, animState.mFlightLenght);
        else
            animState.mFlightPos = std::fmod(animState.mFlightPos + tslf*animState.animWeight, animState.mFlightLenght);

    }

    if (animState.mTempTrack)
    {
        animState.mTempPos = std::min(animState.mTempPos + tslf, animState.mTempLenght);
    }

    animState.animWeight = 1;

    //update weight
    if (state == Flying)
        animState.animWeight = 1;

    //from flight to temp
    if (state == Landing || (state == SwitchFlying && animState.mOldFlightTrack))
    {
        float startOff = animState.mTempPos;

        if (startOff < animWeightSize)
        {
            animState.animWeight = 1.0f - Math::Clamp(startOff / animWeightSize, 0.0f, 1.0f);
        }
        else
            animState.animWeight = 0;

        //at end of oldtrack blending delete it
        if (animState.mOldFlightTrack && animState.animWeight == 0)
            animState.clearOldFlight();
    }

    //from temp to flight
    if (state == Lifting || (state == SwitchFlying && !animState.mOldFlightTrack))
    {
        float rest = animState.mTempLenght - animState.mTempPos;

        if (rest < animWeightSize)
        {
            animState.animWeight = 1.0f - Math::Clamp(rest / animWeightSize, 0.0f, 1.0f);
        }
        else
            animState.animWeight = 0;
    }

    //Global::DebugPrint("state: " + std::to_string(state) + ",w: " + std::to_string(animState.animWeight), true);
}

void CrowPath::setWalkingAnim(Ogre::Vector3 pos)
{
    if (state == Standing)
    {
        //create land anim + init
        createWalkAnimation(pos);
        Global::DebugPrint("walk");
        state = Walking;
    }
    else
    {
        animState.lastPos = pos;
        animState.lastOr = Quaternion(Degree(Math::RangeRandom(0, 360)), Vector3(0, 1, 0));
        Global::DebugPrint("ground");

        state = Standing;
    }
}

void CrowPath::setLandingAnim(Ogre::Vector3 pos)
{
    if (state == Flying)
    {
        //create land anim + init
        createLandAnimation(animState.lastPos, animState.lastOr, pos);
        Global::DebugPrint("land");
        state = Landing;
    }
    else
    {
        animState.lastPos = pos;
        animState.lastOr = Quaternion(Degree(Math::RangeRandom(0, 360)), Vector3(0, 1, 0));
        Global::DebugPrint("ground");

        state = Standing;
    }
}

void CrowPath::setLiftingAnim(Ogre::Animation* flightAnim, float timePos)
{
    animState.mFlightTrack = flightAnim->getNodeTrack(0);
    animState.mFlightTrack->setUseShortestRotationPath(true);
    animState.mFlightTrack->_keyFrameDataChanged();
    animState.mFlightPos = timePos;
    animState.mFlightLenght = animState.mFlightTrack->getKeyFrame(animState.mFlightTrack->getNumKeyFrames() - 1)->getTime();

    if (state == Standing)
    {
        //create lift anim + init
        Ogre::TransformKeyFrame key(0, 0);
        animState.mFlightTrack->getInterpolatedKeyFrame(timePos, &key);

        createLiftAnimation(animState.lastPos, key.getTranslate(), key.getRotation());
        Global::DebugPrint("lift");
        state = Lifting;
    }
    else
    {
        Global::DebugPrint("fly");
        state = Flying;
    }
}

void CrowPath::setSwitchFlightAnim(Ogre::Animation* flightAnim, float timePos)
{
    animState.mOldFlightTrack = animState.mFlightTrack;
    animState.mOldFlightPos = animState.mFlightPos;
    animState.mOldFlightLenght = animState.mFlightLenght;

    animState.mFlightTrack = flightAnim->getNodeTrack(0);
    animState.mFlightPos = timePos;
    animState.mFlightLenght = animState.mFlightTrack->getKeyFrame(animState.mFlightTrack->getNumKeyFrames() - 1)->getTime();

    if (state == Flying)
    {
        //create lift anim + init
        Ogre::TransformKeyFrame key(0, 0);
        animState.mFlightTrack->getInterpolatedKeyFrame(timePos, &key);

        createSwitchFlightAnimation(key.getTranslate(), key.getRotation());
        Global::DebugPrint("switch");
        state = SwitchFlying;
    }
    else
    {
        Global::DebugPrint("fly");
        state = Flying;
    }
}

float CrowPath::getTempTimeLeft()
{
    return animState.mTempLenght - animState.mTempPos;
}

float CrowPath::getTempTime()
{
    return animState.mTempPos;
}

inline void fixSpline(Quaternion& rotation, Quaternion previous)
{
    float fCos = previous.Dot(rotation);
    if (fCos < 0.0f)
        rotation = -rotation;
}

void CrowPath::createWalkAnimation(Ogre::Vector3 endPos)
{
    static int counter = 0;

    float animSpeed = 1;// 0.15f;

    //create mTempTrack
    Animation* anim = Global::mSceneMgr->createAnimation("walking" + std::to_string(counter++), 2 / animSpeed);
    anim->setInterpolationMode(Animation::IM_SPLINE);
    anim->setRotationInterpolationMode(Animation::RIM_SPHERICAL);

    auto track = anim->createNodeTrack(0);
    track->setUseShortestRotationPath(true);

    Vector3 walkDir(endPos - animState.lastPos);
    Quaternion walkOr = MathUtils::quaternionFromDirNoPitch(walkDir);
    fixSpline(walkOr, animState.lastOr);

    Ogre::TransformKeyFrame* kf = track->createNodeKeyFrame(0);
    kf->setTranslate(animState.lastPos);
    kf->setRotation(animState.lastOr);

    kf = track->createNodeKeyFrame(1 / animSpeed);
    kf->setTranslate(animState.lastPos);
    kf->setRotation(walkOr);

    kf = track->createNodeKeyFrame(2 / animSpeed);
    kf->setTranslate(endPos);
    kf->setRotation(walkOr);

    animState.mFlightTrack = track;
    animState.mFlightPos = 0;
    animState.mFlightLenght = track->getKeyFrame(track->getNumKeyFrames() - 1)->getTime();

    animWeightSize = 1;
}

void CrowPath::createSwitchFlightAnimation(Ogre::Vector3 endPos, Ogre::Quaternion endOr)
{
    static int counter = 0;

    float l = animState.lastPos.distance(endPos);
    float animSpeed = l / 45.0f;

    //create mTempTrack
    Animation* anim = Global::mSceneMgr->createAnimation("switch" + std::to_string(counter++), animSpeed);
    anim->setInterpolationMode(Animation::IM_SPLINE);
    anim->setRotationInterpolationMode(Animation::RIM_SPHERICAL);

    auto track = anim->createNodeTrack(0);
    track->setUseShortestRotationPath(true);

    Ogre::TransformKeyFrame* kf = track->createNodeKeyFrame(0);
    kf->setTranslate(animState.lastPos);
    kf->setRotation(animState.lastOr);

    //+side offset
    Vector3 flyDir(endPos - animState.lastPos);
    auto prev = animState.lastOr;
    auto rot = MathUtils::quaternionFromDir(flyDir);
    fixSpline(rot, prev);
    kf = track->createNodeKeyFrame(animSpeed/3.0f);
    Vector3 p1(animState.lastPos + flyDir / 3.0f);
    kf->setTranslate(p1);
    kf->setRotation(rot);
    prev = rot;

    prev = animState.lastOr;
    rot = MathUtils::quaternionFromDir(flyDir);
    fixSpline(rot, prev);
    kf = track->createNodeKeyFrame(animSpeed / 2.0f);
    Vector3 p2(animState.lastPos + flyDir * (2.0f/ 3.0f));
    kf->setTranslate(p2);
    kf->setRotation(rot);
    prev = rot;

    prev = animState.lastOr;
    rot = MathUtils::quaternionFromDir(flyDir);
    fixSpline(rot, prev);
    kf = track->createNodeKeyFrame(animSpeed);
    kf->setTranslate(endPos);
    kf->setRotation(rot);
    prev = rot;

    animState.mTempTrack = track;
    animState.mTempPos = 0;
    animState.mTempLenght = track->getKeyFrame(track->getNumKeyFrames() - 1)->getTime();

    animWeightSize = 1;
}

void CrowPath::createLandAnimation(Vector3 startPos, Ogre::Quaternion startOr, Vector3 end)
{
    static int counter = 0;

    float l = startPos.distance(end);
    float animLen = l / 40.0f;
    float landTime = 1.0f;

    //create mTempTrack
    Animation* anim = Global::mSceneMgr->createAnimation("landing" + std::to_string(counter++), animLen + landTime);
    anim->setInterpolationMode(Animation::IM_SPLINE);
    anim->setRotationInterpolationMode(Animation::RIM_SPHERICAL);

    auto track = anim->createNodeTrack(0);
    track->setUseShortestRotationPath(true);

    Vector3 landDir(end - startPos);
    landDir.normalise();
    Vector3 landPrepPos = end - landDir*5 + Vector3(0, 0.5f, 0);
    Vector3 halfPos = (landPrepPos + startPos) / 2 - Vector3(0, 1, 0);
    Quaternion neutralDir = MathUtils::quaternionFromDirNoPitch(landDir);

    Ogre::TransformKeyFrame* kf = track->createNodeKeyFrame(0);
    kf->setTranslate(startPos);
    kf->setRotation(startOr);

    kf = track->createNodeKeyFrame(animLen/2.0f);
    kf->setTranslate(halfPos);
    kf->setRotation(MathUtils::quaternionFromDir(landPrepPos - startPos));

    kf = track->createNodeKeyFrame(animLen);
    kf->setTranslate(landPrepPos);
    kf->setRotation(neutralDir);

    kf = track->createNodeKeyFrame(animLen + landTime);
    kf->setTranslate(end);
    kf->setRotation(neutralDir);

    animState.mTempTrack = track;
    animState.mTempPos = 0;
    animState.mTempLenght = track->getKeyFrame(track->getNumKeyFrames() - 1)->getTime();

    animWeightSize = 1;
}

void CrowPath::createLiftAnimation(Vector3 start, Vector3 endPos, Ogre::Quaternion endOr)
{
    static int counter = 0;

    float l = start.distance(endPos);
    float liftTime = 0.5f;
    float animLen = l / 30.0f;

    //create mTempTrack
    Animation* anim = Global::mSceneMgr->createAnimation("lifting" + std::to_string(counter++), animLen + liftTime);
    anim->setInterpolationMode(Animation::IM_SPLINE);
    anim->setRotationInterpolationMode(Animation::RIM_SPHERICAL);

    auto track = anim->createNodeTrack(0);
    track->setUseShortestRotationPath(true);

    Vector3 flightDir(endPos - start);
    flightDir.normalise();
    Vector3 jumpPos = start + flightDir + Vector3(0, 2.5f, 0);
    Vector3 halfPos = (jumpPos + endPos) / 2 - Vector3(0, 1, 0);


    Ogre::TransformKeyFrame* kf = track->createNodeKeyFrame(0);
    kf->setTranslate(start);
    kf->setRotation(animState.lastOr);

    kf = track->createNodeKeyFrame(liftTime);
    kf->setTranslate(jumpPos);
    kf->setRotation(MathUtils::quaternionFromDir(halfPos - jumpPos));

    kf = track->createNodeKeyFrame(liftTime + animLen / 3.0f);
    kf->setTranslate(halfPos);
    kf->setRotation(MathUtils::quaternionFromDir(endPos - halfPos));

    kf = track->createNodeKeyFrame(liftTime + animLen);
    kf->setTranslate(endPos);
    kf->setRotation(endOr);

    animState.mTempTrack = track;
    animState.mTempPos = 0;
    animState.mTempLenght = track->getKeyFrame(track->getNumKeyFrames() - 1)->getTime();

    animWeightSize = 1;
}