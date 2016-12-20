#pragma once
#include "stdafx.h"

struct PaitedItem;
class EditorControl;

struct EditorPainter
{
	EditorPainter(EditorControl* p) : parent(p) {}

	void init();

	void setItem(PaitedItem* item);
	PaitedItem* item = nullptr;

	void mousePressed();

	void mouseReleased();

	void mouseMoved();

	enum PaintMode
	{
		Add, Remove
	};

	void setMode(PaintMode mode);
	void setSize(float size);
	void setWeight(float w);

private:

	float size = 1;
	float weight = 1;
	PaintMode mode;

	bool applyPaint();
	bool painting = false;

	EditorControl* parent;
	Ogre::SceneNode* visual = nullptr;
};