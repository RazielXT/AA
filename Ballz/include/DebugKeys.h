#include "stdafx.h"
#include "InputListener.h"
#include "PostProcessMgr.h"
#include "EffectsTasks.h"

class DebugKeys : public InputListener
{
public:
    DebugKeys()
    {
        executionState = UNDEFINED;
    }

    void pressedKey(const OIS::KeyEvent &arg)
    {
        auto postProcMgr = Global::mPPMgr;

        switch (arg.key)
        {
        case OIS::KC_O:
        {
            Global::timestep -= 0.1;
        }
        break;

        case OIS::KC_P:
        {
            Global::timestep += 0.1;
        }
        break;

        case OIS::KC_B:
        {
            auto task = new SwitchColorSchemeFx("0.8,0.95,1.05,2");
            if (task->start())
                Global::mEventsMgr->addTask(task);
            break;
        }
        case OIS::KC_N:
        {
            auto task = new SwitchColorSchemeFx("1.0,0.95,0.85,2");
            if (task->start())
                Global::mEventsMgr->addTask(task);
            break;
        }
        case OIS::KC_Y:
            postProcMgr->ColouringShift.x += 0.02;
            break;

        case OIS::KC_U:
            postProcMgr->ColouringShift.y += 0.02;
            break;

        case OIS::KC_I:
            postProcMgr->ColouringShift.z += 0.02;
            break;

        case OIS::KC_H:
            postProcMgr->ColouringShift.x -= 0.02;
            break;

        case OIS::KC_J:
            postProcMgr->ColouringShift.y -= 0.02;
            break;

        case OIS::KC_K:
            postProcMgr->ColouringShift.z -= 0.02;
            break;

        case OIS::KC_L:
            if (Global::gameMgr->gameState == MENU)
            {
                Global::gameMgr->switchToLevel(4);
            }
            else
                continueExecution = false;

            break;

        case OIS::KC_NUMPAD7:
            postProcMgr->bloomStrDep.y += 0.25;
            break;

        case OIS::KC_NUMPAD8:
            postProcMgr->bloomStrDep.y -= 0.25;
            break;

        case OIS::KC_NUMPAD4:
            postProcMgr->bloomStrDep.x += 0.25;
            break;

        case OIS::KC_NUMPAD5:
            postProcMgr->bloomStrDep.x -= 0.25;
            break;

        case OIS::KC_NUMPAD0:
            postProcMgr->resetValues();
            break;
        }
    }
};