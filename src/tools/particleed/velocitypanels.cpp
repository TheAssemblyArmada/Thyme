#include "velocitypanels.h"
#include "partedapp.h"
#include "partedframe.h"
#include <wx/valnum.h>

OrthoVelocityPropertiesDialog::OrthoVelocityPropertiesDialog(wxWindow *parent) : OrthoVelocityBasePanel(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_xMinTxt->SetValidator(fpval);
    m_yMinTxt->SetValidator(fpval);
    m_zMinTxt->SetValidator(fpval);
    m_xMaxTxt->SetValidator(fpval);
    m_yMaxTxt->SetValidator(fpval);
    m_zMaxTxt->SetValidator(fpval);
}

void OrthoVelocityPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_xMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_yMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_zMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_xMaxTxt->SetValue(buff);
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_yMaxTxt->SetValue(buff);
        wxframe->Get_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_zMaxTxt->SetValue(buff);
    } else {
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_X, std::atof(m_xMinTxt->GetValue()));
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_Y, std::atof(m_yMinTxt->GetValue()));
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MIN_Z, std::atof(m_zMinTxt->GetValue()));
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_X, std::atof(m_xMaxTxt->GetValue()));
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_Y, std::atof(m_yMaxTxt->GetValue()));
        wxframe->Set_Velocity_Ortho_Properties(ParticleSystemsDialog::VAL_MAX_Z, std::atof(m_zMaxTxt->GetValue()));
    }
}

SphericalVelocityPropertiesDialog::SphericalVelocityPropertiesDialog(wxWindow *parent) : SphericalVelocityBasePanel(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_radMinTxt->SetValidator(fpval);
    m_radMaxTxt->SetValidator(fpval);
}

void SphericalVelocityPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Velocity_Spherical_Properties(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Spherical_Properties(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMaxTxt->SetValue(buff);
    } else {
        wxframe->Set_Velocity_Spherical_Properties(ParticleSystemsDialog::VAL_MIN, std::atof(m_radMinTxt->GetValue()));
        wxframe->Set_Velocity_Spherical_Properties(ParticleSystemsDialog::VAL_MAX, std::atof(m_radMaxTxt->GetValue()));
    }
}

HemisphericalVelocityPropertiesDialog::HemisphericalVelocityPropertiesDialog(wxWindow *parent) :
    HemisphericalVelocityBasePanel(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_radMinTxt->SetValidator(fpval);
    m_radMaxTxt->SetValidator(fpval);
}

void HemisphericalVelocityPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Velocity_Hemispherical_Properties(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Hemispherical_Properties(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMaxTxt->SetValue(buff);
    } else {
        wxframe->Set_Velocity_Hemispherical_Properties(ParticleSystemsDialog::VAL_MIN, std::atof(m_radMinTxt->GetValue()));
        wxframe->Set_Velocity_Hemispherical_Properties(ParticleSystemsDialog::VAL_MAX, std::atof(m_radMaxTxt->GetValue()));
    }
}

CylindricalVelocityPropertiesDialog::CylindricalVelocityPropertiesDialog(wxWindow *parent) :
    CylindricalVelocityBasePanel(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_radMinTxt->SetValidator(fpval);
    m_radMaxTxt->SetValidator(fpval);
    m_orthoMinTxt->SetValidator(fpval);
    m_orthoMaxTxt->SetValidator(fpval);
}

void CylindricalVelocityPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Velocity_Cylindrical_Properties_Radial(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Cylindrical_Properties_Radial(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_radMaxTxt->SetValue(buff);
        wxframe->Get_Velocity_Cylindrical_Properties_Ortho(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_orthoMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Cylindrical_Properties_Ortho(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_orthoMaxTxt->SetValue(buff);
    } else {
        wxframe->Set_Velocity_Cylindrical_Properties_Radial(
            ParticleSystemsDialog::VAL_MIN, std::atof(m_radMinTxt->GetValue()));
        wxframe->Set_Velocity_Cylindrical_Properties_Radial(
            ParticleSystemsDialog::VAL_MAX, std::atof(m_radMaxTxt->GetValue()));
        wxframe->Set_Velocity_Cylindrical_Properties_Ortho(
            ParticleSystemsDialog::VAL_MIN, std::atof(m_orthoMinTxt->GetValue()));
        wxframe->Set_Velocity_Cylindrical_Properties_Ortho(
            ParticleSystemsDialog::VAL_MAX, std::atof(m_orthoMaxTxt->GetValue()));
    }
}

OutwardVelocityPropertiesDialog::OutwardVelocityPropertiesDialog(wxWindow *parent) : OutwardVelocityBasePanel(parent)
{
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_majorMinTxt->SetValidator(fpval);
    m_majorMaxTxt->SetValidator(fpval);
    m_minorMinTxt->SetValidator(fpval);
    m_minorMaxTxt->SetValidator(fpval);
}

void OutwardVelocityPropertiesDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;

    if (update_dialogs) {
        wxframe->Get_Velocity_Outward_Properties_Major(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_majorMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Outward_Properties_Major(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_majorMaxTxt->SetValue(buff);
        wxframe->Get_Velocity_Outward_Properties_Minor(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_minorMinTxt->SetValue(buff);
        wxframe->Get_Velocity_Outward_Properties_Minor(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_minorMaxTxt->SetValue(buff);
    } else {
        wxframe->Set_Velocity_Outward_Properties_Major(
            ParticleSystemsDialog::VAL_MIN, std::atof(m_majorMinTxt->GetValue()));
        wxframe->Set_Velocity_Outward_Properties_Major(
            ParticleSystemsDialog::VAL_MAX, std::atof(m_majorMaxTxt->GetValue()));
        wxframe->Set_Velocity_Outward_Properties_Minor(
            ParticleSystemsDialog::VAL_MIN, std::atof(m_minorMinTxt->GetValue()));
        wxframe->Set_Velocity_Outward_Properties_Minor(
            ParticleSystemsDialog::VAL_MAX, std::atof(m_minorMaxTxt->GetValue()));
    }
}
