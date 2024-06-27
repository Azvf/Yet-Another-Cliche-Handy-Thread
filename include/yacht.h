#pragma once

#include "ustl.h"

#include <utility>
#include <type_traits>
#include <cassert>
#include <thread>
#include <future>
#include <memory>
#include <atomic>
#include <mutex>
#include <chrono>
#include <tuple>
#include <unordered_map>
#include <cstdlib>

// 线程时间块
constexpr int32_t kThreadTimeSlice = 100; // ms

// 仿C++20 requrie功能
#define REQUIRES(x) std::enable_if_t<(x), int> = 0

template<class T> T deep_copy(const T& t);
template<class T> T* deep_copy(T* tp);
template<class... Ts> std::tuple<Ts...> deep_copy(const std::tuple<Ts...>&);

// FIXME: ambiguous
// template<class T>
// const T& deep_copy(const T& t) { return t; }

template<class T>
T deep_copy(const T& t) { return t; }

// 深拷贝
//template<class T>
//T* deep_copy(T* tp) { return new T(deep_copy(*tp)); }

// 浅拷贝
template<class T>
T* deep_copy(T* tp) { return tp; }

template<class... Ts, size_t... Is>
std::tuple<Ts...> deep_copy_impl(const std::tuple<Ts...>& t, std::index_sequence<Is...>) {
	return std::tuple<Ts...>{deep_copy(std::get<Is>(t))... };
}

template<class... Ts>
std::tuple<Ts...> deep_copy(const std::tuple<Ts...>& t) {
	return deep_copy_impl(t, std::index_sequence_for<Ts...>());
}

#define FUNC() [&](auto... Ts) { return x(Ts...);}

/**
* @brief: 定时器时间计算模式
*/
enum TimerMode {
	/**
	* Timer_Excluded: 不将定时器任务时间计入interval
	* e.g. |时间片||任务||时间片||任务|
	*/
	Timer_Excluded = 0,
	/**
	* Timer_Included: 将定时器任务时间计入interval
	* e.g.
	* 时间片充裕: [|任务| |剩余时间片|] [|任务| |剩余时间片|]
	* 时间片不足: |任务||任务||任务|
	*/
	Timer_Included = 1,
};

/**
* @todo
*/
enum TaskMode {
	Run_Once_Task	= 0,
	Timer_Task		= 1,
};

enum TaskStatus {
	None_Task		= 1,
	Task_Assigned	= 1 << 1,
	Task_Running	= 1 << 2,
};

/**
* @brief: HandyThread配置项
*/
enum ConfigType {
	None_Config = 0,
	/**
	* @brief: 线程延迟启动时间
	*/
	Delay_Config,
	/**
	* @brief: 定时器任务时间片间隔
	*/
	Timer_Interval_Config,
	/**
	* @brief: 定时器任务时间片间隔计算模式
	*/
	Timer_Mode_Config,
	/**
	* @brief: 任务立即启动/延迟启动(延迟启动时delay延迟依然生效)
	*/
	Deferred_Config,
	/**
	* @brief: detach任务
	*/
	Detached_Config,
	/**
	* @brief: detach callback任务
	*/
	Cb_Detached_Config,
	/**
	* @brief: count
	*/
	Base_Config_Count,
};

/**
* @brief: 线程安全对象, 已兼容普通函数，类成员函数，静态类成员函数及对应模板版本调用.
* TODO: 使用fsm做封装, 负责线程状态切换的调度, 将状态同步交由fsm保证
* TODO: 线程池支持
* TODO: 完善接口错误号
* TODO: 对placerholder支持
*/
class HandyThread;
using HandyThreadPtr			= std::unique_ptr<HandyThread>;
#define MakeHandyThreadPtr(x)	std::make_unique<HandyThread>(x);

class HandyThread {
	using FuturePtr = std::unique_ptr<std::future<void>>;
	using FuncPtr = std::unique_ptr<std::function<void()>>;

public:
	HandyThread() {}

	virtual ~HandyThread() {
		// 线程中调用SendMessage会导致join死锁?
		Stop();
	}

public:
	// /**
	// * @brief: 设置线程配置
	// * @warning: deprecated
	// */
	// void SetThreadConfig(const ThreadConfig& config) {
	// 	std::lock_guard<std::mutex> lk(m_data_mut);
	// 	m_config = config;
	// }

