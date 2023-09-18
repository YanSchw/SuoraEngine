#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stack>
#include <thread>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "Suora/Core/Log.h"

namespace Suora 
{

	using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

	struct ProfileResult
	{
		std::string Name;

		FloatingPointMicroseconds Start;
		std::chrono::microseconds ElapsedTime;
		std::thread::id ThreadID;
	};
	struct InstrumentationResult
	{
		ProfileResult Result;
		std::vector<InstrumentationResult> Children;
	};

	struct InstrumentationSession
	{
		std::string Name;
	};

	class InstrumentationTimer;

	class Instrumentor
	{
	public:
		Instrumentor(const Instrumentor&) = delete;
		Instrumentor(Instrumentor&&) = delete;

		void BeginSession(const std::string& name, const std::string& filepath = "results.json")
		{
			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				// If there is already a current session, then close it before beginning new one.
				// Subsequent profiling output meant for the original session will end up in the
				// newly opened session instead.  That's better than having badly formatted
				// profiling output.
				if (Log::GetLogger(LogCategory::None)) // Edge case: BeginSession() might be before Log::Init()
				{
					SUORA_ERROR(LogCategory::Debug, "Instrumentor::BeginSession('{0}') when session '{1}' already open.", name, m_CurrentSession->Name);
				}
				InternalEndSession();
			}
			m_OutputStream.open(filepath);

			if (m_OutputStream.is_open())
			{
				m_CurrentSession = new InstrumentationSession({name});
				WriteHeader();
			}
			else
			{
				if (Log::GetLogger(LogCategory::None)) // Edge case: BeginSession() might be before Log::Init()
				{
					SUORA_ERROR(LogCategory::Debug, "Instrumentor could not open results file '{0}'.", filepath);
				}
			}
		}

		void EndSession()
		{
			std::lock_guard lock(m_Mutex);
			InternalEndSession();
		}

		void BeginProfile(std::thread::id id)
		{
			if (!m_CurrentProfile[id].empty())
			{
				InstrumentationResult* current = m_CurrentProfile[id].top();
				current->Children.push_back(InstrumentationResult());
				InstrumentationResult& next = current->Children[current->Children.size() - 1];
				m_CurrentProfile[id].push(&next);
			}
			else
			{
				m_Results[id].push_back(InstrumentationResult());
				InstrumentationResult& next = m_Results[id][m_Results[id].size() - 1];
				m_CurrentProfile[id].push(&next);
			}
		}

		void WriteProfile(const ProfileResult& result, bool timer)
		{
			std::stringstream json;

			json << std::setprecision(3) << std::fixed;
			json << ",{";
			json << "\"cat\":\"function\",";
			json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
			json << "\"name\":\"" << result.Name << "\",";
			json << "\"ph\":\"X\",";
			json << "\"pid\":0,";
			json << "\"tid\":" << result.ThreadID << ",";
			json << "\"ts\":" << result.Start.count();
			json << "}";

			std::lock_guard lock(m_Mutex);
			if (m_CurrentSession)
			{
				m_OutputStream << json.str();
				m_OutputStream.flush();
			}

			if (timer && !m_CurrentProfile[result.ThreadID].empty())
			{
				m_CurrentProfile[result.ThreadID].top()->Result = result;
				m_CurrentProfile[result.ThreadID].pop();
			}
		}

		void EndFrame()
		{
			m_LastFrameResults = m_Results;
			m_Results.clear();
			m_CurrentProfile.clear();
		}

		/*static Instrumentor& Get()
		{
			static Instrumentor instance;
			return instance;
		}*/
	private:
		Instrumentor()
			: m_CurrentSession(nullptr)
		{
		}

		~Instrumentor()
		{
			EndSession();
		}		

		void WriteHeader()
		{
			m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
			m_OutputStream.flush();
		}

		void WriteFooter()
		{
			m_OutputStream << "]}";
			m_OutputStream.flush();
		}

		// Note: you must already own lock on m_Mutex before
		// calling InternalEndSession()
		void InternalEndSession()
		{
			if (m_CurrentSession)
			{
				WriteFooter();
				m_OutputStream.close();
				delete m_CurrentSession;
				m_CurrentSession = nullptr;
			}
		}
	private:
		std::mutex m_Mutex;
		InstrumentationSession* m_CurrentSession;
		std::ofstream m_OutputStream;
		std::unordered_map<std::thread::id, std::vector<InstrumentationResult>> m_LastFrameResults;
		std::unordered_map<std::thread::id, std::vector<InstrumentationResult>> m_Results;
		std::unordered_map<std::thread::id, std::stack<InstrumentationResult*>> m_CurrentProfile;

