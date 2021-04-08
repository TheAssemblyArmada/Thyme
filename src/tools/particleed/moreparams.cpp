#include "moreparams.h"
#include "partedapp.h"
#include "partedframe.h"
#include <wx/valnum.h>
#include <wx/valtext.h>

const char *MoreParametersDialog::s_windNames[] = { "None", "Unused", "PingPong", "Circular", nullptr };

MoreParametersDialog::MoreParametersDialog(wxWindow *parent) : MoreParamsBase(parent)
{
    m_initialDelayMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_initialDelayMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_burstDelayMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_burstDelayMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_burstCountMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_burstCountMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_colorScaleMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_colorScaleMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_partLifetimeMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_partLifetimeMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startSizeMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startSizeMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startRateMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startRateMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startDampingMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_startDampingMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_systemLifeTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_slaveOffsetXTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_slaveOffsetYTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_slaveOffsetZTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_driftVelXTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_driftVelYTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_driftVelZTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windStartAngleMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windStartAngleMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windEndAngleMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windEndAngleMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windAngleRateMinTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    m_windAngleRateMaxTxt->Bind(wxEVT_COMMAND_KILL_FOCUS, &MoreParametersDialog::On_Changed, this);
    Bind(wxEVT_CHOICE, &MoreParametersDialog::On_Changed, this, XRCID("m_perParticleChoice"));
    Bind(wxEVT_CHOICE, &MoreParametersDialog::On_Changed, this, XRCID("m_windMotionChoice"));
    Bind(wxEVT_COMBOBOX, &MoreParametersDialog::On_Changed, this, XRCID("m_slaveCombo"));
}

void MoreParametersDialog::On_Changed(wxCommandEvent &event)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxframe->On_Changed(event);
}

void MoreParametersDialog::Init_Entries()
{
    wxTextValidator validator(wxFILTER_NUMERIC);
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_DEFAULT);
    m_initialDelayMinTxt->SetValidator(fpval);
    m_initialDelayMaxTxt->SetValidator(fpval);
    m_burstDelayMinTxt->SetValidator(fpval);
    m_burstDelayMaxTxt->SetValidator(fpval);
    m_burstCountMinTxt->SetValidator(fpval);
    m_burstCountMaxTxt->SetValidator(fpval);
    m_colorScaleMinTxt->SetValidator(fpval);
    m_colorScaleMaxTxt->SetValidator(fpval);
    m_partLifetimeMinTxt->SetValidator(fpval);
    m_partLifetimeMaxTxt->SetValidator(fpval);
    m_startSizeMinTxt->SetValidator(fpval);
    m_startSizeMaxTxt->SetValidator(fpval);
    m_startRateMinTxt->SetValidator(fpval);
    m_startRateMaxTxt->SetValidator(fpval);
    m_startDampingMinTxt->SetValidator(fpval);
    m_startDampingMaxTxt->SetValidator(fpval);
    m_systemLifeTxt->SetValidator(fpval);
    m_slaveOffsetXTxt->SetValidator(fpval);
    m_slaveOffsetYTxt->SetValidator(fpval);
    m_slaveOffsetZTxt->SetValidator(fpval);
    m_driftVelXTxt->SetValidator(fpval);
    m_driftVelYTxt->SetValidator(fpval);
    m_driftVelZTxt->SetValidator(fpval);
    m_windStartAngleMinTxt->SetValidator(fpval);
    m_windStartAngleMaxTxt->SetValidator(fpval);
    m_windEndAngleMinTxt->SetValidator(fpval);
    m_windEndAngleMaxTxt->SetValidator(fpval);
    m_windAngleRateMinTxt->SetValidator(fpval);
    m_windAngleRateMaxTxt->SetValidator(fpval);

    for (int i = 1; s_windNames[i] != nullptr; ++i) {
        m_windMotionChoice->AppendString(s_windNames[i]);
    }
}

