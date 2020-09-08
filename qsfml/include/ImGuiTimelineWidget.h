#pragma once

#include <functional>

class ImGuiTimelineWidget
{
public:

	ImGuiTimelineWidget();
	void ImGuiDrawAndUpdate();

	void setCurrentTime(int currentTime) { _currentTime = currentTime; }
	int getCurrentTime() const { return _currentTime; }
	void setMaxTime(int maxTime) { _maxTime = maxTime; }

	enum class TIMELINE_WIDGET_ID
	{
		LOOP,
		LAST,
		PLAY,
		STOP,
		SPEED_UP,
		SPEED_DOWN,
		RESUME,
	};
	void setCallback(std::function<void(TIMELINE_WIDGET_ID, bool)>& callback) { _callback = callback; }

private:

	void ImGuiDrawContent();
	void setLoop(bool loop);
	void setLast(bool last);

	int _currentTime;
	int _maxTime;
	bool _play;
	bool _stop;
	bool _loop;
	bool _last;

	bool _lastAndLoopWarning;

	std::function<void(TIMELINE_WIDGET_ID, bool)> _callback;
};
