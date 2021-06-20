#pragma once

#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/clrpicker.h>
#include <wx/combobox.h>
#include <wx/dialog.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/simplebook.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/tglbtn.h>
#include <wx/xrc/xmlres.h>

#include "wxpartedui.h"

#include "updatable.h"

class EmissionPointPropertiesDialog : public EmissionPointPropBase, public Updatable
{
public:
    EmissionPointPropertiesDialog(wxWindow *parent) : EmissionPointPropBase(parent) {}

    virtual void Update(bool update_dialogs) override;
};

class EmissionLinePropertiesDialog : public EmissionLinePropBase, public Updatable
{
public:
    EmissionLinePropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class EmissionBoxPropertiesDialog : public EmissionBoxPropBase, public Updatable
{
public:
    EmissionBoxPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class EmissionSpherePropertiesDialog : public EmissionSpherePropBase, public Updatable
{
public:
    EmissionSpherePropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};

class EmissionCylinderPropertiesDialog : public EmissionCylinderPropBase, public Updatable
{
public:
    EmissionCylinderPropertiesDialog(wxWindow *parent);

    virtual void Update(bool update_dialogs) override;
};
