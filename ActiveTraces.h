#ifndef ACTIVETRACES_H
#define ACTIVETRACES_H

#include <string>
#include <vector>
#include <map>

#include <wx/string.h>

class ActiveTraces
{
    public:
        ActiveTraces();
        virtual ~ActiveTraces();
        std::map<wxString, bool> ui;
};
#endif
