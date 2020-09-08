#include <imgui.h>
#include <imgui-SFML.h>

#include <spdlog/spdlog.h>

#include "ImGuiTimelineWidget.h"

ImGuiTimelineWidget::ImGuiTimelineWidget() :
    _currentTime(0),
    _maxTime(0),
    _play(false),
    _stop(false),
    _loop(false),
    _last(true),
    _lastAndLoopWarning(false)
{};

void ImGuiTimelineWidget::ImGuiDrawAndUpdate()
{
    ImGui::Begin("Timeline");

    if (_maxTime > 0)
    {
        ImGuiDrawContent();

        if (_lastAndLoopWarning)
        {
            ImGui::OpenPopup("Modal window");
            if (ImGui::BeginPopupModal("Modal window"))
            {
                ImGui::Text("Last and Loop cannot be both active");
                if (ImGui::Button("Close")) {
                    ImGui::CloseCurrentPopup();
                    _lastAndLoopWarning = false;
                }
                ImGui::EndPopup();
            }
        }
    }
    else
    {
        ImGui::Text("Timeline has no data. Please start a game");
    }

    ImGui::End();
}

void ImGuiTimelineWidget::ImGuiDrawContent()
{
    ImGui::SliderInt("Timeline", &_currentTime, 0, _maxTime);

    if (ImGui::Checkbox("Loop", &_loop))
        setLoop(_loop);

    ImGui::SameLine();
    if (ImGui::Checkbox("Last", &_last))
        setLast(_last);

    ImGui::SameLine();
    if (_play = ImGui::Button("Play"))
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::PLAY, true);

    ImGui::SameLine();
    if (_play = ImGui::Button("Resume"))
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::RESUME, true);

    ImGui::SameLine();
    if (_stop = ImGui::Button("Stop"))
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::STOP, true);

    ImGui::SameLine();
    if (ImGui::Button("Speed--"))
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::SPEED_DOWN, true);

    ImGui::SameLine();
    if (ImGui::Button("Speed++"))
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::SPEED_UP, true);

    ImGui::SameLine();
    ImGui::Text("%d/%d steps", _currentTime, _maxTime);
}

void ImGuiTimelineWidget::setLoop(bool loop)
{
    if (not _last)
    {
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::LOOP, loop);
    }
    else
    {
        _lastAndLoopWarning = true;
        _loop = false;
    }
}

void ImGuiTimelineWidget::setLast(bool last)
{
    if (not _loop)
    {
        if (_callback)
            _callback(TIMELINE_WIDGET_ID::LAST, last);
    } 
    else
    {
        _lastAndLoopWarning = true;
        _last = false;
    }
}

