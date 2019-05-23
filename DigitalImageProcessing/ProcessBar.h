#pragma once
#include <string>
#include <queue>
#include <thread>
#include <mutex>

namespace ImageUtil {

	class ProgressBar
	{
		uint64_t max;
		uint64_t curProgress = 0;
		uint64_t currentValue = 0;

		std::queue<int> upgradeQueue;
		bool stopThread = false;

		static void* progressThread(void *__this);
		std::thread * printThread{};
		std::mutex mtx;

		void updateValue(int value);
	public:
		ProgressBar(int max, const std::string& label);
		explicit ProgressBar(int max);
		~ProgressBar();

		ProgressBar& operator++();
		ProgressBar& operator+=(int value);


		static void show();
		void shutDown();
		void addMax(uint64_t value);

		void reset(int max, const std::string& label);
	};


	static ProgressBar progressBar(0);
}