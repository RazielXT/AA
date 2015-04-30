#pragma once

#include "stdafx.h"
#include "Slide.h"
#include <future>
#include <vector>

struct TargetSlideInfo
{
    Slide* targetSlide = nullptr;
    float targetSlidePosOffset;
    Ogre::Vector3 targetSlidePos;
};

class SlidesAutoTargetAsync
{
public:

    SlidesAutoTargetAsync();
    ~SlidesAutoTargetAsync();

    void updateAutoTarget(Vector3 pos, Vector3 dir, float tslf, float rayDistance = 30, Slide* ignoredSlide = nullptr);
    void hideAutoTarget();

    bool pressedAction();

    TargetSlideInfo targetInfo;

    std::vector<Slide*> loadedSlides;

private:

    Slide* preparedSlide;
    float preparedSlideOffset;
    Ogre::Vector3 preparedSlidePos;

    Ogre::BillboardSet* targetBillboardSet;
    Ogre::SceneNode* billboardNode;

    float targetTimer;

    std::future<bool> targetResult;
    bool getTargetSlideFunc(Vector3 pos, Vector3 dir, float rayDistance, Slide* ignoredSlide);

    bool getTargetSlideRay(Vector3 pos, Vector3 dir, float rayDistance, Slide* ignoredSlide);
    bool getTargetSlideTouch(Vector3 pos, Vector3 dir, Slide* ignoredSlide);
};