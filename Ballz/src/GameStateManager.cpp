#include "stdafx.h"
#include "GameStateManager.h"
#include "Player.h"
#include "levelsInit.h"
#include "DebugKeys.h"
#include "VolumeDetection.h"
#include "GUtils.h"
#include "Energy.h"
#include "Gate.h"

GameStateManager::GameStateManager(Ogre::Camera* cam, Ogre::RenderSystem* rs)
{
    gameConfig.loadCfg();

    myMenu = new GuiOverlay(&gameConfig, cam, Global::mWindow, rs, Global::soundEngine);

    wMaterials.init();

    LevelInfo info;
    info.path = "../../media/menu/";
    info.sceneFile = "menu.scene";
    info.init = createMenuLevel;
    info.fogColor = Ogre::ColourValue(0.5f, 0.55f, 0.65f, 0.5f);
    info.fogStartDistance = 80;
    info.fogEndDistance = 150;
    info.ColorShift = Ogre::Vector4(1.08f, 1.12f, 1.16f, 1);
    info.ContSatuSharpNoise = 0;
    info.ambientColor = ColourValue(0.35f, 0.35f, 0.35f);
    info.skyboxName = "TCENoonSkyBox";
    info.bloomStr = 1;
    info.bloomDepth = 0.35f;
    levels[0] = info;

    info.name = "Park";
    info.path = "../../media/park/";
    info.sceneFile = "park.scene";
    info.init = createLevelTuto;
    info.bloomStr = 1.5f;
    info.bloomDepth = 0.5f;
    levels[1] = info;

    info.name = "Caves";
    info.path = "../../media/";
    info.sceneFile = "caves.scene";
    info.init = createLevel1_1;
    levels[2] = info;

    info.name = "Valley";
    info.path = "../../media/valley/";
    info.sceneFile = "valley.scene";
    info.init = createLevel2;
    levels[3] = info;

    info.name = "Test1";
    info.path = "../../media/testLvl/";
    info.sceneFile = "test.scene";
    info.init = createTestLevel;
    info.fogColor = Ogre::ColourValue(0.5f, 0.55f, 0.65f, 0.5f);
    info.fogStartDistance = 80;
    info.fogEndDistance = 150;
    info.ColorShift = Ogre::Vector4(1.0f, 1.0f, 1.02f, 1);
    info.ContSatuSharpNoise = Ogre::Vector4(0, 0, 0, 0);
    info.ambientColor = ColourValue(0.35f, 0.35f, 0.35f);
    info.skyboxName = "TCENoonSkyBox";
    info.bloomStr = 1.1f;
    info.bloomDepth = 0.38f;
    info.bloomAdd = 0.45f;
    info.bloomSize = 1.5f;
    levels[4] = info;

    info.name = "Test2";
    info.path = "../../media/testLvl2/";
    info.sceneFile = "test.scene";
    info.init = createTestLevel2;
    levels[5] = info;

    dbg = new DebugKeys();
    dbg->registerInputListening();
}

GameStateManager::~GameStateManager()
{
    delete dbg;
    delete myMenu;
}

void GameStateManager::switchToMainMenu()
{
    switchToLevel(0);
}

LevelInfo* GameStateManager::getCurrentLvlInfo()
{
    return &levels[lastLVL];
}

LevelInfo* GameStateManager::getLvlInfo(int id)
{
    if (levels.find(id) != levels.end())
        return &levels[id];

    return nullptr;
}

void GameStateManager::switchToLevel(int lvl)
{
    if (lvl == 0)
        myMenu->setMainMenu();
    else if (gameState == PAUSE || gameState == MENU)
        myMenu->clearMenu();

    lastLVL = lvl;
    gameState = lvl == 0 ? MENU : GAME;

    if (Global::player)
    {
        delete Global::player;
        Global::player = nullptr;
    }

    clearLevel();

    if (gameState == GAME)
    {
        Global::player = new Player(&wMaterials);
    }

    dbg->reloadVariables();

    auto& lvlInfo = levels[lvl];

    SceneParser::loadScene(lvlInfo.path + lvlInfo.sceneFile);
    lvlInfo.init();
    Global::gameMgr->materialEdits.applyChanges();
    Global::gameMgr->geometryMgr->update();
    SceneCubeMap::renderAll();

    Global::mPPMgr->fadeIn(Vector3(0, 0, 0), 2.f, true);
}

