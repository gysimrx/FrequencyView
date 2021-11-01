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

    wxTextFile file("/home/marx/sigrok/FrequencyView/DebugFreqView");
    if( file.Exists() )
      file.Open();
    else
      file.Create();
    file.Clear();
    file.AddLine("onSessionUpdate");
    file.Write();
    file.Close();

    size_t channels;
    size_t *lengths;
    double **data;
    evt.GetData(&data, &lengths, &channels);
    if (channels != 1 )
    {
        wxLogMessage("Channel Mismatch check SpectrumFeeder and Protocol");
        delete [] lengths;
        if (data)
            for (size_t i = 0 ; i < channels; ++i)
                delete [] (data[i]);
        delete [] data;
        return;
    }

    size_t totallen = lengths[0];        // = 201 SweepPoints * 2 values per pnt (R;I) * 4 S-Param Traces = Total 1608
    size_t len = totallen/4/2;
    size_t itlen = totallen/4;
    wxLogMessage(std::to_string(totallen).c_str());
    file.Open();
    file.AddLine(std::to_string(totallen).c_str());
    file.Write();
    file.Close();

    double *R1 = new double[len];
    double *R2 = new double[len];
    double *R3 = new double[len];
    double *R4 = new double[len];

    double *I1 = new double[len];
    double *I2 = new double[len];
    double *I3 = new double[len];
    double *I4 = new double[len];

    double *data_array = *data;
    double *p1  = &(data_array)[0];
    data_array = data[1];
    double *p2  = &(data_array)[1];
    data_array = data[2];
    double *p3  = &(data_array)[2];
    data_array = data[3];
    double *p4  = &(data_array)[3];

    if(channels >= 1)
    {
        for (size_t i = 0 ; i < itlen ; ++i)
        {
            if(i % 2 )
            {
                I1[i/2] = *(p1+i);
                I2[i/2] = *(p1+i+itlen);
                I3[i/2] = *(p1+i+itlen*2);
                I4[i/2] = *(p1+i+itlen*3);

            }
            else
            {
                R1[i/2] = *(p1+i);
                R2[i/2] = *(p1+i+itlen);
                R3[i/2] = *(p1+i+itlen*2);
                R4[i/2] = *(p1+i+itlen*3);

            }
        }
    }

    wxPlotData *pltData1 = new wxPlotData(I1, R1, len);
    wxPlotData *pltData2 = new wxPlotData(R2, I2, len);
    wxPlotData *pltData3 = new wxPlotData(I3, R3, len);
    wxPlotData *pltData4 = new wxPlotData(I4, R4, len);

    pltData1->SetBoundingRect(wxRect2DDouble(I1[0], ref_level_-2.0, I1[len-1]-I1[0], 4.0));

    if (plotCtrl_->GetCurveCount())
        plotCtrl_->DeleteCurve(-1, false);

    plotCtrl_->AddCurve(pltData1); // S21
    plotCtrl_->AddCurve(pltData2); // S11
    plotCtrl_->AddCurve(pltData3); // S22
    plotCtrl_->AddCurve(pltData4); // S12

    /** add start, stop, center and sweep-pts to static text field below plot window */
}

void NetworkAnalyzerPanel::OnSessionEnded(SessionEvent&)
{
    wxLogMessage("Session ended\n");
}

