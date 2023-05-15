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

#include "coloralpha.h"
#include "emitswitch.h"
#include "moreparams.h"
#include "particlesystemplate.h"
#include <list>
#include <string>
#include <vector>

extern wxWindow *g_mainWindow;

class ParticleSystemsDialog : public PartEdBaseFrame
{
    enum : unsigned
    {
        PARAM_PARTICLE,
        PARAM_SLAVE_SYS,
        PARAM_PER_PARTICLE,
        PARAM_COUNT = 3,
    };

public:
    enum SwitchType
    {
        SWITCH_HOLLOW,
        SWITCH_ONE_SHOT,
        SWITCH_GROUND_ALIGNED,
        SWITCH_ABOVE_GROUND_ONLY,
        SWITCH_TOWARDS_EMITTER,
    };

    enum MinMaxType
    {
        VAL_MIN,
        VAL_MAX,
    };

    enum XYZType
    {
        VAL_X,
        VAL_Y,
        VAL_Z,
    };

    enum MinMaxXYZType
    {
        VAL_MIN_X,
        VAL_MIN_Y,
        VAL_MIN_Z,
        VAL_MAX_X,
        VAL_MAX_Y,
        VAL_MAX_Z,
    };

    enum CylinderType
    {
        EM_CYLINDER_RADIUS,
        EM_CYLINDER_LENGTH,
    };

public:
    ParticleSystemsDialog(wxWindow *parent, const wxString &label);

    // Menu wx event handlers
    void On_Request_Reload(wxCommandEvent &event);
    void On_Write(wxCommandEvent &event);
    void On_Reload_Texture(wxCommandEvent &event);
    void On_About(wxCommandEvent &event);

    // Frame control wx event handlers
    void On_Selected_System(wxCommandEvent &event);
    void On_Kill_All(wxCommandEvent &event);
    void On_Priority(wxCommandEvent &event);
    void On_Emission_Type(wxCommandEvent &event);
    void On_Velocity_Type(wxCommandEvent &event);
    void On_Particle_Type(wxCommandEvent &event);
    void On_Shader_Type(wxCommandEvent &event);
    void On_Color_Alpha(wxCommandEvent &event);
    void On_Switches(wxCommandEvent &event);
    void On_Continued(wxCommandEvent &event);
    void On_Changed(wxCommandEvent &event);
    void On_Cap_Update(wxCommandEvent &event) { m_updateParticleCap = true; }
    void On_Drop_System_Combo(wxCommandEvent &event);

    // wx Event Handler for the close event.
    void On_Exit(wxCloseEvent &event);

    // DLL API implementations
    void Append_System(const char *system);
    void Append_Thing(const char *thing);
    void Get_Selected_Particle_AsciiString_Parm(int32_t entry, char *name, void **temp);
    void Get_Selected_Particle_System_Name(char *name);
    void Remove_All_Particle_Systems();
    void Remove_All_Thing_Templates();
    void Update_Current_Particle_Cap(int32_t cap);
    void Update_Current_Particle_System(void *system);
    void Update_Particle_AsciiString_Parm(int32_t entry, const char *name, void **system);
    void Update_System_Use_Parameters(void *system);
    int32_t Get_New_Particle_Cap();
    bool Has_Updated_Selected_Particle_System();
    bool Should_Reload_Textures();
    bool Should_Update_Particle_Cap();
    bool Should_Write_INI();
    bool Has_Requested_Kill_All();
    bool Has_Requested_Reload();
    bool Should_Busy_Wait() { return m_busyWait; }

