#pragma once

#include "AnimationIncremental.h"

class BoardMapShowStrategy
{
public:
	virtual void setUpperLimit(int count) = 0;
	virtual void Update() = 0;
	virtual int getCurrentValue() const = 0;
	virtual void setCurrentValue(int currentValue) = 0;
};

class SlideShowStrategy : public BoardMapShowStrategy, public AnimationIncremental<int>
{
	void setUpperLimit(int count)
	{
		AnimationIncremental<int>::setUpperLimit(count);
	}

	void Update() {
		AnimationIncremental<int>::Update();
	}

	int getCurrentValue() const
	{
		return AnimationIncremental<int>::getCurrentValue();
	}

	void setCurrentValue(int currentValue) {
		AnimationIncremental<int>::setCurrentValue(currentValue);
	}
};

class ShowLastStrategy : public BoardMapShowStrategy
{
	void setUpperLimit(int count)
	{
		_currentValue = count;
	}

	void Update() {	}

	int getCurrentValue() const
	{
		return _currentValue;
	}

	void setCurrentValue(int currentValue) {}

private:
	int _currentValue = 0;
};
