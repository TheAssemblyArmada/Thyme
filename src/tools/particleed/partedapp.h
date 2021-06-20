#pragma once

#include <wx/app.h>

extern const int CMD_SHOW_WINDOW;
extern const int CMD_TERMINATE;

class ParticleSystemsDialog;

class PartEdApp : public wxApp
{
public:
    PartEdApp();

    virtual bool OnInit() override;

    ParticleSystemsDialog *Frame() { return m_frame; }

private:
    void OnShowWindow(wxThreadEvent &event);
    void OnTerminate(wxThreadEvent &event);

private:
    ParticleSystemsDialog *m_frame;
};

wxDECLARE_APP(PartEdApp);
