#pragma once
#include "stdafx.h"

class ZipLine : public EventTask, public PlayerListener
{
public:

	struct LineProjState
	{
		Vector3 projPos;
		float cProgress;
		float left;
		float sqDistance;
	};

	struct ZipLinePoint
	{
		Vector3 pos;
		Vector3 dir;
	};

	struct SlideState
	{
		int mPoint;
		float mProgress;
		float speed;
		Vector3 currentDir;
	};

	ZipLine(std::vector<Ogre::Vector3> points);

	bool start();
	bool update(Ogre::Real tslf);

	void pressedKey(const OIS::KeyEvent &arg);

private:

	std::vector<ZipLinePoint> zipLine;
	SlideState sliding;

	bool placePointOnLine(Vector3& point);
	inline LineProjState getProjectedState(Ogre::Vector3& point, Ogre::Vector3& start, Ogre::Vector3& end);
	inline Vector3 getCurrentLinePos();

	
};