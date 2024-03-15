#pragma once

#include <coroutine>
#include <iostream>

class Task
{
public:
	struct promise_type;
	using CoroutineHandle = std::coroutine_handle<promise_type>;
	struct promise_type
	{
		Task get_return_object() noexcept
		{
			return Task{ CoroutineHandle::from_promise(*this) };
		}
		std::suspend_always initial_suspend() noexcept { return{}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void unhandled_exception() noexcept { /*std::throw;*/ }
		void return_void() { return; }
	};

	Task(CoroutineHandle&& _handle) : m_pCoHandle(std::make_shared<CoroutineHandle>(_handle)) { }
	Task(Task& _task) = delete;
	Task(Task&& _task) : m_pCoHandle(_task.m_pCoHandle) { _task.m_pCoHandle = nullptr; }
	~Task() { }

	void Run() { if (m_pCoHandle) { m_pCoHandle->resume(); } }
	bool IsFinish() { if (m_pCoHandle) { return m_pCoHandle->done(); } return false; }
private:
	std::shared_ptr<CoroutineHandle> m_pCoHandle;
};

//struct Await
//{
//	constexpr bool await_ready() const noexcept { return false; }
//	void await_suspend(std::coroutine_handle<> _handle) const noexcept
//	{
//		if (_handle.done() == false) {
//		}
//	}
//	constexpr void await_resume() const noexcept { }
//};