		friend class Profiler;
	};

	class InstrumentationTimer
	{
	public:
		InstrumentationTimer(const char* name, int line)
			: m_Name(name), m_Line(line), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::steady_clock::now();
			//Instrumentor::Get().BeginProfile(std::this_thread::get_id());
		}

		~InstrumentationTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::steady_clock::now();
			auto highResStart = FloatingPointMicroseconds{ m_StartTimepoint.time_since_epoch() };
			auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

			//Instrumentor::Get().WriteProfile({ m_Name, highResStart, elapsedTime, std::this_thread::get_id() }, true);

			m_Stopped = true;
		}
	private:
		const char* m_Name;
		int m_Line;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;
	};

	namespace InstrumentorUtils 
	{

		template <size_t N>
		struct ChangeResult
		{
			char Data[N];
		};

		template <size_t N, size_t K>
		constexpr auto CleanupOutputString(const char(&expr)[N], const char(&remove)[K])
		{
			ChangeResult<N> result = {};

			size_t srcIndex = 0;
			size_t dstIndex = 0;
			while (srcIndex < N)
			{
				size_t matchIndex = 0;
				while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
					matchIndex++;
				if (matchIndex == K - 1)
					srcIndex += matchIndex;
				result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
				srcIndex++;
			}
			return result;
		}
	}
}

#define SUORA_PROFILE 0
#define SUORA_PROFILE_TIMERS 1
#if SUORA_PROFILE
	// Resolve which function signature macro will be used. Note that this only
	// is resolved when the (pre)compiler starts, so the syntax highlighting
	// could mark the wrong one in your editor!
	#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
		#define SUORA_FUNC_SIG __PRETTY_FUNCTION__
	#elif defined(__DMC__) && (__DMC__ >= 0x810)
		#define SUORA_FUNC_SIG __PRETTY_FUNCTION__
	#elif (defined(__FUNCSIG__) || (_MSC_VER))
		#define SUORA_FUNC_SIG __FUNCSIG__
	#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
		#define SUORA_FUNC_SIG __FUNCTION__
	#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
		#define SUORA_FUNC_SIG __FUNC__
	#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
		#define SUORA_FUNC_SIG __func__
	#elif defined(__cplusplus) && (__cplusplus >= 201103)
		#define SUORA_FUNC_SIG __func__
	#else
		#define SUORA_FUNC_SIG "SUORA_FUNC_SIG unknown!"
	#endif

	#define SUORA_PROFILE_BEGIN_SESSION(name, filepath) ::Suora::Instrumentor::Get().BeginSession(name, filepath)
	#define SUORA_PROFILE_END_SESSION() ::Suora::Instrumentor::Get().EndSession()
	#define SUORA_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Suora::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Suora::InstrumentationTimer timer##line(fixedName##line.Data, line)
	#define SUORA_PROFILE_SCOPE_LINE(name, line) SUORA_PROFILE_SCOPE_LINE2(name, line)
	#define SUORA_PROFILE_SCOPE(name) SUORA_PROFILE_SCOPE_LINE(name, __LINE__)
	#define SUORA_PROFILE_FUNCTION() SUORA_PROFILE_SCOPE(SUORA_FUNC_SIG)
#elif SUORA_PROFILE_TIMERS
	#define SUORA_PROFILE_BEGIN_SESSION(name, filepath)
	#define SUORA_PROFILE_END_SESSION()
	#define SUORA_PROFILE_SCOPE_LINE2(name, line) constexpr auto fixedName##line = ::Suora::InstrumentorUtils::CleanupOutputString(name, "__cdecl ");\
											   ::Suora::InstrumentationTimer timer##line(fixedName##line.Data, line)
	#define SUORA_PROFILE_SCOPE_LINE(name, line) SUORA_PROFILE_SCOPE_LINE2(name, line)
	#define SUORA_PROFILE_SCOPE(name) SUORA_PROFILE_SCOPE_LINE(name, __LINE__)
	#define SUORA_PROFILE_FUNCTION()
#else
	#define SUORA_PROFILE_BEGIN_SESSION(name, filepath)
	#define SUORA_PROFILE_END_SESSION()
	#define SUORA_PROFILE_SCOPE(name)
	#define SUORA_PROFILE_FUNCTION()
#endif