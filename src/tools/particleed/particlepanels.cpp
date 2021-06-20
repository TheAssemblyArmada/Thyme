#include "particlepanels.h"
#include "partedapp.h"
#include "partedframe.h"

void _3DParticleParametersDialog::On_Change()
{
    m_particleCombo->Clear();
}

void _3DParticleParametersDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxString sval;

    if (m_particleCombo->IsListEmpty()) {
        m_particleCombo->AppendString("(None)");
        for (auto str : wxframe->Get_Template_List()) {
            m_particleCombo->AppendString(str.c_str());
        }
    }

    if (update_dialogs) {
        wxframe->Get_Particle(sval);
        m_particleCombo->SetStringSelection(sval);
    } else {
        sval = m_particleCombo->GetStringSelection();
        wxframe->Set_Particle(sval);
    }
}

void _2DParticleParametersDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxString sval;

    if (update_dialogs) {
        wxframe->Get_Particle(sval);
        m_particleChoice->SetStringSelection(sval);
    } else {
        sval = m_particleChoice->GetStringSelection();
        wxframe->Set_Particle(sval);
    }
}

void StreakParticleParametersDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxString sval;

    if (update_dialogs) {
        wxframe->Get_Particle(sval);
        m_streakChoice->SetStringSelection(sval);
    } else {
        sval = m_streakChoice->GetStringSelection();
        wxframe->Set_Particle(sval);
    }
}
