#include "emissionpanels.h"
#include "partedapp.h"
#include "partedframe.h"
#include <wx/valnum.h>

void EmissionPointPropertiesDialog::Update(bool update_dialogs) {}

EmissionLinePropertiesDialog::EmissionLinePropertiesDialog(wxWindow *parent) : EmissionLinePropBase(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_xLineStartTxt->SetValidator(fpval);
    m_yLineStartTxt->SetValidator(fpval);
    m_zLineStartTxt->SetValidator(fpval);
    m_xLineEndTxt->SetValidator(fpval);
    m_yLineEndTxt->SetValidator(fpval);
    m_zLineEndTxt->SetValidator(fpval);
}

void EmissionLinePropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_xLineStartTxt->SetValue(buff);
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_yLineStartTxt->SetValue(buff);
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_zLineStartTxt->SetValue(buff);
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_xLineEndTxt->SetValue(buff);
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_yLineEndTxt->SetValue(buff);
        wxframe->Get_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_zLineEndTxt->SetValue(buff);
    } else {
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_X, std::atof(m_xLineStartTxt->GetValue()));
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_Y, std::atof(m_yLineStartTxt->GetValue()));
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MIN_Z, std::atof(m_zLineStartTxt->GetValue()));
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_X, std::atof(m_xLineEndTxt->GetValue()));
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_Y, std::atof(m_yLineEndTxt->GetValue()));
        wxframe->Set_Emission_Line_Properties(ParticleSystemsDialog::VAL_MAX_Z, std::atof(m_zLineEndTxt->GetValue()));
    }
}

EmissionBoxPropertiesDialog::EmissionBoxPropertiesDialog(wxWindow *parent) : EmissionBoxPropBase(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_xBoxTxt->SetValidator(fpval);
    m_yBoxTxt->SetValidator(fpval);
    m_zBoxTxt->SetValidator(fpval);
}

void EmissionBoxPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Emission_Square_Properties(ParticleSystemsDialog::VAL_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_xBoxTxt->SetValue(buff);
        wxframe->Get_Emission_Square_Properties(ParticleSystemsDialog::VAL_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_yBoxTxt->SetValue(buff);
        wxframe->Get_Emission_Square_Properties(ParticleSystemsDialog::VAL_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_zBoxTxt->SetValue(buff);
    } else {
        wxframe->Set_Emission_Square_Properties(ParticleSystemsDialog::VAL_X, std::atof(m_xBoxTxt->GetValue()));
        wxframe->Set_Emission_Square_Properties(ParticleSystemsDialog::VAL_Y, std::atof(m_yBoxTxt->GetValue()));
        wxframe->Set_Emission_Square_Properties(ParticleSystemsDialog::VAL_Z, std::atof(m_zBoxTxt->GetValue()));
    }
}

EmissionSpherePropertiesDialog::EmissionSpherePropertiesDialog(wxWindow *parent) : EmissionSpherePropBase(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_radiusTxt->SetValidator(fpval);
}

void EmissionSpherePropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Emission_Sphere_Properties(fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radiusTxt->SetValue(buff);
    } else {
        wxframe->Set_Emission_Sphere_Properties(std::atof(m_radiusTxt->GetValue()));
    }
}

EmissionCylinderPropertiesDialog::EmissionCylinderPropertiesDialog(wxWindow *parent) : EmissionCylinderPropBase(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_radiusTxt->SetValidator(fpval);
    m_lengthTxt->SetValidator(fpval);
}

void EmissionCylinderPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Emission_Cylinder_Properties(ParticleSystemsDialog::EM_CYLINDER_RADIUS, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radiusTxt->SetValue(buff);
        wxframe->Get_Emission_Cylinder_Properties(ParticleSystemsDialog::EM_CYLINDER_LENGTH, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_lengthTxt->SetValue(buff);
    } else {
        wxframe->Set_Emission_Cylinder_Properties(
            ParticleSystemsDialog::EM_CYLINDER_RADIUS, std::atof(m_radiusTxt->GetValue()));
        wxframe->Set_Emission_Cylinder_Properties(
            ParticleSystemsDialog::EM_CYLINDER_LENGTH, std::atof(m_lengthTxt->GetValue()));
    }
}
