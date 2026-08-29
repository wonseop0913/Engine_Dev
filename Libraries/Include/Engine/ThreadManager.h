#pragma once

class BULB_API ThreadManager
{
	friend class BulbApplication;

private:
	ThreadManager() = default;
	~ThreadManager();

	void Init();
	
public:
	ThreadManager(const ThreadManager& rhs) = delete;
	ThreadManager& operator=(const ThreadManager& rhs) = delete;

	static ThreadManager* GetInstance();
	static Bulb::ProcessResult Delete();

	template<class F, class... Args>
	auto EnqueueJob(F&& f, Args&&... args) -> future<typename invoke_result<F, Args...>::type>;

	const vector<bool>& GetThreadActivations() { return _threadActivations; }

private:
	static ThreadManager* s_instance;

	size_t _numThreads;
	bool _stop = false;

	vector<thread> _threads;
	queue<function<void()>> _jobs;

	mutex _queueMutex;
	condition_variable _condition;
	
	vector<bool> _threadActivations;
};

template<class F, class... Args>
auto ThreadManager::EnqueueJob(F&& f, Args&&... args) -> future<typename invoke_result<F, Args...>::type>
{
	using return_type = typename invoke_result<F, Args...>::type;

	auto task = make_shared<packaged_task<return_type()>>(
		bind(forward<F>(f), forward<Args>(args)...)
	);

	future<return_type> res = task->get_future();
	{
		unique_lock<mutex> lock(_queueMutex);
		if (_stop)
			return future<return_type>(); // Ȥ�� ���� ó��

		_jobs.emplace([task]() { (*task)(); });
	}
	_condition.notify_one();
	return res;
}

