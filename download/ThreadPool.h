#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

using namespace std;

class ThreadBarrier
{
private:
	mutex& entry;
	condition_variable barrier;
	int count;

public:
	ThreadBarrier(mutex& m, int n) : entry(m)
	{
		count = n + 1;
	}

	void checkin()
	{
		unique_lock<mutex> lock(entry);

		if (-- count > 0)
		{
			barrier.wait(lock, [this]
			{
				return count <= 0;
			});
		} 
		else barrier.notify_all();
	}
};

class ThreadPool
{
private:
	bool stopping;
	queue<function<void()>> works;
	vector<thread> workers;
	mutex entry;
	condition_variable condition;
	ThreadBarrier barrier;

public:
	ThreadPool(int n) : stopping(false), barrier(entry, n)
	{
		workers.reserve(n);

		for (int i = 0; i < n; i++)
		{
			works.emplace([this]
			{
				barrier.checkin();
			});

			workers.emplace_back(&ThreadPool::WorkerAction, this);
		}

		barrier.checkin();
	}

	~ThreadPool()
	{
		StopAllThreads();

		condition.notify_all();

		for (thread& worker : workers)
		{
			worker.join();
		}
	}

	void StopAllThreads()
	{
		unique_lock<mutex> lock(entry);

		stopping = true;
		
		for (thread& worker : workers)
		{
			works.emplace([]
			{
				throw string("Terminated!");
			});
		}
	}

	void WorkerAction()
	{
		while (true)
		{
			auto handle = GetNextWork();

			try
			{
				handle();
			}
			catch (string TerminatedMessage)
			{
				break;
			}
		}
	}

	function<void()> GetNextWork()
	{
		unique_lock<mutex> lock(entry);

		condition.wait(lock, [this]
		{
			return stopping || !works.empty();
		});

		function<void()> work = works.front();

		works.pop();

		return work;
	}

	template<class method, class... input>
	void Enqueue(method&& fn, input&&... in)
	{
		unique_lock<mutex> lock(entry);

		if (stopping) throw runtime_error("Error: Stopped ThreadPool!");

		works.emplace(bind(forward<method>(fn), forward<input>(in)...));

		condition.notify_one();
	}
};

#endif
