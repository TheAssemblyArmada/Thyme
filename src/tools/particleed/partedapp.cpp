#include "partedapp.h"
#include "partedframe.h"
#include <captainslog.h>

#include <wx/xrc/xh_all.h>
#include <wx/xrc/xmlres.h>

const int CMD_SHOW_WINDOW = wxNewId();
const int CMD_TERMINATE = wxNewId();

PartEdApp::PartEdApp() : m_frame(nullptr)
{
    // Keep the wx "main" thread running even without windows. This greatly
    // simplifies threads handling, because we don't have to correctly
    // implement wx-thread restarting.
    //
    // Note that this only works if you don't explicitly call ExitMainLoop(),
    // except in reaction to wx_dll_cleanup()'s message. wx_dll_cleanup()
    // relies on the availability of wxApp instance and if the event loop
    // terminated, wxEntry() would return and wxApp instance would be
    // destroyed.
    //
    // Also note that this is efficient, because if there are no windows, the
    // thread will sleep waiting for a new event. We could safe some memory
    // by shutting the thread down when it's no longer needed, though.
    SetExitOnFrameDelete(false);

    // Connect(CMD_SHOW_WINDOW, wxEVT_THREAD, wxThreadEventHandler(PartEdApp::OnShowWindow));
    // Connect(CMD_TERMINATE, wxEVT_THREAD, wxThreadEventHandler(PartEdApp::OnTerminate));
    Bind(wxEVT_THREAD, &PartEdApp::OnShowWindow, this, CMD_SHOW_WINDOW);
    Bind(wxEVT_THREAD, &PartEdApp::OnTerminate, this, CMD_TERMINATE);
}

bool PartEdApp::OnInit()
{
    wxXmlResource *res = wxXmlResource::Get();
    res->AddHandler(new wxUnknownWidgetXmlHandler);
    res->AddHandler(new wxComboBoxXmlHandler);
    res->AddHandler(new wxCheckBoxXmlHandler);
    res->AddHandler(new wxDialogXmlHandler);
    res->AddHandler(new wxChoiceXmlHandler);
    res->AddHandler(new wxPanelXmlHandler);
    res->AddHandler(new wxSizerXmlHandler);
    res->AddHandler(new wxFrameXmlHandler);
    res->AddHandler(new wxScrolledWindowXmlHandler);
    res->AddHandler(new wxMenuBarXmlHandler);
    res->AddHandler(new wxMenuXmlHandler);
    res->AddHandler(new wxStaticTextXmlHandler);
    res->AddHandler(new wxSimplebookXmlHandler);
    res->AddHandler(new wxTextCtrlXmlHandler);
    res->AddHandler(new wxButtonXmlHandler);
    res->AddHandler(new wxColourPickerCtrlXmlHandler);
    res->AddHandler(new wxToggleButtonXmlHandler);
    res->AddHandler(new wxListBoxXmlHandler);
    InitXmlResource();

    captainslog_init(LOGLEVEL_DEBUG, "PartedLog.txt", true, false, false);

    return true;
}

void PartEdApp::OnShowWindow(wxThreadEvent &event)
{
    m_frame = new ParticleSystemsDialog(NULL, event.GetString());
    m_frame->Show(true);
}

void PartEdApp::OnTerminate(wxThreadEvent &WXUNUSED(event))
{
    ExitMainLoop();
}

// we can't have WinMain() in a DLL and want to start the app ourselves
wxIMPLEMENT_APP_NO_MAIN(PartEdApp);
