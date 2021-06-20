#include "partedframe.h"
#include "emissionpanels.h"
#include "particlepanels.h"
#include "velocitypanels.h"
#include "updatable.h"
#include <captainslog.h>
#include <cstdlib>
#include <wx/choice.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/xrc/xmlres.h>

wxWindow *g_mainWindow;

const char *ParticleSystemsDialog::s_priorityNames[PARTICLE_PRIORITY_COUNT + 1] = {
    "NONE",
    "WEAPON_EXPLOSION",
    "SCORCHMARK",
    "DUST_TRAIL",
    "BUILDUP",
    "DEBRIS_TRAIL",
    "UNIT_DAMAGE_FX",
    "DEATH_EXPLOSION",
    "SEMI_CONSTANT",
    "CONSTANT",
    "WEAPON_TRAIL",
    "AREA_EFFECT",
    "CRITICAL",
    "ALWAYS_RENDER",
    nullptr,
};

const char *ParticleSystemsDialog::s_emissionTypes[ParticleSystemInfo::EMISSION_VOLUME_COUNT + 1] = {
    "NONE",
    "POINT",
    "LINE",
    "BOX",
    "SPHERE",
    "CYLINDER",
    nullptr,
};

const char *ParticleSystemsDialog::s_velocityTypes[ParticleSystemInfo::EMISSION_VELOCITY_COUNT + 1] = {
    "NONE",
    "ORTHO",
    "SPHERICAL",
    "HEMISPHERICAL",
    "CYLINDRICAL",
    "OUTWARD",
    nullptr,
};

const char *ParticleSystemsDialog::s_parameterTypes[ParticleSystemInfo::PARTICLE_TYPE_COUNT + 1] = {
    "NONE",
    "PARTICLE",
    "DRAWABLE",
    "STREAK",
    "VOLUME_PARTICLE",
    "SMUDGE",
    nullptr,
};

const char *ParticleSystemsDialog::s_shaderTypes[ParticleSystemInfo::PARTICLE_SHADER_COUNT + 1] = {
    "NONE",
    "ADDITIVE",
    "ALPHA",
    "ALPHA_TEST",
    "MULTIPLY",
    nullptr,
};

