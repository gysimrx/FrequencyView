#ifndef FREQUENCYVIEWMAIN_H
#define FREQUENCYVIEWMAIN_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <wx/frame.h>
#include <wx/timer.h>
#include <wx/menu.h>

#include <wx/aui/aui.h>

#include "ActiveTraces.h"

class wxTextCtrl;
class SessionEvent;
class wxConfigBase;

namespace sigrok{
    class Context;
    class Session;
    class HardwareDevice;
};

class FrequencyViewFrame: public wxFrame
{
public:
    FrequencyViewFrame(wxFrame *frame, const wxString& title);
    ~FrequencyViewFrame();
private:
    wxAuiManager auiManager_;
    wxAuiNotebook* auiNotebook_;
    wxTextCtrl *logTxtCtrl_;

    std::shared_ptr<sigrok::Context> context_;

    wxConfigBase *config_;
    std::map<int, std::string> inputFormatCmdId_;

    void initLogicAnalyzer(std::shared_ptr<sigrok::Session> session, std::shared_ptr<sigrok::HardwareDevice> device);
    void initSpectrumAnalyzer(std::shared_ptr<sigrok::Session> session, std::shared_ptr<sigrok::HardwareDevice> device);
    void initNetworkAnalyzer(std::shared_ptr<sigrok::Session> session, std::shared_ptr<sigrok::HardwareDevice> device);

    void onExit(wxCommandEvent &event);
    void onAbout(wxCommandEvent &event);
    void onConnect(wxCommandEvent &event);
    void onImport(wxCommandEvent &event);
    void onSessionEvent(SessionEvent&);
    void OnAuiNotebook(wxAuiNotebookEvent &event);
    void OnPopupClick(wxCommandEvent &evt);

    ActiveTraces* Traces1 = new ActiveTraces();
    ActiveTraces* Traces2 = new ActiveTraces();
    wxMenu *TabPopupMenu1 = new wxMenu("");
    wxMenu *TabPopupMenu2 = new wxMenu("");
    wxMenu *TraceSelection1 = new wxMenu("");
    wxMenu *TraceSelection2 = new wxMenu("");

    void populateFileLoaderMenu(wxMenu *load);
    wxString generateWildcardString(const std::string &description, const std::vector<std::string> &exts);
    wxString getImportfileFromUser(const wxString &wildcard);
    DECLARE_EVENT_TABLE()
};


#endif
