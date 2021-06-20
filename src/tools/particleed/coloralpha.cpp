#include "coloralpha.h"
#include "partedapp.h"
#include "partedframe.h"
#include <wx/valnum.h>
#include <wx/valtext.h>

ColorAndAlphaDialog::ColorAndAlphaDialog(wxWindow *parent) : ColorAlphaBase(parent) 
{
    // Convenience array to allow iteration in other class methods.
    m_controls[0].color_picker = m_colKeyCheck1;
    m_controls[0].color_frame = m_colKeyFrame1;
    m_controls[0].alpha_min = m_alphaKeyMin1;
    m_controls[0].alpha_max = m_alphaKeyMax1;
    m_controls[0].alpha_frame = m_alphaKeyFrame1;
    m_controls[1].color_picker = m_colKeyCheck2;
    m_controls[1].color_frame = m_colKeyFrame2;
    m_controls[1].alpha_min = m_alphaKeyMin2;
    m_controls[1].alpha_max = m_alphaKeyMax2;
    m_controls[1].alpha_frame = m_alphaKeyFrame2;
    m_controls[2].color_picker = m_colKeyCheck3;
    m_controls[2].color_frame = m_colKeyFrame3;
    m_controls[2].alpha_min = m_alphaKeyMin3;
    m_controls[2].alpha_max = m_alphaKeyMax3;
    m_controls[2].alpha_frame = m_alphaKeyFrame3;
    m_controls[3].color_picker = m_colKeyCheck4;
    m_controls[3].color_frame = m_colKeyFrame4;
    m_controls[3].alpha_min = m_alphaKeyMin4;
    m_controls[3].alpha_max = m_alphaKeyMax4;
    m_controls[3].alpha_frame = m_alphaKeyFrame4;
    m_controls[4].color_picker = m_colKeyCheck5;
    m_controls[4].color_frame = m_colKeyFrame5;
    m_controls[4].alpha_min = m_alphaKeyMin5;
    m_controls[4].alpha_max = m_alphaKeyMax5;
    m_controls[4].alpha_frame = m_alphaKeyFrame5;
    m_controls[5].color_picker = m_colKeyCheck6;
    m_controls[5].color_frame = m_colKeyFrame6;
    m_controls[5].alpha_min = m_alphaKeyMin6;
    m_controls[5].alpha_max = m_alphaKeyMax6;
    m_controls[5].alpha_frame = m_alphaKeyFrame6;
    m_controls[6].color_picker = m_colKeyCheck7;
    m_controls[6].color_frame = m_colKeyFrame7;
    m_controls[6].alpha_min = m_alphaKeyMin7;
    m_controls[6].alpha_max = m_alphaKeyMax7;
    m_controls[6].alpha_frame = m_alphaKeyFrame7;
    m_controls[7].color_picker = m_colKeyCheck8;
    m_controls[7].color_frame = m_colKeyFrame8;
    m_controls[7].alpha_min = m_alphaKeyMin8;
    m_controls[7].alpha_max = m_alphaKeyMax8;
    m_controls[7].alpha_frame = m_alphaKeyFrame8;

    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck1"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck2"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck3"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck4"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck5"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck6"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck7"));
    Bind(wxEVT_COLOURPICKER_CHANGED, &ColorAndAlphaDialog::On_Changed, this, XRCID("m_colKeyCheck8"));

    for (int i = 0; i < 8; ++i) {
        m_controls[0].color_frame->Bind(wxEVT_COMMAND_KILL_FOCUS, &ColorAndAlphaDialog::On_Changed, this);
        m_controls[0].alpha_min->Bind(wxEVT_COMMAND_KILL_FOCUS, &ColorAndAlphaDialog::On_Changed, this);
        m_controls[0].alpha_max->Bind(wxEVT_COMMAND_KILL_FOCUS, &ColorAndAlphaDialog::On_Changed, this);
        m_controls[0].alpha_frame->Bind(wxEVT_COMMAND_KILL_FOCUS, &ColorAndAlphaDialog::On_Changed, this);
    }
}

void ColorAndAlphaDialog::On_Changed(wxCommandEvent &event)
{
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    wxframe->On_Changed(event);
}

void ColorAndAlphaDialog::Init_Entries()
{
    wxTextValidator validator(wxFILTER_NUMERIC);
    wxFloatingPointValidator<float> fpval(2, nullptr, wxNUM_VAL_ZERO_AS_BLANK);

    for (int i = 0; i < 8; ++i) {
        m_controls[i].color_frame->SetValidator(validator);
        m_controls[i].alpha_frame->SetValidator(validator);
        m_controls[i].alpha_min->SetValidator(fpval);
        m_controls[i].alpha_max->SetValidator(fpval);
    }
}

void ColorAndAlphaDialog::Update(bool update_dialogs)
{
    static_assert(ParticleSystemInfo::KEYFRAME_COUNT <= 8, "Control array size is too small and needs adjusting.");
    ParticleSystemsDialog *wxframe = wxGetApp().Frame();

    if (wxframe == nullptr) {
        return;
    }

    if (update_dialogs) {
        for (int i = 0; i < ParticleSystemInfo::KEYFRAME_COUNT; ++i) {
            char buff[128];

            // Handle Color
            RGBColorKeyframe color_key;
            wxframe->Get_Color_Key_Frame(i, color_key);
            std::snprintf(buff, sizeof(buff), "%" PRIu32, color_key.frame);
            m_controls[i].color_picker->SetColour(wxColour(color_key.color.red, color_key.color.green, color_key.color.blue));
            m_controls[i].color_frame->SetValue(buff);

            // Handle Alpha
            ParticleSystemInfo::RandomKeyframe alpha_key;
            wxframe->Get_Alpha_Key_Frame(i, alpha_key);
            std::snprintf(buff, sizeof(buff), "%" PRIu32, alpha_key.frame);
            m_controls[i].alpha_frame->SetValue(buff);
            std::snprintf(buff, sizeof(buff), "%.2f", alpha_key.var.Get_Min());
            m_controls[i].alpha_min->SetValue(buff);
            std::snprintf(buff, sizeof(buff), "%.2f", alpha_key.var.Get_Max());
            m_controls[i].alpha_max->SetValue(buff);
        }
    } else {
        for (int i = 0; i < ParticleSystemInfo::KEYFRAME_COUNT; ++i) {
            // Handle Color
            RGBColorKeyframe color_key;
            wxColour color = m_controls[i].color_picker->GetColour();
            color_key.color.red = color.Red() / 255.0f;
            color_key.color.green = color.Green() / 255.0f;
            color_key.color.blue = color.Blue() / 255.0f;
            color_key.frame = std::atoi(m_controls[i].color_frame->GetValue());
            wxframe->Set_Color_Key_Frame(i, color_key);

            // Handle Alpha
            ParticleSystemInfo::RandomKeyframe alpha_key;
            wxframe->Get_Alpha_Key_Frame(i, alpha_key);
            alpha_key.frame = std::atoi(m_controls[i].alpha_frame->GetValue());
            alpha_key.var.Set_Min(std::atof(m_controls[i].alpha_min->GetValue()));
            alpha_key.var.Set_Max(std::atof(m_controls[i].alpha_max->GetValue()));
            wxframe->Set_Alpha_Key_Frame(i, alpha_key);
        }
    }
}