	// /**
	// * @brief: 获取线程配置
	// * @depreacted
	// */
	// ThreadConfig ThreadConfigInfo() {
	// 	std::lock_guard<std::mutex> lk(m_data_mut);
	// 	return m_config;
	// }

	/**
	* @brief: 设置线程配置接口2
	*/
	decltype(auto) SetConfig(ConfigType config_type, const std::string& config_value) {
		std::lock_guard<std::mutex> lk(m_data_mut);
		m_confmap[config_type] = config_value;
		ConfigParse();
		return this;
	}

	/**
	* @brief: 设置回调
	*/
	template<
		class		_Fn,
		class...	_Args
	>
	decltype(auto) SetCallbackCtx(_Fn&& _Fx, _Args&&... _Ax) {
		std::lock_guard<std::mutex> lk(m_data_mut);
		m_cb = std::make_unique<HandyThread>();
		m_cb->SetConfig(Deferred_Config, "1")
			->SetConfig(Detached_Config, m_config.cb_detached ? "1" : "0")
			->Run(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
		return this;
	}

	// template <
	// 	class		_Fn,
	// 	class...	_Args
	// >
	// static auto PlaceholderFuncWrapper(_Fn&& _Fx, _Args&&... _Ax) {
	// 	return
	// 		[this, func = std::bind(_Fx, std::forward<_Args>(_Ax)...)]() mutable -> void {
	// 			TypicalRunOnceTask(
	// 				std::make_unique<std::tuple<std::decay_t<_Fn>, std::decay_t<_Args>...>>(
	// 					std::tuple_cat(std::make_tuple(_Fx), _Ax)
	// 				)
	// 			);
	// 			return;
	// 		};
	// }

	/**
	 * @brief: (原神)启动
	 */
	template <
		class		_Fn,
		class...	_Args
	>
	void Run(TaskMode mode, _Fn&& _Fx, _Args&&... _Ax) {
		switch (mode) {
		case Run_Once_Task:
			std::invoke(&HandyThread::RunOnce<_Fn, _Args...>, this, std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
			break;
		case Timer_Task:
			std::invoke(&HandyThread::RunTimerTask<_Fn, _Args...>, this, std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...);
			break;
		default:
			assert(0);
			break;
		}
	}

	/**
	* @brief: 停止线程
	*/
	void Stop() {
		std::unique_lock<std::mutex> lk(m_state_mut);
		if (m_task) {
			if (!m_config.detached && m_fut && m_fut->valid()) {
				m_stop = true;
				m_fut->wait();
				m_fut = ustl::nullopt;
				m_stop = false;
			}
		}
	}

	/**
	* @brief: 等待线程
	*/
	void Wait(int32_t time = -1) {
		std::unique_lock<std::mutex> lk(m_state_mut);
		InnerWait(time);
	}

	/**
	* @brief: 获取线程执行任务
	*/
	const auto GetTask() {
		Wait(-1);
		return m_task;
	}

	/**
	* @brief: 启动deferred任务或再执行原任务
	*/
	template<class... _Placeholders>
	void Launch(_Placeholders&&... _Phs) {
		std::lock_guard<std::mutex> lk(m_state_mut);
		InnerLaunch(std::forward<_Placeholders>(_Phs)...);
	}

	/**
	* @brief: 重置HandyThread
	* @todo
	*/
	void Reset() {
		assert(0);
	}

	/**
	* @brief: 获取状态
	*/
	TaskStatus Status() {
		std::lock_guard<std::mutex> lk(m_state_mut);
		std::lock_guard<std::mutex> lk2(m_data_mut);
		if (!m_task) {
			return None_Task;
		}
		else {
			if (InnerWait(0)) {
				return Task_Assigned;
			}
			else {
				return Task_Running;
			}
		}

		assert(0);
		return None_Task;
	}

private:
	template <
		class		_Fn,
		class...	_Args
	>
	void RunOnce(_Fn&& _Fx, _Args&&... _Ax) {
		std::lock_guard<std::mutex> lk(m_state_mut);
		ConfigParse();

		if (m_fut) {
			assert(0 && "thread still runing!");
			return;
		}

		m_stop = false;
		m_task = [this, _Fx = std::forward<_Fn>(_Fx), _Ax = std::make_tuple(std::forward<_Args>(_Ax)...)]() mutable -> void {
			TypicalRunOnceTask(
				std::make_unique<std::tuple<std::decay_t<_Fn>, std::decay_t<_Args>...>>(
					std::tuple_cat(std::make_tuple(_Fx), _Ax)
				)
			);
			};

		std::lock_guard<std::mutex> lk2(m_data_mut);
		if (!m_fut && !m_config.deferred) {
			InnerLaunch();
		}
	}

#if _HAS_CXX17  
	template <
		class		_Fn,
		class...	_Args
	>
	void RunTimerTask(_Fn&& _Fx, _Args&&... _Ax) {
		std::lock_guard<std::mutex> lk(m_state_mut);
		ConfigParse();

		if (m_fut) {
			assert(0 && "thread still runing!");
			return;
		}

		m_stop = false;

		if constexpr (std::is_convertible_v<bool, std::invoke_result_t<_Fn, _Args...>>) {
			m_task = [this, _Fx = std::forward<_Fn>(_Fx), _Ax = std::make_tuple(std::forward<_Args>(_Ax)...)]() mutable -> void {
					TypicalTimerTask(
						std::make_unique<std::tuple<std::decay_t<_Fn>, std::decay_t<_Args>...>>(
							std::tuple_cat(std::make_tuple(_Fx), _Ax)
						)
					);
				};
		}
		else if constexpr (std::is_same_v<void, std::invoke_result_t<_Fn, _Args...>>) {
			auto fn_with_default_ret = [fx = std::move(_Fx)](_Args&&... args) {
				std::invoke(fx, std::forward<_Args>(args)...);
				return true;
				};

			using default_fn_type = decltype(fn_with_default_ret);

			m_task = [this, _Fx = std::forward<default_fn_type>(fn_with_default_ret), _Ax = std::make_tuple(std::forward<_Args>(_Ax)...)]() mutable -> void {
						TypicalTimerTask(
							std::make_unique<std::tuple<std::decay_t<default_fn_type >, std::decay_t<_Args>...>>(
								std::tuple_cat(std::make_tuple(_Fx), _Ax)
							)
						);
					};
		}
		else {
			static_assert(0 && "unknown return value");
		}

		std::lock_guard<std::mutex> lk2(m_data_mut);
		if (!m_fut && !m_config.deferred) {
			InnerLaunch();
		}
	}
#else
	template <
		class		_Fn,
		class...	_Args,
		std::enable_if_t<std::is_convertible<decltype(std::invoke(std::declval<_Fn>(), std::declval<_Args>()...)), bool>::value, int> = 0
	>
	void RunTimerTask(_Fn&& _Fx, _Args&&... _Ax) {
		std::lock_guard<std::mutex> lk(m_state_mut);
		ConfigParse();

		if (m_fut) {
			assert(0 && "thread still runing!");
			return;
		}

		m_stop = false;
		m_task = [this, _Fx = std::forward<_Fn>(_Fx), _Ax = std::make_tuple(std::forward<_Args>(_Ax)...)]() mutable -> void {
					TypicalTimerTask(
						std::make_unique<std::tuple<std::decay_t<_Fn>, std::decay_t<_Args>...>>(
							std::tuple_cat(std::make_tuple(_Fx), _Ax)
						)
					);
				};

		std::lock_guard<std::mutex> lk2(m_data_mut);
		if (!m_fut && !m_config.deferred) {
			InnerLaunch();
		}
	}

	template <
		class		_Fn,
		class...	_Args,
		std::enable_if_t<std::is_void<decltype(std::invoke(std::declval<_Fn>(), std::declval<_Args>()...))>::value, int> = 0
	>
	void RunTimerTask(_Fn&& _Fx, _Args&&... _Ax) {
		std::lock_guard<std::mutex> lk(m_state_mut);
		ConfigParse();

		if (m_fut) {
			assert(0 && "thread still runing!");
			return;
		}

		m_stop = false;

		auto fn_with_default_ret = [fx = std::move(_Fx)](_Args&&... args) {
			std::invoke(fx, std::forward<_Args>(args)...);
			return true;
			};

		using default_fn_type = decltype(fn_with_default_ret);

		m_task = [this, _Fx = std::forward<default_fn_type>(fn_with_default_ret), _Ax = std::make_tuple(std::forward<_Args>(_Ax)...)]() mutable -> void {
					TypicalTimerTask(
						std::make_unique<std::tuple<std::decay_t<default_fn_type >, std::decay_t<_Args>...>>(
							std::tuple_cat(std::make_tuple(_Fx), _Ax)
						)
					);
				};

		std::lock_guard<std::mutex> lk2(m_data_mut);
		if (!m_fut && !m_config.deferred) {
			InnerLaunch();
		}
	}
#endif

	

private:
#pragma region 模板任务内部实现
	/**
	* @brief: 一次性线程任务模块化实现
	*/
	template <
		class _Target
	>
	void TypicalRunOnceTask(_Target&& _Tar) {
		if (!DelayCheck()) {
			return;
		}

		auto args = deep_copy(*_Tar);
		InnerRun(std::forward<_Target>(std::make_unique<decltype(args)>(args)));

		std::lock_guard<std::mutex> lk(m_data_mut);
		if (m_cb) {
			m_cb->InnerLaunch();
		}
	}

	/**
	* @brief: 定时器任务模块化实现
	*/
	template <
		class _Target
	>
	void TypicalTimerTask(_Target&& _Tar) {
		if (!DelayCheck()) {
			return;
		}

		bool bStop = false;
		auto getCurTime = [] { return std::chrono::high_resolution_clock::now(); };
		
		while (1) {
			std::chrono::steady_clock::time_point startTime;

			{
				std::lock_guard<std::mutex> lk(m_data_mut);
				if (m_config.timer_mode == Timer_Included) {
					startTime = getCurTime();
				}
			}

			auto args = deep_copy(*_Tar);
			if (!InnerRun(std::forward<_Target>(std::make_unique<decltype(args)>(args)))) {
				break;
			}

			int32_t interval;

			{
				std::lock_guard<std::mutex> lk(m_data_mut);
				if (m_config.timer_mode == Timer_Excluded) {
					startTime = getCurTime();
				}

				interval = m_config.timer_interval;
			}

			while (std::chrono::duration_cast<std::chrono::milliseconds>(getCurTime() - startTime).count() < interval) {
				// 检测更新线程是否关闭
				if (m_stop) {
					bStop = true;
					break;
				}

				auto check_time_slice = std::max(interval / 10, 1);
				std::this_thread::sleep_for(std::chrono::milliseconds(check_time_slice));
			}

			if (bStop || m_stop) {
				break;
			}
		}

		std::lock_guard<std::mutex> lk(m_data_mut);
		if (m_cb) {
			m_cb->InnerLaunch();
		}
	}

	/**
	* @brief: 限时任务模块化实现
	* @todo: 有需求可优化轮询时间粒度
	*/
	template <
		class _Target
	>
	void TypicalTaskWithTimeout(int32_t time_limit, _Target&& _Tar) {
		namespace sc = std::chrono;

		if (!DelayCheck()) {
			return;
		}

		auto real_task = [this, args = deep_copy(*_Tar)] {
				InnerRun(std::forward<_Target>(std::make_unique<decltype(args)>(args)));
			};

		auto real_fut = std::async(std::launch::async, real_task);

		auto cur_time = [] { return std::chrono::high_resolution_clock::now(); };
		auto start_time = cur_time();
		while (1) {
			auto elapsed_time = sc::duration_cast<sc::milliseconds>(start_time - cur_time()).count();
			if (elapsed_time > time_limit) {
				break;
			}

			if (m_stop) {
				break;
			}

			// @todo: 提供用户接口做额外的条件判定和状态处理?

			if (real_fut.wait_for(100ms) == std::future_status::ready) {
				break;
			}
		}

		std::lock_guard<std::mutex> lk(m_data_mut);
		if (m_cb) {
			m_cb->InnerLaunch();
		}
	}

#pragma endregion	// region 模板任务内部实现

protected:
	/**
	* @brief: 可通过继承后重载此函数实现线程池调用
	* @experimental
	*/
	virtual ustl::optional<std::future<void>> LaunchAsync() {
		return
			ustl::make_optional<std::future<void>>(
				std::async(std::launch::async, m_task.value())
			);
	}

protected:
	/**
	* @brief: 解析配置参数
	* @todo: 优化解析过程
	*/
	void ConfigParse() {
		auto has_config =
			[] (std::unordered_map<ConfigType, std::string>& m, ConfigType t) -> bool {
				return (m.find(t) != m.cend());
			};

		if (has_config(m_confmap, Delay_Config)) {
			m_config.delay = std::atoi(m_confmap[Delay_Config].data());
		}
		if (has_config(m_confmap, Timer_Interval_Config)) {
			m_config.timer_interval = std::atoi(m_confmap[Timer_Interval_Config].data());
		}
		if (has_config(m_confmap, Timer_Mode_Config)) {
			m_config.timer_mode = (TimerMode)std::atoi(m_confmap[Timer_Mode_Config].data());
		}
		if (has_config(m_confmap, Deferred_Config)) {
			m_config.deferred = std::atoi(m_confmap[Deferred_Config].data());
		}
		if (has_config(m_confmap, Detached_Config)) {
			m_config.detached = std::atoi(m_confmap[Detached_Config].data());
		}

		ExtendConfigParse();
	}

	/**
	* @brief: 用于子类扩展配置
	*/
	virtual void ExtendConfigParse() {}

protected:
#pragma region inner函数无锁实现
	//template<class... _Placeholders>
	//void InnerLaunch(_Placeholders&&... _Phs) {
	//	if (m_task) {
	//		if (!m_fut) {
	//			m_fut = 
	//				std::make_unique<std::future<void>>(
	//					std::async(
	//						std::launch::async, 
	//						&HandyThread::InnerLaunchPhHelper<_Placeholders...>,
	//						m_task,
	//						std::forward<_Placeholders>(_Phs)...
	//					)
	//				);
	//		}
	//		else {
	//			assert(0);
	//			// TODO
	//			// HandyThread::InnerLaunchPhHelper(m_task, std::forward<_Placeholders>(_Phs)...);
	//		}
	//	}
	//}

	void InnerLaunch() {
		if (!m_task) {
			assert(0);
			return;
		}

		// FIXME: 解决线程池问题
		if (!m_fut) {
			m_fut = LaunchAsync();
		}
		else {
			assert(0);
		}
	}

	bool InnerWait(int32_t time = -1) {
		bool ready = false;
		if (m_task) {
			if (m_fut && m_fut->valid()) {
				if (time < 0) {
					m_fut->wait();
					ready = true;
				}
				else {
					if (m_fut->wait_for(std::chrono::milliseconds(time)) == std::future_status::ready) {
						ready = true;
					}

				}
			}

			if (ready) {
				m_fut = ustl::nullopt;
			}
		}
		return ready;
	}
#pragma endregion	// region inner函数无锁实现

protected:
#pragma region util
	/**
	* @brief: 任务启动延时
	* @todo: 优化启动时间片粒度，目前是固定100ms
	*/
	bool DelayCheck() {
		std::unique_lock<std::mutex> lk(m_data_mut);
		if (m_config.delay) {
			int check_count = m_config.delay / kThreadTimeSlice + 1;
			lk.unlock();
			while (check_count) {
				std::this_thread::sleep_for(std::chrono::milliseconds(kThreadTimeSlice));

				if (m_stop) {
					return false;
				}

				check_count--;
			}
		}
		return true;
	}
#pragma endregion	// region util

protected:
#pragma region 模板util
	/**
	* @brief: placeholder函数调用helper
	*/
	template<class... _Placeholders>
	static void InnerLaunchPhHelper(std::shared_ptr<std::function<void()>> task, _Placeholders&&... _Phs) {
		std::invoke(*task, std::forward<_Placeholders>(_Phs)...);
	}

	/**
	* @brief: 将变长参数最终解包后调用
	*/
	template<class _Target, size_t... _Idxs>
	static auto InnerRun2(typename _Target::element_type& _Tup,
		std::index_sequence<_Idxs...>) {
		return std::invoke(std::move(std::get<_Idxs>(_Tup))...);
	}

	/**
	* @brief: 将函数体与调用参数解包分离
	*/
	template<class _Target>
	static auto InnerRun(_Target&& _Tar) {
		_Target local(std::forward<_Target>(_Tar));
		return InnerRun2<_Target>(*local, std::make_index_sequence<std::tuple_size_v<typename _Target::element_type>>());
	}
#pragma endregion	// region 模板util

protected:
	/**
	* @brief: 线程配置
	*/
	struct ThreadConfig {
		int32_t delay				= 0;				// ms
		int32_t timer_interval		= kThreadTimeSlice; // ms
		enum TimerMode timer_mode	= Timer_Included;
		bool deferred				= false;
		bool detached				= false;
		bool cb_detached			= false;
	};

protected:
	std::mutex m_data_mut;
	std::mutex m_state_mut;
	ustl::optional<std::function<void()>> m_task;
	ustl::optional<std::future<void>> m_fut;
	std::atomic<bool> m_stop = false;

	ThreadConfig m_config;
	std::unordered_map<ConfigType, std::string> m_confmap;
	std::unique_ptr<HandyThread> m_cb;
};
