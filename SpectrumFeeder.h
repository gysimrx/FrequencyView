#ifndef SPECTRUM_FEEDER
#define SPECTRUM_FEEDER

#include <libsigrokcxx/libsigrokcxx.hpp>

class wxEvtHandler;

class SpectrumFeeder
{
public:
    SpectrumFeeder(wxEvtHandler *handler, int id);
    ~SpectrumFeeder() = default;

    void data_feed_callback(std::shared_ptr<sigrok::Device> device, std::shared_ptr<sigrok::Packet> packet);

protected:
    void clear_trace(size_t ch);

    size_t channel;
    unsigned int sweep_points;
    double *values[4];

    wxEvtHandler *evtHandler_;
    int id_;
};

#endif

