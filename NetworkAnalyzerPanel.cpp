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
    frequency_(900000000.0),
    span_(100000000.0),
    rbw_(100000)
{
    initPlotControl();

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(plotCtrl_, 1, wxEXPAND);
    SetSizerAndFit(sizer);

//    device->config_set(sigrok::ConfigKey::PRESET,                Glib::Variant<bool>::create(true));

    device->config_set(sigrok::ConfigKey::SPAN,                  Glib::Variant<gdouble>::create(span_));
    device->config_set(sigrok::ConfigKey::BAND_CENTER_FREQUENCY, Glib::Variant<gdouble>::create(frequency_));
//    device->dev_acquisition_start():
//    device->config_set(sigrok::ConfigKey::RESOLUTION_BANDWIDTH,  Glib::Variant<uint64_t>::create(rbw_));
//    ref_level_ = static_cast<double>(Glib::VariantBase::cast_dynamic<Glib::Variant<gdouble>>(device->config_get(sigrok::ConfigKey::REF_LEVEL)).get());

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
    plotCtrl_ = new wxPlotCtrl(this, wxID_ANY);

    plotCtrl_->SetXAxisLabel("f [Hz]");
    plotCtrl_->SetYAxisLabel("P [dBm]");
    plotCtrl_->SetShowXAxis(true);
    plotCtrl_->SetShowYAxis(true);
    plotCtrl_->SetShowXAxisLabel(true);
    plotCtrl_->SetShowYAxisLabel(true);
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
            (int)event.IsDataCurve(), event.GetCurveDataIndex(),
            event.GetMouseFunction()));
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
    if (channels != 1 )
    {
        delete [] lengths;
        if (data)
            for (size_t i = 0 ; i < channels; ++i)
                delete [] (data[i]);
        delete [] data;
        return;
    }
    size_t len = lengths[0]/4;

    double *data_array = *data;
    double *p  = &(data_array)[0];
    double *p2 = new double[len];
    double *p3 = new double[len];
    double *p4 = new double[len];
    memcpy(p2, data_array +     len, len*sizeof(double));
    memcpy(p3, data_array + 2 * len, len*sizeof(double));
    memcpy(p4, data_array + 3 * len, len*sizeof(double));


    double *f  = new double[len];
    double *f2 = new double[len];
    double *f3 = new double[len];
    double *f4 = new double[len];

    for (size_t i = 0 ; i < len ; ++i)
        f[i] = frequency_ - span_ / 2.0 + span_/2.0/len + i*span_/len;

    wxLogMessage(std::to_string(len).c_str());
    memcpy(f2,f,len*sizeof(double));
    memcpy(f3,f,len*sizeof(double));
    memcpy(f4,f,len*sizeof(double));

    wxPlotData *pltData  = new wxPlotData(f , p , len);
    wxPlotData *pltData2 = new wxPlotData(f2, p2, len);
    wxPlotData *pltData3 = new wxPlotData(f3, p3, len);
    wxPlotData *pltData4 = new wxPlotData(f4, p4, len);

    pltData->SetBoundingRect(wxRect2DDouble(f[0], ref_level_-50.0, f[len-1]-f[0], 50.0));

    if (plotCtrl_->GetCurveCount())
        plotCtrl_->DeleteCurve(-1, false);


    plotCtrl_->AddCurve(pltData);
    plotCtrl_->AddCurve(pltData2);
    plotCtrl_->AddCurve(pltData3);
    plotCtrl_->AddCurve(pltData4);

    /** add start, stop, center and sweep-pts to static text field below plot window */
}

void NetworkAnalyzerPanel::OnSessionEnded(SessionEvent&)
{
    wxLogMessage("Session ended\n");
}

