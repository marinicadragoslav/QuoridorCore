#ifndef CONSOLEWIDGET_H
#define CONSOLEWIDGET_H

#include <map>
#include <functional>
#include <iostream>
#include <cstring>

#include <imgui.h>
#include <imgui-SFML.h>

#include "ConsoleApp.h"

namespace detail
{
    struct CaseInsensitiveComparator
    {
        const int strcicmp(const char * a, const char * b) const
        {
            for (;; a++, b++) {
                int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
                if (d != 0 || !*a)
                    return d;
            }
        }

        bool operator()(const std::string& a, const std::string& b) const noexcept
        {
            return strcicmp(a.c_str(), b.c_str()) < 0;
        }
    };
}   // namespace detail


template <typename T>
using CaseInsensitiveMap = std::map<std::string, T, detail::CaseInsensitiveComparator>;


class ImGUIConsoleWidget
{
    public:
    ImGUIConsoleWidget();
    ~ImGUIConsoleWidget();

    void    Draw(const char* title, bool* p_open);
    qcli::ConsoleApp::CliCommand& AddCommand(qcli::ConsoleApp::CommandCb exec, const std::string& syntax, const std::string& section);
    void    AddLog(const char* fmt, ...);
    void    ExecCommand(const char* command_line);

private:
    char                  InputBuf[256];
    std::vector<std::string>    Items;

    std::vector<std::string>  History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    std::ostringstream  consoleOut;
    qcli::ConsoleApp console;

private:
    void    ClearLog();
    int     TextEditCallback(ImGuiInputTextCallbackData* data);
};


#endif