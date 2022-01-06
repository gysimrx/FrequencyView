#include "NetworkAnalyzerPanel.h"

#include <wx/log.h>
#include <wx/sizer.h>
#include <wx/textfile.h>

#include <wx/plotctrl/plotctrl.h>

#include "SessionEvent.h"
#include "SessionThread.h"
#include "SpectrumFeeder.h"

BEGIN_EVENT_TABLE(NetworkAnalyzerPanel, wxPanel)
    EVT_PLOTCTRL_CURVE_SEL_CHANGING  (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_CURVE_SEL_CHANGED   (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_MOUSE_MOTION        (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_CLICKED             (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_DOUBLECLICKED       (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_POINT_CLICKED       (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_POINT_DOUBLECLICKED (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_AREA_SEL_CREATING   (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_AREA_SEL_CHANGING   (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_AREA_SEL_CREATED    (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_VIEW_CHANGING       (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_VIEW_CHANGED        (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_CURSOR_CHANGING     (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_CURSOR_CHANGED      (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_MOUSE_FUNC_CHANGING (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)
    EVT_PLOTCTRL_MOUSE_FUNC_CHANGED  (wxID_ANY, NetworkAnalyzerPanel::OnPlotCtrlEvent)

    EVT_SESSION_UPDATE               (wxID_ANY, NetworkAnalyzerPanel::OnSessionUpdate)
    EVT_SESSION_ENDED                (wxID_ANY, NetworkAnalyzerPanel::OnSessionEnded)

    EVT_MENU                         (wxID_ANY, NetworkAnalyzerPanel::OnPlot)
    EVT_UPDATE_UI                    (wxID_ANY, NetworkAnalyzerPanel::OnUpdatePlot)
END_EVENT_TABLE()

NetworkAnalyzerPanel::NetworkAnalyzerPanel(wxWindow *parent, wxEvtHandler *evtHandler, wxWindowID winid,
                    std::shared_ptr<sigrok::Session> session,
                    std::shared_ptr<sigrok::HardwareDevice> device)
:
    wxPanel(parent),
    plotCtrl_(nullptr),
    session_(session),
    frequency_(1900000000.0),
    span_(1500000000.0),
    rbw_(100000)
{
    initPlotControl();

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(plotCtrl_, 1, wxEXPAND);
    SetSizerAndFit(sizer);

    device->config_set(sigrok::ConfigKey::PRESET,                Glib::Variant<bool>::create(true));
    device->config_set(sigrok::ConfigKey::SPAN,                  Glib::Variant<gdouble>::create(span_));
    device->config_set(sigrok::ConfigKey::BAND_CENTER_FREQUENCY, Glib::Variant<gdouble>::create(frequency_));
    Glib::ustring sparams = "S22,S21,S12,S11";
    device->config_set(sigrok::ConfigKey::SPARAMS,               Glib::Variant<Glib::ustring>::create(sparams));


    /// demo to send command !!!CALC1:DATA? FDATA
    Glib::ustring cmd = "freq:cent 1500000000";
    device->config_set(sigrok::ConfigKey::COMMAND_SET, Glib::Variant<Glib::ustring>::create(cmd));


    /// demo to read back !!!
    Glib::ustring req_cmd = "CONF:TRAC:CAT?";
    device->config_set(sigrok::ConfigKey::COMMAND_REQ, Glib::Variant<Glib::ustring>::create(req_cmd));
    Glib::ustring ans = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(device->config_get(sigrok::ConfigKey::COMMAND_REQ)).get();
    wxLogMessage(wxString::Format("received answer from device: %s", ans.data()));

    /// get active Traces via SPARAMS protocol Setting!!!
    Glib::ustring ans2 = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(device->config_get(sigrok::ConfigKey::SPARAMS)).get();
    wxLogMessage(wxString::Format("active Traces: %s", ans2.data()));
    int i = 0;
    while(i != -1)
    {
        i = ans2.find(',');
        Glib::ustring substring = ans2.substr(0,i);
        ans2.erase(0,i+1);
        wxLogMessage(wxString::Format("subsring =  %s", substring.data()));
    }

    SpectrumFeeder *df = new SpectrumFeeder(evtHandler, GetId());
    auto feed_callback = [=] (std::shared_ptr<sigrok::Device> device, std::shared_ptr<sigrok::Packet> packet)
    { df->data_feed_callback(device, packet); };
    session->add_datafeed_callback(feed_callback);
    auto stop_callback = [=] () { delete df; };
    session->set_stopped_callback(stop_callback);


    if ((new SessionThread(session_))->Run() != wxTHREAD_NO_ERROR)
        wxLogMessage("Can't create the thread!");

}

NetworkAnalyzerPanel::~NetworkAnalyzerPanel()
{
    session_->stop();
    if (session_.use_count() == 1)
        for (auto dev : session_->devices())
            dev->close();
}

void NetworkAnalyzerPanel::initPlotControl()
{
    plotCtrl_ = new wxPlotCtrl(this, wxID_ANY, wxPlotCtrl::GridType::SmithChart);
    plotCtrl_->SetBottomAxisLabel("f [Hz]");
    plotCtrl_->SetLeftAxisLabel("a [dBfs]");
    plotCtrl_->SetShowBottomAxis(true);
    plotCtrl_->SetShowLeftAxis(true);
    plotCtrl_->SetShowBottomAxisLabel(true);
    plotCtrl_->SetShowLeftAxisLabel(true);
    plotCtrl_->SetShowKey(false);
    plotCtrl_->SetShowPlotTitle(false);
    plotCtrl_->SetDrawGrid();
}

void NetworkAnalyzerPanel::OnPlot(wxCommandEvent &event)
{
//    if (event.GetId() == ID_DRAW_SYMBOLS)     plotCtrl_->SetDrawSymbols(event.IsChecked());
//    else if (event.GetId() == ID_DRAW_LINES)  plotCtrl_->SetDrawLines(event.IsChecked());
//    else if (event.GetId() == ID_DRAW_SPLINE) plotCtrl_->SetDrawSpline(event.IsChecked());
}

void NetworkAnalyzerPanel::OnUpdatePlot(wxUpdateUIEvent &event)
{
//    if (event.GetId() == ID_DRAW_SYMBOLS)     event.Check(plotCtrl_->GetDrawSymbols());
//    else if (event.GetId() == ID_DRAW_LINES)  event.Check(plotCtrl_->GetDrawLines());
//    else if (event.GetId() == ID_DRAW_SPLINE) event.Check(plotCtrl_->GetDrawSpline());
}

void NetworkAnalyzerPanel::OnPlotCtrlEvent(wxPlotCtrlEvent& event)
{
    event.Skip();

    if (!plotCtrl_) return; // wait until the pointer is set

    wxEventType eventType = event.GetEventType();
    int active_index = plotCtrl_->GetActiveIndex();

    if (eventType == wxEVT_PLOTCTRL_ERROR)
    {
        //SetStatusText(event.GetString());
    }
    else if (eventType == wxEVT_PLOTCTRL_MOUSE_MOTION)
    {

        wxString s = wxString::Format(wxT("Mouse (%g %g) Active index %d, "),
                                      event.GetX(), event.GetY(), active_index);

        if (plotCtrl_->IsCursorValid())
        {
            wxPoint2DDouble cursorPt = plotCtrl_->GetCursorPoint();
            s += wxString::Format(wxT("Cursor curve %d, data index %d, point (%g %g)"),
                plotCtrl_->GetCursorCurveIndex(), plotCtrl_->GetCursorDataIndex(),
                cursorPt.m_x, cursorPt.m_y);
        }

        //SetStatusText(s);
    }
    else if (eventType == wxEVT_PLOTCTRL_VIEW_CHANGING)
    {
        //SetStatusText(wxString::Format(wxT("View Changing %g %g"), event.GetX(), event.GetY()));
    }
    else if (eventType == wxEVT_PLOTCTRL_VIEW_CHANGED)
    {
        //SetStatusText(wxString::Format(wxT("View Changed %g %g"), event.GetX(), event.GetY()));
    }
    else
    {
         wxLogMessage(wxString::Format(wxT("%s xy(%g %g) CurveIndex %d, IsDataCurve %d DataIndex %d, MouseFn %d\n"),
            "wxPlotCtrlEvent::GetEventName(eventType).c_str()",
            event.GetX(), event.GetY(), event.GetCurveIndex(),
            event.GetCurveDataIndex(), event.GetMouseFunction()));
    }
}



void NetworkAnalyzerPanel::OnSessionUpdate(SessionEvent &evt)
{
    static int not_taken = 0;
    if (not_taken++ < 20)
        return;
    not_taken = 0;

    size_t channels;
    size_t *lengths;
    double **data;
    evt.GetData(&data, &lengths, &channels);
//    if (channels < 4 )
//    {
//        wxLogMessage("Channel Mismatch check SpectrumFeeder and Protocol");
//        delete [] lengths;
//        if (data)
//            for (size_t i = 0 ; i < channels; ++i)
//                delete [] (data[i]);
//        delete [] data;
//        return;
//    }

    size_t totallen = lengths[0];        // = 201 SweepPoints * 2 values per pnt (R;I) = 402
    size_t len = totallen/2;

    double *p[channels];
    for (size_t i = 0; i < channels; i++)
        p[i] = data[i];

    double** I = new double*[channels];
    for (size_t i = 0; i < channels; i++)
        I[i] = new double[len];

    double** R = new double*[channels];
    for (size_t i = 0; i < channels; i++)
        R[i] = new double[len];

    for (size_t i = 0 ; i < totallen ; ++i)
    {
        if(i % 2 )
        {
            for(size_t channel = 0; channel < channels; channel++)
                I[channel][i/2] = *(p[channel]+i);
        }
        else
        {
            for(size_t channel = 0; channel < channels; channel++)
                R[channel][i/2] = *(p[channel]+i);
        }
    }

    wxPlotData* pltData[channels];
    for (size_t i = 0; i<channels; i++)
        pltData[i] = new wxPlotData(R[i], I[i], len);

    if (plotCtrl_->GetCurveCount())
        plotCtrl_->DeleteCurve(-1, false);

    for (size_t i = 0; i<channels; i++)
        plotCtrl_->AddCurve(pltData[i]);

    delete R;
    delete I;
    /** add start, stop, center and sweep-pts to static text field below plot window */
}

void NetworkAnalyzerPanel::OnSessionEnded(SessionEvent&)
{
    wxLogMessage("Session ended\n");
}