ParticleSystemsDialog::ParticleSystemsDialog(wxWindow *parent, const wxString &label) :
    PartEdBaseFrame(parent),
    m_colorAlpha(this),
    m_emitterSwitch(this),
    m_moreParams(this),
    m_hasRequestedKillAll(false),
    m_hasRequestedReload(false),
    m_updateSelectedSystem(false),
    m_busyWait(false),
    m_reloadTextures(false),
    m_updateParticleCap(false),
    m_systemListDirty(false)
{
    g_mainWindow = parent;
    SetPosition({ 0, 0 });

    // If we aren't building standalone, then some structs don't have X and Y dimensions.
#ifdef GAME_DLL
    m_angleXMin->Disable();
    m_angleXMax->Disable();
    m_angleYMin->Disable();
    m_angleYMax->Disable();
    m_angRateXMin->Disable();
    m_angRateXMax->Disable();
    m_angRateYMin->Disable();
    m_angRateYMax->Disable();
#endif

    Init_Entries();
    
    // Set up event handlers.
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Request_Reload, this, XRCID("m_reloadSys"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Request_Reload, this, XRCID("m_reloadAll"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Write, this, XRCID("m_saveSys"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Write, this, XRCID("m_saveAll"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Reload_Texture, this, XRCID("m_reloadTexture"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_About, this, wxID_ABOUT);
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Selected_System, this, XRCID("m_goBtn"));
    Bind(wxEVT_COMBOBOX, &ParticleSystemsDialog::On_Selected_System, this, XRCID("m_systemCombo"));
    Bind(wxEVT_COMBOBOX_DROPDOWN, &ParticleSystemsDialog::On_Drop_System_Combo, this, XRCID("m_systemCombo"));
    Bind(wxEVT_BUTTON, &ParticleSystemsDialog::On_Kill_All, this, XRCID("m_killAllBtn"));
    Bind(wxEVT_CHOICE, &ParticleSystemsDialog::On_Priority, this, XRCID("m_priorityCombo"));
    Bind(wxEVT_CHOICE, &ParticleSystemsDialog::On_Emission_Type, this, XRCID("m_emissionTypeCombo"));
    Bind(wxEVT_CHOICE, &ParticleSystemsDialog::On_Velocity_Type, this, XRCID("m_velocityTypeCombo"));
    Bind(wxEVT_CHOICE, &ParticleSystemsDialog::On_Particle_Type, this, XRCID("m_particleTypeCombo"));
    Bind(wxEVT_CHOICE, &ParticleSystemsDialog::On_Shader_Type, this, XRCID("m_shaderTypeCombo"));
    Bind(wxEVT_TOGGLEBUTTON, &ParticleSystemsDialog::On_Color_Alpha, this, XRCID("m_toggleColorAlpha"));
    Bind(wxEVT_TOGGLEBUTTON, &ParticleSystemsDialog::On_Switches, this, XRCID("m_toggleSwitches"));
    Bind(wxEVT_TOGGLEBUTTON, &ParticleSystemsDialog::On_Continued, this, XRCID("m_toggleContinued"));
    Bind(wxEVT_CLOSE_WINDOW, &ParticleSystemsDialog::On_Exit, this);
    m_partCap->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Cap_Update, this);
    m_angleXMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angleXMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angleYMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angleYMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angleZMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angleZMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateXMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateXMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateYMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateYMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateZMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angRateZMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_gravity->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angDampMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_angDampMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_velDampMin->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);
    m_velDampMax->Bind(wxEVT_COMMAND_KILL_FOCUS, &ParticleSystemsDialog::On_Changed, this);

    m_particleParams.resize(PARAM_COUNT);
}

void ParticleSystemsDialog::On_Request_Reload(wxCommandEvent &event)
{
    m_hasRequestedReload = true;
}

void ParticleSystemsDialog::On_Write(wxCommandEvent &event)
{
    m_writeINI = true;
}

void ParticleSystemsDialog::On_Reload_Texture(wxCommandEvent &event)
{
    if (m_parameterBook->GetSelection() == 0) {
        m_parameterBook->SetSelection(3);
        Update(true);
        m_reloadTextures = true;
    }
}

void ParticleSystemsDialog::On_About(wxCommandEvent &WXUNUSED(event))
{
    wxMessageBox("This window is running in its own thread,\n"
                 "using private wxWidgets instance compiled into the DLL.",
        "About",
        wxOK | wxICON_INFORMATION);
}

void ParticleSystemsDialog::On_Selected_System(wxCommandEvent &event)
{
    m_updateSelectedSystem = true;
}

void ParticleSystemsDialog::On_Kill_All(wxCommandEvent &event)
{
    m_hasRequestedKillAll = true;
}

void ParticleSystemsDialog::On_Priority(wxCommandEvent &event)
{
    wxChoice *choice = wxDynamicCast(event.GetEventObject(), wxChoice);

    if (choice == nullptr || choice->GetParent() == nullptr) {
        return;
    }
}

void ParticleSystemsDialog::On_Emission_Type(wxCommandEvent &event)
{
    m_emissionBook->SetSelection(m_emissionTypeCombo->GetSelection());
}

void ParticleSystemsDialog::On_Velocity_Type(wxCommandEvent &event)
{
    m_velocityBook->SetSelection(m_velocityTypeCombo->GetSelection());
}

void ParticleSystemsDialog::On_Particle_Type(wxCommandEvent &event)
{
    m_parameterBook->SetSelection(m_particleTypeCombo->GetSelection());
}

void ParticleSystemsDialog::On_Shader_Type(wxCommandEvent &event)
{
    wxChoice *choice = wxDynamicCast(event.GetEventObject(), wxChoice);

    if (choice == nullptr || choice->GetParent() == nullptr) {
        return;
    }
}

void ParticleSystemsDialog::On_Color_Alpha(wxCommandEvent &event)
{
    m_colorAlpha.Show(m_toggleColorAlpha->GetValue());
}

void ParticleSystemsDialog::On_Switches(wxCommandEvent &event)
{
    m_emitterSwitch.Show(m_toggleSwitches->GetValue());
}

void ParticleSystemsDialog::On_Continued(wxCommandEvent &event)
{
    m_moreParams.Show(m_toggleContinued->GetValue());
}

void ParticleSystemsDialog::On_Changed(wxCommandEvent &event)
{
    // When we've changed something, update the particle system pointer we hold from the game.
    Update(false);
}

void ParticleSystemsDialog::On_Drop_System_Combo(wxCommandEvent &event)
{
    if (m_systemListDirty) {
        m_particleSystemsList.Sort();
        m_systemCombo->Clear();
        m_systemCombo->Append(m_particleSystemsList);
        m_systemListDirty = false;
    }
}

void ParticleSystemsDialog::On_Exit(wxCloseEvent &event)
{
    // Minimise the window instead of closing it, it should remain in existence until the game
    // calls for its destruction.
    Iconize();
}

void ParticleSystemsDialog::Append_System(const char *system)
{
    if (system == nullptr) {
        return;
    }

    //wxString string(system);
    m_particleSystemsList.Add(system);
    m_systemListDirty = true;
    //m_systemCombo->Append(system);
}

void ParticleSystemsDialog::Append_Thing(const char *thing)
{
    if (thing == nullptr) {
        return;
    }

    //captainslog_info("Appending '%s' to thing template list.", thing);

    m_thingTemplateList.push_back(thing);
}

void ParticleSystemsDialog::Get_Selected_Particle_AsciiString_Parm(int32_t entry, char *name, void **temp)
{
    if (entry >= 0 && entry < PARAM_COUNT && name != nullptr && m_currentParticleSystemTemplate != nullptr) {
        wxString &param = m_particleParams[entry];

        if (param.empty()) {
            name[0] = '\0';
        } else {
            std::strcpy(name, param.c_str());
        }
    }

    if (temp != nullptr) {
        *temp = m_currentParticleSystemTemplate;
    }
}

void ParticleSystemsDialog::Get_Selected_Particle_System_Name(char *name)
{
    std::strcpy(name, m_systemCombo->GetStringSelection());
}

void ParticleSystemsDialog::Remove_All_Particle_Systems()
{
    m_particleSystemsList.clear();
    m_systemCombo->Clear();
}

void ParticleSystemsDialog::Remove_All_Thing_Templates()
{
    m_thingTemplateList.clear();

    // Clear the combo box on the 3DParticle page.
    _3DParticleParametersDialog *page =
        wxDynamicCast(m_parameterBook->GetPage(ParticleSystemInfo::PARTICLE_TYPE_DRAWABLE - 1), _3DParticleParametersDialog);

    if (page != nullptr) {
        page->On_Change();
    }
}

void ParticleSystemsDialog::Update_Current_Particle_Cap(int32_t cap)
{
    char buff[32];
    std::snprintf(buff, sizeof(buff), "%" PRId32, cap);
    m_partCap->SetValue(buff);
}

void ParticleSystemsDialog::Update_Current_Particle_System(void *system)
{
    m_currentParticleSystemTemplate = static_cast<ParticleSystemTemplate *>(system);
    Update(true);
}

void ParticleSystemsDialog::Update_Particle_AsciiString_Parm(int32_t entry, const char *name, void **system)
{
    if (entry >= 0 && entry < PARAM_COUNT && name != nullptr) {
        m_particleParams[entry] = name;
    }

    if (system != nullptr) {
        *system = m_currentParticleSystemTemplate;
    }
}

void ParticleSystemsDialog::Update_System_Use_Parameters(void *system)
{
    m_currentParticleSystemTemplate = static_cast<ParticleSystemTemplate *>(system);
    Update(false);
}

int32_t ParticleSystemsDialog::Get_New_Particle_Cap()
{
    return atoi(m_partCap->GetValue());
}

bool ParticleSystemsDialog::Has_Updated_Selected_Particle_System()
{
    if (!m_updateSelectedSystem) {
        return false;
    }

    // Clear the update flag once application is notified.
    m_updateSelectedSystem = false;
    return true;
}

bool ParticleSystemsDialog::Should_Reload_Textures()
{
    if (!m_reloadTextures) {
        return false;
    }

    // Clear the reload flag once application is notified.
    m_reloadTextures = false;
    return true;
}

bool ParticleSystemsDialog::Should_Update_Particle_Cap()
{
    if (!m_updateParticleCap) {
        return false;
    }

    // Clear the update flag once application is notified.
    m_updateParticleCap = false;
    return true;
}

bool ParticleSystemsDialog::Should_Write_INI()
{
    if (!m_writeINI) {
        return false;
    }

    // Clear the write flag once application is notified.
    m_writeINI = false;
    return true;
}

bool ParticleSystemsDialog::Has_Requested_Kill_All()
{
    if (!m_hasRequestedKillAll) {
        return false;
    }

    // Clear the request flag once application is notified.
    m_hasRequestedKillAll = false;
    return true;
}

bool ParticleSystemsDialog::Has_Requested_Reload()
{
    if (!m_hasRequestedReload) {
        return false;
    }

    // Clear the request flag once application is notified.
    m_hasRequestedReload = false;
    return true;
}

void ParticleSystemsDialog::Get_Color_Key_Frame(int index, RGBColorKeyframe &key) const
{
    if (index < 0 || index >= ParticleSystemInfo::KEYFRAME_COUNT) {
        return;
    }

    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    key = m_currentParticleSystemTemplate->m_colorKey[index];
}

void ParticleSystemsDialog::Set_Color_Key_Frame(int index, const RGBColorKeyframe &key)
{
    if (index < 0 || index >= ParticleSystemInfo::KEYFRAME_COUNT) {
        return;
    }

    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_currentParticleSystemTemplate->m_colorKey[index] = key;
}

void ParticleSystemsDialog::Get_Alpha_Key_Frame(int index, ParticleSystemInfo::RandomKeyframe &key) const
{
    if (index < 0 || index >= ParticleSystemInfo::KEYFRAME_COUNT) {
        return;
    }

    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    key = m_currentParticleSystemTemplate->m_alphaKey[index];
}

void ParticleSystemsDialog::Set_Alpha_Key_Frame(int index, const ParticleSystemInfo::RandomKeyframe &key)
{
    if (index < 0 || index >= ParticleSystemInfo::KEYFRAME_COUNT) {
        return;
    }

    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_currentParticleSystemTemplate->m_alphaKey[index] = key;
}

void ParticleSystemsDialog::Get_Switch(SwitchType type, bool &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case SWITCH_HOLLOW:
            val = m_currentParticleSystemTemplate->m_isEmissionVolumeHollow;
            break;
        case SWITCH_ONE_SHOT:
            val = m_currentParticleSystemTemplate->m_isOneShot;
            break;
        case SWITCH_GROUND_ALIGNED:
            val = m_currentParticleSystemTemplate->m_isGroundAligned;
            break;
        case SWITCH_ABOVE_GROUND_ONLY:
            val = m_currentParticleSystemTemplate->m_isEmitAboveGroundOnly;
            break;
        case SWITCH_TOWARDS_EMITTER:
            val = m_currentParticleSystemTemplate->m_isParticleUpTowardsEmitter;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Switch(SwitchType type, const bool &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case SWITCH_HOLLOW:
            m_currentParticleSystemTemplate->m_isEmissionVolumeHollow = val;
            break;
        case SWITCH_ONE_SHOT:
            m_currentParticleSystemTemplate->m_isOneShot = val;
            break;
        case SWITCH_GROUND_ALIGNED:
            m_currentParticleSystemTemplate->m_isGroundAligned = val;
            break;
        case SWITCH_ABOVE_GROUND_ONLY:
            m_currentParticleSystemTemplate->m_isEmitAboveGroundOnly = val;
            break;
        case SWITCH_TOWARDS_EMITTER:
            m_currentParticleSystemTemplate->m_isParticleUpTowardsEmitter = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Initial_Delay(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_initialDelay.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_initialDelay.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Initial_Delay(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_initialDelay.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_initialDelay.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Burst_Delay(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_burstDelay.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_burstDelay.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Burst_Delay(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_burstDelay.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_burstDelay.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Burst_Count(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_burstCount.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_burstCount.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Burst_Count(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_burstCount.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_burstCount.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Color_Scale(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_colorScale.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_colorScale.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Color_Scale(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_colorScale.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_colorScale.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Lifetime(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_lifetime.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_lifetime.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Lifetime(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_lifetime.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_lifetime.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Start_Size(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_startSize.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_startSize.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Start_Size(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_startSize.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_startSize.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Start_Size_Rate(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_startSizeRate.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_startSizeRate.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Start_Size_Rate(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_startSizeRate.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_startSizeRate.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Size_Rate(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_sizeRate.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_sizeRate.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Size_Rate(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_sizeRate.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_sizeRate.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Size_Damping(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_sizeRateDamping.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_sizeRateDamping.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Size_Damping(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (minmax) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_sizeRateDamping.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_sizeRateDamping.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_System_Life(const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_currentParticleSystemTemplate->m_systemLifetime = val;
}

void ParticleSystemsDialog::Get_System_Life(float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_currentParticleSystemTemplate->m_systemLifetime;
}

void ParticleSystemsDialog::Set_Slave_Offset(XYZType xyz, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            m_currentParticleSystemTemplate->m_slavePosOffset.x = val;
            break;
        case VAL_Y:
            m_currentParticleSystemTemplate->m_slavePosOffset.y = val;
            break;
            break;
        case VAL_Z:
            m_currentParticleSystemTemplate->m_slavePosOffset.z = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Slave_Offset(XYZType xyz, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            val = m_currentParticleSystemTemplate->m_slavePosOffset.x;
            break;
        case VAL_Y:
            val = m_currentParticleSystemTemplate->m_slavePosOffset.y;
            break;
            break;
        case VAL_Z:
            val = m_currentParticleSystemTemplate->m_slavePosOffset.z;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Drift_Velocity(XYZType xyz, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            m_currentParticleSystemTemplate->m_driftVelocity.x = val;
            break;
        case VAL_Y:
            m_currentParticleSystemTemplate->m_driftVelocity.y = val;
            break;
            break;
        case VAL_Z:
            m_currentParticleSystemTemplate->m_driftVelocity.z = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Drift_Velocity(XYZType xyz, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            val = m_currentParticleSystemTemplate->m_driftVelocity.x;
            break;
        case VAL_Y:
            val = m_currentParticleSystemTemplate->m_driftVelocity.y;
            break;
            break;
        case VAL_Z:
            val = m_currentParticleSystemTemplate->m_driftVelocity.z;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Particle(const wxString &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_particleParams[PARAM_PARTICLE] = val;
}

void ParticleSystemsDialog::Get_Particle(wxString &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_particleParams[PARAM_PARTICLE].c_str();
}

void ParticleSystemsDialog::Set_Slave_System(const wxString &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_particleParams[PARAM_SLAVE_SYS] = val;
}

void ParticleSystemsDialog::Get_Slave_System(wxString &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_particleParams[PARAM_SLAVE_SYS].c_str();
}

void ParticleSystemsDialog::Set_Per_Particle(const wxString &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_particleParams[PARAM_PER_PARTICLE] = val;
}

void ParticleSystemsDialog::Get_Per_Particle(wxString &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_particleParams[PARAM_PER_PARTICLE].c_str();
}

void ParticleSystemsDialog::Set_Wind_Start_Angle(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = val / 180.0f * 3.1415927f;

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_windMotionStartAngleMin = angle;
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_windMotionStartAngleMin = angle;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Wind_Start_Angle(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = 0.0f;

    switch (minmax) {
        case VAL_MIN:
            angle = m_currentParticleSystemTemplate->m_windMotionStartAngleMin;
            break;
        case VAL_MAX:
            angle = m_currentParticleSystemTemplate->m_windMotionStartAngleMax;
            break;
        default:
            break;
    }

    val = angle * 180.0f / 3.1415927f;
}

void ParticleSystemsDialog::Set_Wind_End_Angle(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = val / 180.0f * 3.1415927f;

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_windMotionEndAngleMin = angle;
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_windMotionEndAngleMin = angle;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Wind_End_Angle(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = 0.0f;

    switch (minmax) {
        case VAL_MIN:
            angle = m_currentParticleSystemTemplate->m_windMotionEndAngleMin;
            break;
        case VAL_MAX:
            angle = m_currentParticleSystemTemplate->m_windMotionEndAngleMax;
            break;
        default:
            break;
    }

    val = angle * 180.0f / 3.1415927f;
}

void ParticleSystemsDialog::Set_Wind_Angle_Rate(MinMaxType minmax, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = val / 180.0f * 3.1415927f;

    switch (minmax) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_windAngleChangeMin = angle;
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_windAngleChangeMax = angle;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Wind_Angle_Rate(MinMaxType minmax, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    float angle = 0.0f;

    switch (minmax) {
        case VAL_MIN:
            angle = m_currentParticleSystemTemplate->m_windAngleChangeMin;
            break;
        case VAL_MAX:
            angle = m_currentParticleSystemTemplate->m_windAngleChangeMax;
            break;
        default:
            break;
    }

    val = angle * 180.0f / 3.1415927f;
}

void ParticleSystemsDialog::Set_Wind_Motion(const int &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_currentParticleSystemTemplate->m_windMotion = ParticleSystemInfo::WindMotion(val);
}

void ParticleSystemsDialog::Get_Wind_Motion(int &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_currentParticleSystemTemplate->m_windMotion;
}

void ParticleSystemsDialog::Set_Emission_Line_Properties(MinMaxXYZType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN_X:
            m_currentParticleSystemTemplate->m_emissionVolume.line.start.x = val;
            break;
        case VAL_MIN_Y:
            m_currentParticleSystemTemplate->m_emissionVolume.line.start.y = val;
            break;
        case VAL_MIN_Z:
            m_currentParticleSystemTemplate->m_emissionVolume.line.start.z = val;
            break;
        case VAL_MAX_X:
            m_currentParticleSystemTemplate->m_emissionVolume.line.end.x = val;
            break;
        case VAL_MAX_Y:
            m_currentParticleSystemTemplate->m_emissionVolume.line.end.y = val;
            break;
        case VAL_MAX_Z:
            m_currentParticleSystemTemplate->m_emissionVolume.line.end.z = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Emission_Line_Properties(MinMaxXYZType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN_X:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.start.x;
            break;
        case VAL_MIN_Y:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.start.y;
            break;
        case VAL_MIN_Z:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.start.z;
            break;
        case VAL_MAX_X:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.end.x;
            break;
        case VAL_MAX_Y:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.end.y;
            break;
        case VAL_MAX_Z:
            val = m_currentParticleSystemTemplate->m_emissionVolume.line.end.z;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Emission_Square_Properties(XYZType xyz, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            m_currentParticleSystemTemplate->m_emissionVolume.box.x = val;
            break;
        case VAL_Y:
            m_currentParticleSystemTemplate->m_emissionVolume.box.y = val;
            break;
        case VAL_Z:
            m_currentParticleSystemTemplate->m_emissionVolume.box.z = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Emission_Square_Properties(XYZType xyz, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (xyz) {
        case VAL_X:
            val = m_currentParticleSystemTemplate->m_emissionVolume.box.x;
            break;
        case VAL_Y:
            val = m_currentParticleSystemTemplate->m_emissionVolume.box.y;
            break;
        case VAL_Z:
            val = m_currentParticleSystemTemplate->m_emissionVolume.box.z;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Emission_Sphere_Properties(const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    m_currentParticleSystemTemplate->m_emissionVolume.sphere = val;
}

void ParticleSystemsDialog::Get_Emission_Sphere_Properties(float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    val = m_currentParticleSystemTemplate->m_emissionVolume.sphere;
}

void ParticleSystemsDialog::Set_Emission_Cylinder_Properties(CylinderType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case EM_CYLINDER_RADIUS:
            m_currentParticleSystemTemplate->m_emissionVolume.cylinder.radius = val;
            break;
        case EM_CYLINDER_LENGTH:
            m_currentParticleSystemTemplate->m_emissionVolume.cylinder.length = val;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Emission_Cylinder_Properties(CylinderType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case EM_CYLINDER_RADIUS:
            val =  m_currentParticleSystemTemplate->m_emissionVolume.cylinder.radius;
            break;
        case EM_CYLINDER_LENGTH:
            val = m_currentParticleSystemTemplate->m_emissionVolume.cylinder.length;
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Ortho_Properties(MinMaxXYZType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN_X:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.x.Set_Min(val);
            break;
        case VAL_MIN_Y:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.y.Set_Min(val);
            break;
        case VAL_MIN_Z:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.z.Set_Min(val);
            break;
        case VAL_MAX_X:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.x.Set_Max(val);
            break;
        case VAL_MAX_Y:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.y.Set_Max(val);
            break;
        case VAL_MAX_Z:
            m_currentParticleSystemTemplate->m_emissionVelocity.ortho.z.Set_Max(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Ortho_Properties(MinMaxXYZType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN_X:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.x.Get_Min();
            break;
        case VAL_MIN_Y:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.y.Get_Min();
            break;
        case VAL_MIN_Z:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.z.Get_Min();
            break;
        case VAL_MAX_X:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.x.Get_Max();
            break;
        case VAL_MAX_Y:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.y.Get_Max();
            break;
        case VAL_MAX_Z:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.ortho.z.Get_Max();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Spherical_Properties(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.spherical.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.spherical.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Spherical_Properties(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.spherical.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.spherical.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Hemispherical_Properties(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.hemispherical.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.hemispherical.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Hemispherical_Properties(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.hemispherical.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.hemispherical.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Cylindrical_Properties_Radial(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.radial.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.radial.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Cylindrical_Properties_Radial(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.radial.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.radial.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Cylindrical_Properties_Ortho(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.normal.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.normal.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Cylindrical_Properties_Ortho(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.normal.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.cylindrical.normal.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Outward_Properties_Major(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.outward.outward.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.outward.outward.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Outward_Properties_Major(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.outward.outward.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.outward.outward.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Set_Velocity_Outward_Properties_Minor(MinMaxType type, const float &val)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            m_currentParticleSystemTemplate->m_emissionVelocity.outward.other.Set_Min(val);
            break;
        case VAL_MAX:
            m_currentParticleSystemTemplate->m_emissionVelocity.outward.other.Set_Min(val);
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Get_Velocity_Outward_Properties_Minor(MinMaxType type, float &val) const
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    switch (type) {
        case VAL_MIN:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.outward.other.Get_Min();
            break;
        case VAL_MAX:
            val = m_currentParticleSystemTemplate->m_emissionVelocity.outward.other.Get_Min();
            break;
        default:
            break;
    }
}

void ParticleSystemsDialog::Init_Entries()
{
    wxTextValidator validator(wxFILTER_NUMERIC);
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_ZERO_AS_BLANK);
    m_partCap->SetValidator(validator);
    m_angleXMin->SetValidator(fpval);
    m_angleXMax->SetValidator(fpval);
    m_angleYMin->SetValidator(fpval);
    m_angleYMax->SetValidator(fpval);
    m_angleZMin->SetValidator(fpval);
    m_angleZMax->SetValidator(fpval);
    m_angRateXMin->SetValidator(fpval);
    m_angRateXMax->SetValidator(fpval);
    m_angRateYMin->SetValidator(fpval);
    m_angRateYMax->SetValidator(fpval);
    m_angRateZMin->SetValidator(fpval);
    m_angRateZMax->SetValidator(fpval);
    m_gravity->SetValidator(fpval);
    m_angDampMin->SetValidator(fpval);
    m_angDampMax->SetValidator(fpval);
    m_velDampMin->SetValidator(fpval);
    m_velDampMax->SetValidator(fpval);

    // Initialise the priority names, these names should match those found in gametype.cpp in the main engine.
    for (unsigned i = 1; s_priorityNames[i] != nullptr; ++i) {
        m_priorityCombo->Append(s_priorityNames[i]);
    }

    m_priorityCombo->SetSelection(0);

    // Initialise the emission names, see particlesysinfo.h for the enum they correspond to.
    for (unsigned i = 1; s_emissionTypes[i] != nullptr; ++i) {
        m_emissionTypeCombo->Append(s_emissionTypes[i]);
    }

    // Populate Simplebook for switching between panels for different emission types.
    m_emissionBook->AddPage(new EmissionPointPropertiesDialog(m_emissionBook), "Point Properties");
    m_emissionBook->AddPage(new EmissionLinePropertiesDialog(m_emissionBook), "Line Properties");
    m_emissionBook->AddPage(new EmissionBoxPropertiesDialog(m_emissionBook), "Box Properties");
    m_emissionBook->AddPage(new EmissionSpherePropertiesDialog(m_emissionBook), "Sphere Properties");
    m_emissionBook->AddPage(new EmissionCylinderPropertiesDialog(m_emissionBook), "Cylinder Properties");

    m_emissionTypeCombo->SetSelection(0);
    m_emissionBook->ChangeSelection(0);

    // Initialise the emission names, see particlesysinfo.h for the enum they correspond to.
    for (unsigned i = 1; s_velocityTypes[i] != nullptr; ++i) {
        m_velocityTypeCombo->Append(s_velocityTypes[i]);
    }

    // Populate Simplebook for switching between panels for different velocity types.
    m_velocityBook->AddPage(new OrthoVelocityPropertiesDialog(m_velocityBook), "Ortho Velocity");
    m_velocityBook->AddPage(new SphericalVelocityPropertiesDialog(m_velocityBook), "Spherical Velocity");
    m_velocityBook->AddPage(new HemisphericalVelocityPropertiesDialog(m_velocityBook), "Hemispherical Velocity");
    m_velocityBook->AddPage(new CylindricalVelocityPropertiesDialog(m_velocityBook), "Cylindrical Velocity");
    m_velocityBook->AddPage(new OutwardVelocityPropertiesDialog(m_velocityBook), "Outward Velocity");

    m_velocityTypeCombo->SetSelection(0);
    m_velocityBook->ChangeSelection(0);

    // Initialise the emission names, see particlesysinfo.h for the enum they correspond to.
    for (unsigned i = 1; s_parameterTypes[i] != nullptr; ++i) {
        m_particleTypeCombo->Append(s_parameterTypes[i]);
    }

    // Populate Simplebook for switching between panels for different particle types.
    m_parameterBook->AddPage(new _2DParticleParametersDialog(m_parameterBook), "2d Parameters");
    m_parameterBook->AddPage(new _3DParticleParametersDialog(m_parameterBook), "3d Parameters");
    m_parameterBook->AddPage(new StreakParticleParametersDialog(m_parameterBook), "Streak Parameters");
    m_parameterBook->AddPage(new wxPanel(m_parameterBook), "Volume Parameters");
    m_parameterBook->AddPage(new wxPanel(m_parameterBook), "Smudge Parameters");

    m_particleTypeCombo->SetSelection(0);
    m_parameterBook->ChangeSelection(0);

    // Initialise the emission names, see particlesysinfo.h for the enum they correspond to.
    for (unsigned i = 1; s_shaderTypes[i] != nullptr; ++i) {
        m_shaderTypeCombo->Append(s_shaderTypes[i]);
    }

    m_shaderTypeCombo->SetSelection(0);
    m_colorAlpha.Init_Entries();
    m_emitterSwitch.Init_Entries();
    m_moreParams.Init_Entries();
}

void ParticleSystemsDialog::Update(bool update_dialogs)
{
    if (m_currentParticleSystemTemplate == nullptr) {
        return;
    }

    if (update_dialogs) {
        // Handle the combo boxes.
        m_priorityCombo->SetStringSelection(s_priorityNames[m_currentParticleSystemTemplate->m_priority]);
        m_emissionTypeCombo->SetStringSelection(s_emissionTypes[m_currentParticleSystemTemplate->m_emissionVolumeType]);
        m_emissionBook->SetSelection(m_emissionTypeCombo->GetSelection());
        m_velocityTypeCombo->SetStringSelection(s_velocityTypes[m_currentParticleSystemTemplate->m_emissionVelocityType]);
        m_velocityBook->SetSelection(m_velocityTypeCombo->GetSelection());
        m_particleTypeCombo->SetStringSelection(s_parameterTypes[m_currentParticleSystemTemplate->m_particleType]);
        m_parameterBook->SetSelection(m_particleTypeCombo->GetSelection());
        m_shaderTypeCombo->SetStringSelection(s_shaderTypes[m_currentParticleSystemTemplate->m_shaderType]);

        // Handle values.
        char buff[128];
        // Zero Hour ABI does not have these members, but Generals and later SAGE games do.
#ifndef GAME_DLL
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleX.Get_Min());
        m_angleXMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleX.Get_Max());
        m_angleXMax->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleY.Get_Min());
        m_angleYMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleY.Get_Max());
        m_angleYMax->SetValue(buff);
#endif
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleZ.Get_Min());
        m_angleZMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angleZ.Get_Max());
        m_angleZMax->SetValue(buff);

#ifndef GAME_DLL
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateX.Get_Min());
        m_angRateXMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateX.Get_Max());
        m_angRateXMax->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateY.Get_Min());
        m_angRateYMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateY.Get_Max());
        m_angRateYMax->SetValue(buff);
#endif
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateZ.Get_Min());
        m_angRateZMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularRateZ.Get_Max());
        m_angRateZMax->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularDamping.Get_Min());
        m_angDampMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_angularDamping.Get_Max());
        m_angDampMax->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_velDamping.Get_Min());
        m_velDampMin->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_velDamping.Get_Max());
        m_velDampMax->SetValue(buff);
        std::snprintf(buff, sizeof(buff), "%.2f", m_currentParticleSystemTemplate->m_gravity);
        m_gravity->SetValue(buff);
    } else {
        // Handle the combo boxes.
        m_currentParticleSystemTemplate->m_priority = ParticlePriorityType(m_priorityCombo->GetSelection() + 1);
        m_currentParticleSystemTemplate->m_emissionVolumeType =
            ParticleSystemInfo::EmissionVolumeType(m_emissionTypeCombo->GetSelection() + 1);
        m_currentParticleSystemTemplate->m_emissionVelocityType =
            ParticleSystemInfo::EmissionVelocityType(m_velocityTypeCombo->GetSelection() + 1);
        m_currentParticleSystemTemplate->m_particleType =
            ParticleSystemInfo::ParticleType(m_particleTypeCombo->GetSelection() + 1);
        m_currentParticleSystemTemplate->m_shaderType =
            ParticleSystemInfo::ParticleShaderType(m_shaderTypeCombo->GetSelection() + 1);

        // Handle values.
#ifndef GAME_DLL
        m_currentParticleSystemTemplate->m_angleX.Set_Min(std::atof(m_angleXMin->GetValue()));
        m_currentParticleSystemTemplate->m_angleX.Set_Max(std::atof(m_angleXMax->GetValue()));
        m_currentParticleSystemTemplate->m_angleY.Set_Min(std::atof(m_angleYMin->GetValue()));
        m_currentParticleSystemTemplate->m_angleY.Set_Max(std::atof(m_angleYMax->GetValue()));
#endif
        m_currentParticleSystemTemplate->m_angleZ.Set_Min(std::atof(m_angleZMin->GetValue()));
        m_currentParticleSystemTemplate->m_angleZ.Set_Max(std::atof(m_angleZMax->GetValue()));

#ifndef GAME_DLL
        m_currentParticleSystemTemplate->m_angularRateX.Set_Min(std::atof(m_angRateXMin->GetValue()));
        m_currentParticleSystemTemplate->m_angularRateX.Set_Max(std::atof(m_angRateXMax->GetValue()));
        m_currentParticleSystemTemplate->m_angularRateY.Set_Min(std::atof(m_angRateYMin->GetValue()));
        m_currentParticleSystemTemplate->m_angularRateY.Set_Max(std::atof(m_angRateYMax->GetValue()));
#endif
        m_currentParticleSystemTemplate->m_angularRateZ.Set_Min(std::atof(m_angRateZMin->GetValue()));
        m_currentParticleSystemTemplate->m_angularRateZ.Set_Max(std::atof(m_angRateZMax->GetValue()));
        m_currentParticleSystemTemplate->m_angularDamping.Set_Min(std::atof(m_angDampMin->GetValue()));
        m_currentParticleSystemTemplate->m_angularDamping.Set_Max(std::atof(m_angDampMax->GetValue()));
        m_currentParticleSystemTemplate->m_velDamping.Set_Min(std::atof(m_velDampMin->GetValue()));
        m_currentParticleSystemTemplate->m_velDamping.Set_Max(std::atof(m_velDampMax->GetValue()));
        m_currentParticleSystemTemplate->m_gravity = std::atof(m_gravity->GetValue());
    }

    m_colorAlpha.Update(update_dialogs);
    m_emitterSwitch.Update(update_dialogs);
    m_moreParams.Update(update_dialogs);

    Updatable *update = dynamic_cast<Updatable *>(m_emissionBook->GetCurrentPage());

    if (update != nullptr) {
        update->Update(update_dialogs);
    }

    update = dynamic_cast<Updatable *>(m_velocityBook->GetCurrentPage());

    if (update != nullptr) {
        update->Update(update_dialogs);
    }

    update = dynamic_cast<Updatable *>(m_parameterBook->GetCurrentPage());

    if (update != nullptr) {
        update->Update(update_dialogs);
    }
}
