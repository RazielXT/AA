#include "stdafx.h"
#include "DetailGeometryEdit.h"
#include "ManualDetailGeometry.h"

DetailGeometryEdit::DetailGeometryEdit(LoadedManualDG* manualDG, int matID)
{
    sg = manualDG->sg;
    entity = nullptr;

    materialPtr = Ogre::MaterialManager::getSingleton().getByName(manualDG->usedMats[matID]->getName());

    loadMaterialInfo();
    dgName = manualDG->name;

    changedMaterial = Global::gameMgr->sceneEdits.loadSavedDetailGeometryChanges(*this, dgName + originName);

    rows = { { dgName,EditRow::Caption },{ originName,EditRow::Caption },{ "Save",EditRow::Action },{ "PS",EditRow::Params } };
}

EditVariables* DetailGeometryEdit::getParams(const std::string& row)
{
    return MaterialEdit::getParams(row);
}

void DetailGeometryEdit::editChanged(EditVariable& var, const std::string& row)
{
    MaterialEdit::editChanged(var, row);
}

void DetailGeometryEdit::customAction(std::string name)
{
    if (name == "Save")
    {
        if (changedMaterial)
            Global::gameMgr->sceneEdits.addDetailGeometryEdit(*this, dgName + originName);
        else
            Global::gameMgr->sceneEdits.removeDetailGeometryEdit(dgName + originName);
    }
}

DetailGeometryEdit* DetailGeometryEdit::query()
{
    auto dg = ManualDetailGeometry::getClosest();

    if (!dg)
        return nullptr;

    auto edit = new DetailGeometryEdit(dg, 0);

    return edit;
}

void DetailGeometryEdit::applyChanges(std::map < std::string, DetailGeometryEdit >& changes)
{
    for (auto& dg : changes)
    {
        auto& loadedDg = ManualDetailGeometry::loadedMDG;

        LoadedManualDG* dgInfo = nullptr;
        for (auto ldg : loadedDg)
        {
            if (ldg.name == dg.first)
            {
                dgInfo = &ldg;
            }
        }

        if (dgInfo)
        {
            for (auto mat : dgInfo->usedMats)
            {
                if (dg.second.originName == mat->getName())
                {
                    for (auto& var : dg.second.psVariables)
                    {
                        int pass = mat->getTechnique(0)->getNumPasses() - 1;
                        mat->getTechnique(0)->getPass(pass)->getFragmentProgramParameters()->setNamedConstant(var.name, var.buffer, 1, var.size);
                    }
                }
            }
        }
    }
}

void DetailGeometryEdit::resetMaterial()
{
    MaterialEdit::resetMaterial();
}

void DetailGeometryEdit::materialChanged()
{
    MaterialEdit::materialChanged();
}
