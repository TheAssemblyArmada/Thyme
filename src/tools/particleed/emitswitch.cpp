#include "emitswitch.h"
#include "partedapp.h"
#include "partedframe.h"

EmissionSwitchesDialog::EmissionSwitchesDialog(wxWindow *parent) : EmitSwitchBase(parent)
{
    Bind(wxEVT_CHECKBOX, &EmissionSwitchesDialog::On_Changed, this, XRCID("m_hollow"));
    Bind(wxEVT_CHECKBOX, &EmissionSwitchesDialog::On_Changed, this, XRCID("m_oneShot"));
    Bind(wxEVT_CHECKBOX, &EmissionSwitchesDialog::On_Changed, this, XRCID("m_alignXY"));
    Bind(wxEVT_CHECKBOX, &EmissionSwitchesDialog::On_Changed, this, XRCID("m_aboveGround"));
    Bind(wxEVT_CHECKBOX, &EmissionSwitchesDialog::On_Changed, this, XRCID("m_particleUp"));
}

void EmissionSwitchesDialog::On_Changed(wxCommandEvent &event)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxframe->On_Changed(event);
}

void EmissionSwitchesDialog::Init_Entries() {}

void EmissionSwitchesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    bool val;

    if (update_dialogs) {
        wxframe->Get_Switch(ParticleSystemsDialog::SWITCH_HOLLOW, val);
        m_hollow->SetValue(val);
        wxframe->Get_Switch(ParticleSystemsDialog::SWITCH_ONE_SHOT, val);
        m_oneShot->SetValue(val);
        wxframe->Get_Switch(ParticleSystemsDialog::SWITCH_GROUND_ALIGNED, val);
        m_alignXY->SetValue(val);
        wxframe->Get_Switch(ParticleSystemsDialog::SWITCH_ABOVE_GROUND_ONLY, val);
        m_aboveGround->SetValue(val);
        wxframe->Get_Switch(ParticleSystemsDialog::SWITCH_TOWARDS_EMITTER, val);
        m_particleUp->SetValue(val);
    } else {
        val = m_hollow->GetValue();
        wxframe->Set_Switch(ParticleSystemsDialog::SWITCH_HOLLOW, val);
        val = m_oneShot->GetValue();
        wxframe->Set_Switch(ParticleSystemsDialog::SWITCH_ONE_SHOT, val);
        val = m_alignXY->GetValue();
        wxframe->Set_Switch(ParticleSystemsDialog::SWITCH_GROUND_ALIGNED, val);
        val = m_aboveGround->GetValue();
        wxframe->Set_Switch(ParticleSystemsDialog::SWITCH_ABOVE_GROUND_ONLY, val);
        val = m_particleUp->GetValue();
        wxframe->Set_Switch(ParticleSystemsDialog::SWITCH_TOWARDS_EMITTER, val);
    }
}