    // Wrappers for other classes to get/set the particle system
    void Get_Color_Key_Frame(int index, RGBColorKeyframe &key) const;
    void Set_Color_Key_Frame(int index, const RGBColorKeyframe &key);
    void Get_Alpha_Key_Frame(int index, ParticleSystemInfo::RandomKeyframe &key) const;
    void Set_Alpha_Key_Frame(int index, const ParticleSystemInfo::RandomKeyframe &key);
    void Get_Switch(SwitchType type, bool &val) const;
    void Set_Switch(SwitchType type, const bool &val);
    void Set_Initial_Delay(MinMaxType minmax, const float &val);
    void Get_Initial_Delay(MinMaxType minmax, float &val) const;
    void Set_Burst_Delay(MinMaxType minmax, const float &val);
    void Get_Burst_Delay(MinMaxType minmax, float &val) const;
    void Set_Burst_Count(MinMaxType minmax, const float &val);
    void Get_Burst_Count(MinMaxType minmax, float &val) const;
    void Set_Color_Scale(MinMaxType minmax, const float &val);
    void Get_Color_Scale(MinMaxType minmax, float &val) const;
    void Set_Lifetime(MinMaxType minmax, const float &val);
    void Get_Lifetime(MinMaxType minmax, float &val) const;
    void Set_Start_Size(MinMaxType minmax, const float &val);
    void Get_Start_Size(MinMaxType minmax, float &val) const;
    void Set_Start_Size_Rate(MinMaxType minmax, const float &val);
    void Get_Start_Size_Rate(MinMaxType minmax, float &val) const;
    void Set_Size_Rate(MinMaxType minmax, const float &val);
    void Get_Size_Rate(MinMaxType minmax, float &val) const;
    void Set_Size_Damping(MinMaxType minmax, const float &val);
    void Get_Size_Damping(MinMaxType minmax, float &val) const;
    void Set_System_Life(const float &val);
    void Get_System_Life(float &val) const;
    void Set_Slave_Offset(XYZType xyz, const float &val);
    void Get_Slave_Offset(XYZType xyz, float &val) const;
    void Set_Drift_Velocity(XYZType xyz, const float &val);
    void Get_Drift_Velocity(XYZType xyz, float &val) const;
    void Set_Particle(const wxString &val);
    void Get_Particle(wxString &val) const;
    void Set_Slave_System(const wxString &val);
    void Get_Slave_System(wxString &val) const;
    void Set_Per_Particle(const wxString &val);
    void Get_Per_Particle(wxString &val) const;
    void Set_Wind_Start_Angle(MinMaxType minmax, const float &val);
    void Get_Wind_Start_Angle(MinMaxType minmax, float &val) const;
    void Set_Wind_End_Angle(MinMaxType minmax, const float &val);
    void Get_Wind_End_Angle(MinMaxType minmax, float &val) const;
    void Set_Wind_Angle_Rate(MinMaxType minmax, const float &val);
    void Get_Wind_Angle_Rate(MinMaxType minmax, float &val) const;
    void Set_Wind_Motion(const int &val);
    void Get_Wind_Motion(int &val) const;
    void Set_Emission_Line_Properties(MinMaxXYZType type, const float &val);
    void Get_Emission_Line_Properties(MinMaxXYZType type, float &val) const;
    void Set_Emission_Square_Properties(XYZType xyz, const float &val);
    void Get_Emission_Square_Properties(XYZType xyz, float &val) const;
    void Set_Emission_Sphere_Properties(const float &val);
    void Get_Emission_Sphere_Properties(float &val) const;
    void Set_Emission_Cylinder_Properties(CylinderType type, const float &val);
    void Get_Emission_Cylinder_Properties(CylinderType type, float &val) const;
    void Set_Velocity_Ortho_Properties(MinMaxXYZType type, const float &val);
    void Get_Velocity_Ortho_Properties(MinMaxXYZType type, float &val) const;
    void Set_Velocity_Spherical_Properties(MinMaxType type, const float &val);
    void Get_Velocity_Spherical_Properties(MinMaxType type, float &val) const;
    void Set_Velocity_Hemispherical_Properties(MinMaxType type, const float &val);
    void Get_Velocity_Hemispherical_Properties(MinMaxType type, float &val) const;
    void Set_Velocity_Cylindrical_Properties_Radial(MinMaxType type, const float &val);
    void Get_Velocity_Cylindrical_Properties_Radial(MinMaxType type, float &val) const;
    void Set_Velocity_Cylindrical_Properties_Ortho(MinMaxType type, const float &val);
    void Get_Velocity_Cylindrical_Properties_Ortho(MinMaxType type, float &val) const;
    void Set_Velocity_Outward_Properties_Major(MinMaxType type, const float &val);
    void Get_Velocity_Outward_Properties_Major(MinMaxType type, float &val) const;
    void Set_Velocity_Outward_Properties_Minor(MinMaxType type, const float &val);
    void Get_Velocity_Outward_Properties_Minor(MinMaxType type, float &val) const;

    const wxArrayString &Get_System_List() const { return m_particleSystemsList; }
    const std::list<wxString> &Get_Template_List() const { return m_thingTemplateList; }

private:
    void Init_Entries();
    void Update(bool update_dialogs);

protected:
    ColorAndAlphaDialog m_colorAlpha;
    EmissionSwitchesDialog m_emitterSwitch;
    MoreParametersDialog m_moreParams;
    std::vector<wxString> m_particleParams;
    wxArrayString m_particleSystemsList;
    std::list<wxString> m_thingTemplateList;
    ParticleSystemTemplate *m_currentParticleSystemTemplate;
    bool m_hasRequestedKillAll;
    bool m_hasRequestedReload;
    bool m_updateSelectedSystem;
    bool m_busyWait;
    bool m_reloadTextures;
    bool m_updateParticleCap;
    bool m_writeINI;
    bool m_systemListDirty;

    static const char *s_priorityNames[];
    static const char *s_emissionTypes[];
    static const char *s_velocityTypes[];
    static const char *s_parameterTypes[];
    static const char *s_shaderTypes[];
};