void GameStateManager::reloadSceneSettings()
{
    auto& lvlInfo = levels[lastLVL];

    Global::mSceneMgr->setAmbientLight(lvlInfo.ambientColor);
    PostProcessMgr* postProcMgr = Global::mPPMgr;
    postProcMgr->vars.ColouringShift = lvlInfo.ColorShift;
    postProcMgr->vars.ContSatuSharpNoise = lvlInfo.ContSatuSharpNoise;
    postProcMgr->vars.bloomStrDepAddSize.z = lvlInfo.bloomAdd;
    postProcMgr->vars.bloomStrDepAddSize.x = lvlInfo.bloomStr;
    postProcMgr->vars.bloomStrDepAddSize.y = lvlInfo.bloomDepth;
    postProcMgr->vars.bloomStrDepAddSize.w = lvlInfo.bloomSize;
    Global::mSceneMgr->setSkyBox(true, lvlInfo.skyboxName);
    Global::mSceneMgr->setFog(FOG_LINEAR, lvlInfo.fogColor, 1, lvlInfo.fogStartDistance, lvlInfo.fogEndDistance);

    Global::mWorld->setWorldSize(Vector3(-2000, -500, -2000), Vector3(2000, 500, 2000));
}

void GameStateManager::restartLevel()
{
    switchToLevel(lastLVL);
}

void GameStateManager::reloadLevel()
{
    SceneParser::reloadScene(levels[lastLVL].path + levels[lastLVL].sceneFile);
}

bool GameStateManager::insideMenuPressed()
{
    bool continueExecution = true;
    int i = myMenu->mainMenuPressed();

    if (i > 0)
    {
        switchState(i);
    }
    if (i == SS_MAINMENU)
    {
        if (gameState == PAUSE)
            switchState(i);
        else
            continueExecution = false;
    }
    if (i == -1)
    {
        Global::mPPMgr->vars.radialHorizBlurVignette.z = 0.0;
        gameState = GAME;
    }
    if (i == SS_RESTART)
    {
        switchState(i);
    }

    return continueExecution;
}

void GameStateManager::insideMenuMoved(int x, int y)
{
    myMenu->mouseMoveUpdate(x,y);
}

void GameStateManager::switchState(int target, float time)
{
    stateTarget = target;
    switchStateTimer = time;

    Global::mPPMgr->fadeOut(Vector3(0, 0, 0), time);
    myMenu->clearMenu();
}

void GameStateManager::addDebugKey(std::string name, float* target, float step /* = 0.2f */)
{
    dbg->debugVars.push_back(DebugVar(name, target, step));
}

void GameStateManager::updateStateSwitching(float tslf)
{
    if (switchStateTimer > 0)
    {
        switchStateTimer -= tslf;
        return;
    }
    else if (!switchingState)
    {
        switchingState = true;
        return;
    }

    if (stateTarget > 0)
    {
        switchToLevel(stateTarget);
    }
    if (stateTarget == SS_MAINMENU)
    {
        switchToMainMenu();
    }
    if (stateTarget == SS_RESTART)
    {
        restartLevel();
    }

    stateTarget = 0;
    switchingState = false;
}

void GameStateManager::update(float tslf)
{
    if (stateTarget!=0)
        updateStateSwitching(tslf);
    else
        switch (gameState)
        {
        case GAME:
            myMenu->setDebugValue(Global::mWindow->getLastFPS(), GUtils::debug, dbg->debugVars, dbg->debugVarsLine);
            myMenu->updateIngame(tslf);
            break;
        case PAUSE:
            myMenu->updateIngameMenu(tslf);
            break;
        case MENU:
            myMenu->updateMainMenu(tslf);
            break;
        }

    if (Global::fallSoundOffsetH > 0)
    {
        Global::fallSoundOffsetH -= tslf;
    }
    if (Global::fallSoundOffsetL > 0)
    {
        Global::fallSoundOffsetL -= tslf;
    }
}

void GameStateManager::escapePressed()
{
    static float lastNoise = 0;
    static float lastCont = 0;

    if (gameState == GAME)
    {
        myMenu->clearMenu();
        myMenu->setIngameMenu();
        Global::mPPMgr->vars.radialHorizBlurVignette.z = 2.0;

        lastNoise = Global::mPPMgr->vars.ContSatuSharpNoise.z;
        lastCont = Global::mPPMgr->vars.ContSatuSharpNoise.x;
        Global::mPPMgr->vars.ContSatuSharpNoise.z = 1;
        Global::mPPMgr->vars.ContSatuSharpNoise.x = 1;

        gameState = PAUSE;
    }
    else if (gameState == PAUSE)
    {
        myMenu->clearMenu();
        Global::mPPMgr->vars.radialHorizBlurVignette.z = 0.0;
        Global::mPPMgr->vars.ContSatuSharpNoise.z = lastNoise;
        Global::mPPMgr->vars.ContSatuSharpNoise.x = lastCont;

        gameState = GAME;
    }
}

void GameStateManager::clearLevel()
{
    SceneCubeMap::clearAll();
    geometryMgr->clear();
    Global::mWorld->destroyAllBodies();
    Global::mSceneMgr->clearScene();
    Global::mEventsMgr->clear();
    Global::soundEngine->removeAllSoundSources();
    Global::mPPMgr->resetValues();
    VolumeDetectionManager::get.reset();
    SceneEnergies::reset();
    Gate::reset();
}
