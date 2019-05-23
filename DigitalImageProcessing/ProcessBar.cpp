#include "ProcessBar.h"
#include <iostream>
#include <iomanip>


using namespace std;

void* ImageUtil::ProgressBar::progressThread(void * __this)
{
	ProgressBar * _this = static_cast<ProgressBar *>(__this);
	while (!_this->stopThread)
	{
		if (!_this->upgradeQueue.empty())
		{
			int value = 0;
			if (_this->mtx.try_lock())
			{
				value = _this->upgradeQueue.front();
				_this->upgradeQueue.pop();
				_this->mtx.unlock();
			}

			_this->updateValue(value);
			
		}
	}
	return nullptr;
}

ImageUtil::ProgressBar::ProgressBar(const int max) :max(max), printThread(new thread(&progressThread, static_cast<void *>(this)))
{
	//printThread->detach();
}


ImageUtil::ProgressBar::~ProgressBar()
{
	stopThread = true;
	if (printThread->joinable())
		printThread->join();
	delete printThread;
}

ImageUtil::ProgressBar& ImageUtil::ProgressBar::operator++()
{

	mtx.lock();
	upgradeQueue.push(1);
	mtx.unlock();


	return *this;
}

ImageUtil::ProgressBar& ImageUtil::ProgressBar::operator+=(int value)
{
	mtx.lock();

	upgradeQueue.push(value);
	mtx.unlock();


	return *this;
}

void ImageUtil::ProgressBar::show()
{
	cout << "0%  10%  20%  30%  40%  50%  60%  70%  80%  90%  100%" << endl;
	cout << "|----|----|----|----|----|----|----|----|----|----|" << endl;
}

void ImageUtil::ProgressBar::shutDown()
{
	stopThread = true;
}

void ImageUtil::ProgressBar::addMax(uint64_t value)
{
	mtx.lock();
	max += value;
	mtx.unlock();

}

void ImageUtil::ProgressBar::reset(int max, const std::string& label)
{

	mtx.lock();
	this->max = max;
	cout << endl << label << endl;
	show();

	curProgress = 0;
	currentValue = 0;

	while (!upgradeQueue.empty())
		upgradeQueue.pop();

	mtx.unlock();

}

inline void ImageUtil::ProgressBar::updateValue(const int value)
{
	currentValue += value;
	const uint64_t prog = currentValue * 50 / max;

	if (prog > curProgress)
	{
		cout << "\r" << setfill('*') << setw(prog) << "*";
		curProgress = prog;
		if (curProgress >= 50)
		{
			cout << "*" << endl;
		}
	}
}

ImageUtil::ProgressBar::ProgressBar(int max, const std::string& label) :ProgressBar(max)
{
	cout << label << endl;
	show();
}
