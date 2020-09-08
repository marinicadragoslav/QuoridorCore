#pragma once

#include <mutex>
#include <vector>

#include "BoardState.h"
#include "Game.h"

template<typename T>
class vector_thread_safe
{
public:

	void push(T entry)
	{
		const std::lock_guard<std::mutex> lock(_mtx);
		_timelineHistory.push_back(entry);
	}

	T operator[](size_t i)
	{
		const std::lock_guard<std::mutex> lock(_mtx);

		if (_timelineHistory.size() < i)
			throw "Cannot get from invalid index";

		return _timelineHistory[i];
	}

	size_t count() const
	{
		return _timelineHistory.size();
	}

	void dropFrom(size_t index)
	{
		_timelineHistory.erase(_timelineHistory.begin() + index + 1, _timelineHistory.end());
	}

private:
	std::vector<T> _timelineHistory;
	std::mutex _mtx;
};

struct TimelineEntry {
	qcore::Game game;

	TimelineEntry() = delete;

	TimelineEntry(const qcore::Game& g) : game(g) {};

	TimelineEntry(const TimelineEntry& from) = default;// : map(from.map), game(from.game) {};

	TimelineEntry(TimelineEntry&& from) = default;// : map(from.map), game(from.game)	{};

	TimelineEntry& operator =(const TimelineEntry&) = default;

	operator const qcore::BoardMap () { qcore::BoardMap map; game.getBoardState()->createBoardMap(map, 0); return map; }
};

class TimelineRepo : public vector_thread_safe<TimelineEntry> {};