void MoreParametersDialog::Update(bool update_dialogs)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    char buff[128];
    float fval;
    wxString sval;
    int ival;

    if (m_slaveCombo->IsListEmpty()) {
        m_slaveCombo->AppendString("(None)");
        m_slaveCombo->Append(wxframe->Get_System_List());
    }

    if (m_perParticleChoice->IsEmpty()) {
        m_perParticleChoice->AppendString("(None)");
        m_perParticleChoice->Append(wxframe->Get_System_List());
    }

    if (update_dialogs) {
        wxframe->Get_Initial_Delay(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_initialDelayMinTxt->SetValue(buff);
        wxframe->Get_Initial_Delay(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_initialDelayMaxTxt->SetValue(buff);
        wxframe->Get_Burst_Delay(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_burstDelayMinTxt->SetValue(buff);
        wxframe->Get_Burst_Delay(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_burstDelayMaxTxt->SetValue(buff);
        wxframe->Get_Burst_Count(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_burstCountMinTxt->SetValue(buff);
        wxframe->Get_Burst_Count(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_burstCountMaxTxt->SetValue(buff);
        wxframe->Get_Color_Scale(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_colorScaleMinTxt->SetValue(buff);
        wxframe->Get_Color_Scale(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_colorScaleMaxTxt->SetValue(buff);
        wxframe->Get_Lifetime(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_partLifetimeMinTxt->SetValue(buff);
        wxframe->Get_Lifetime(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_partLifetimeMaxTxt->SetValue(buff);
        wxframe->Get_Start_Size(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startSizeMinTxt->SetValue(buff);
        wxframe->Get_Start_Size(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startSizeMaxTxt->SetValue(buff);
        wxframe->Get_Start_Size_Rate(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startSizeRateMinTxt->SetValue(buff);
        wxframe->Get_Start_Size_Rate(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startSizeRateMaxTxt->SetValue(buff);
        wxframe->Get_Size_Rate(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startRateMinTxt->SetValue(buff);
        wxframe->Get_Size_Rate(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startRateMaxTxt->SetValue(buff);
        wxframe->Get_Size_Damping(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startDampingMinTxt->SetValue(buff);
        wxframe->Get_Size_Damping(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_startDampingMaxTxt->SetValue(buff);
        wxframe->Get_System_Life(fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_systemLifeTxt->SetValue(buff);
        wxframe->Get_Slave_Offset(ParticleSystemsDialog::VAL_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_slaveOffsetXTxt->SetValue(buff);
        wxframe->Get_Slave_Offset(ParticleSystemsDialog::VAL_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_slaveOffsetYTxt->SetValue(buff);
        wxframe->Get_Slave_Offset(ParticleSystemsDialog::VAL_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_slaveOffsetZTxt->SetValue(buff);
        wxframe->Get_Drift_Velocity(ParticleSystemsDialog::VAL_X, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_driftVelXTxt->SetValue(buff);
        wxframe->Get_Drift_Velocity(ParticleSystemsDialog::VAL_Y, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_driftVelYTxt->SetValue(buff);
        wxframe->Get_Drift_Velocity(ParticleSystemsDialog::VAL_Z, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_driftVelZTxt->SetValue(buff);

        wxframe->Get_Slave_System(sval);

        if (!sval.IsEmpty()) {
            m_slaveCombo->SetStringSelection(sval);
        } else {
            m_slaveCombo->SetStringSelection("(None)");
        }
        
        wxframe->Get_Per_Particle(sval);

        if (!sval.IsEmpty()) {
            m_perParticleChoice->SetStringSelection(sval);
        } else {
            m_perParticleChoice->SetStringSelection("(None)");
        }

        wxframe->Get_Wind_Start_Angle(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windStartAngleMinTxt->SetValue(buff);
        wxframe->Get_Wind_Start_Angle(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windStartAngleMaxTxt->SetValue(buff);
        wxframe->Get_Wind_End_Angle(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windEndAngleMinTxt->SetValue(buff);
        wxframe->Get_Wind_End_Angle(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windEndAngleMaxTxt->SetValue(buff);
        wxframe->Get_Wind_Angle_Rate(ParticleSystemsDialog::VAL_MIN, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windAngleRateMinTxt->SetValue(buff);
        wxframe->Get_Wind_Angle_Rate(ParticleSystemsDialog::VAL_MAX, fval);
        std::snprintf(buff, sizeof(buff), "%.2f", fval);
        m_windAngleRateMaxTxt->SetValue(buff);
        wxframe->Get_Wind_Motion(ival);
        m_windMotionChoice->SetStringSelection(s_windNames[ival]);
    } else {
        wxframe->Set_Initial_Delay(ParticleSystemsDialog::VAL_MIN, std::atof(m_initialDelayMinTxt->GetValue()));
        wxframe->Set_Initial_Delay(ParticleSystemsDialog::VAL_MAX, std::atof(m_initialDelayMaxTxt->GetValue()));
        wxframe->Set_Burst_Delay(ParticleSystemsDialog::VAL_MIN, std::atof(m_burstDelayMinTxt->GetValue()));
        wxframe->Set_Burst_Delay(ParticleSystemsDialog::VAL_MAX, std::atof(m_burstDelayMaxTxt->GetValue()));
        wxframe->Set_Burst_Count(ParticleSystemsDialog::VAL_MIN, std::atof(m_burstCountMinTxt->GetValue()));
        wxframe->Set_Burst_Count(ParticleSystemsDialog::VAL_MAX, std::atof(m_burstCountMaxTxt->GetValue()));
        wxframe->Set_Color_Scale(ParticleSystemsDialog::VAL_MIN, std::atof(m_colorScaleMinTxt->GetValue()));
        wxframe->Set_Color_Scale(ParticleSystemsDialog::VAL_MAX, std::atof(m_colorScaleMaxTxt->GetValue()));
        wxframe->Set_Lifetime(ParticleSystemsDialog::VAL_MIN, std::atof(m_partLifetimeMinTxt->GetValue()));
        wxframe->Set_Lifetime(ParticleSystemsDialog::VAL_MAX, std::atof(m_partLifetimeMaxTxt->GetValue()));
        wxframe->Set_Start_Size(ParticleSystemsDialog::VAL_MIN, std::atof(m_startSizeMinTxt->GetValue()));
        wxframe->Set_Start_Size(ParticleSystemsDialog::VAL_MAX, std::atof(m_startSizeMaxTxt->GetValue()));
        wxframe->Set_Start_Size_Rate(ParticleSystemsDialog::VAL_MIN, std::atof(m_startSizeRateMinTxt->GetValue()));
        wxframe->Set_Start_Size_Rate(ParticleSystemsDialog::VAL_MAX, std::atof(m_startSizeRateMaxTxt->GetValue()));
        wxframe->Set_Start_Size(ParticleSystemsDialog::VAL_MIN, std::atof(m_startRateMinTxt->GetValue()));
        wxframe->Set_Start_Size(ParticleSystemsDialog::VAL_MAX, std::atof(m_startRateMaxTxt->GetValue()));
        wxframe->Set_Size_Damping(ParticleSystemsDialog::VAL_MIN, std::atof(m_startDampingMinTxt->GetValue()));
        wxframe->Set_Size_Damping(ParticleSystemsDialog::VAL_MAX, std::atof(m_startDampingMaxTxt->GetValue()));
        wxframe->Set_System_Life(std::atof(m_systemLifeTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_X, std::atof(m_slaveOffsetXTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_Y, std::atof(m_slaveOffsetYTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_Z, std::atof(m_slaveOffsetZTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_X, std::atof(m_driftVelXTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_Y, std::atof(m_driftVelYTxt->GetValue()));
        wxframe->Set_Slave_Offset(ParticleSystemsDialog::VAL_Z, std::atof(m_driftVelZTxt->GetValue()));
        sval = m_slaveCombo->GetStringSelection();
        wxframe->Set_Slave_System(sval.IsEmpty() ? "(None)" : sval);
        sval = m_perParticleChoice->GetStringSelection();
        wxframe->Set_Per_Particle(sval.IsEmpty() ? "(None)" : sval);
        wxframe->Set_Wind_Start_Angle(ParticleSystemsDialog::VAL_MIN, std::atof(m_windStartAngleMinTxt->GetValue()));
        wxframe->Set_Wind_Start_Angle(ParticleSystemsDialog::VAL_MAX, std::atof(m_windStartAngleMaxTxt->GetValue()));
        wxframe->Set_Wind_End_Angle(ParticleSystemsDialog::VAL_MIN, std::atof(m_windEndAngleMinTxt->GetValue()));
        wxframe->Set_Wind_End_Angle(ParticleSystemsDialog::VAL_MAX, std::atof(m_windEndAngleMaxTxt->GetValue()));
        wxframe->Set_Wind_Angle_Rate(ParticleSystemsDialog::VAL_MIN, std::atof(m_windAngleRateMinTxt->GetValue()));
        wxframe->Set_Wind_Angle_Rate(ParticleSystemsDialog::VAL_MAX, std::atof(m_windAngleRateMaxTxt->GetValue()));
        wxframe->Set_Wind_Motion(m_windMotionChoice->GetSelection());
    }
}
