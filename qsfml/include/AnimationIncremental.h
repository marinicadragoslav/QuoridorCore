#pragma once

#include <SFML/System/Clock.hpp>

template<typename T>
class AnimationIncremental {
public:
	AnimationIncremental() :
		_isRunning(false),
		_interval(AnimationIncremental::TIMELINE_ANIMATION_FRAME_DURATION),
		_incrementValue(1),
		_lowerLimit(0),
		_upperLimit(0),
		_loop(false)
	{
		_currentValue = _lowerLimit;
	}

	void Update()
	{
		if (_isRunning and _clock.getElapsedTime().asSeconds() > _interval)
		{
			_currentValue = (_currentValue + _incrementValue) % (_upperLimit + 1);

			if (not _loop and _currentValue + _incrementValue >= _upperLimit + 1)
				pause();

			_clock.restart();
		}
	}

	T getCurrentValue() const { return _currentValue; }
	void setCurrentValue(T currentValue) { _currentValue = currentValue; }
	void setStop(bool stop) { _isRunning = !stop; }
	inline void pause() { setStop(true); }
	inline void resume() { setStop(false); }
	inline void stop() { setStop(true); }
	inline void start() { resume(); }

	void setUpperLimit(T upperLimit) { _upperLimit = upperLimit; }
	T getUpperLimit() const { return _upperLimit; }

	void setIncrement(T incrementValue) { _incrementValue = incrementValue; }

	void setAnimationInterval(float timeInSeconds) {
		if (timeInSeconds < 0.0f)
			return;

		spdlog::info("{} set to {}", __func__, timeInSeconds);

		_interval = timeInSeconds;
	}
	float getAnimationInterval() const { return _interval; }

	void loop(bool active) { _loop = active; }

private:
	sf::Clock _clock;
	bool _isRunning;
	bool _loop;
	T _currentValue;
	float _interval;

	T _incrementValue;
	T _lowerLimit;
	T _upperLimit;


	inline static const float TIMELINE_ANIMATION_FRAME_DURATION = 1.0f;
};
