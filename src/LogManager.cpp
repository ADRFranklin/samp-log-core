#include <fstream>

#ifdef WIN32
#  include <windows.h>
#else
#  include <sys/stat.h>
#endif

#include "LogManager.hpp"
#include "LogConfig.hpp"
#include "crashhandler.hpp"
#include "utils.hpp"

#include <memory>

using samplog::samplog_LogLevel;


LogManager::LogManager() :
	_threadRunning(true),
	_internalLogger("log-core")
{
	crashhandler::Install();

	_thread = std::thread(std::bind(&LogManager::Process, this));
}

LogManager::~LogManager()
{
	{
		std::lock_guard<std::mutex> lg(_queueMtx);
		_threadRunning = false;
	}
	_queueNotifier.notify_one();
	if (_thread.joinable()) {
		_thread.join();
	}
}

void LogManager::Queue(Action_t &&action)
{
	{
		std::lock_guard<std::mutex> lg(_queueMtx);
		_queue.push(std::move(action));
	}
	_queueNotifier.notify_one();
}

void LogManager::WriteLevelLogString(std::string const &time, samplog_LogLevel level,
	std::string const &module_name, std::string const &message)
{
	const char *filename = nullptr;
	size_t idx = 0;
	switch (level) {
	case samplog_LogLevel::WARNING:
		filename = "warnings.log";
		idx = 0;
		break;
	case samplog_LogLevel::ERROR:
		filename = "errors.log";
		idx = 1;
		break;
	case samplog_LogLevel::FATAL:
		filename = "fatals.log";
		idx = 2;
		break;
	default:
		return;
	}

	auto globalConfig = LogConfig::Get()->GetGlobalConfig();
	auto file_path = globalConfig.LogsRootFolder + filename;
	auto &state = _levelFiles[idx];

	if (state.path != file_path) {
		if (state.stream.is_open()) {
			state.stream.close();
		}
		state.path = file_path;
		state.dirEnsured = false;
	}

	if (!state.dirEnsured) {
		utils::EnsureFolders(file_path);
		state.dirEnsured = true;
	}
	if (!state.stream.is_open()) {
		state.stream.open(file_path, std::ofstream::out | std::ofstream::app);
		if (!state.stream) {
			return;
		}
	}

	state.stream <<
		"[" << time << "] " <<
		"[" << module_name << "] " <<
		message << '\n' << std::flush;
}

void LogManager::Process()
{
	std::unique_lock<std::mutex> lk(_queueMtx);

	while (true) {
		_queueNotifier.wait(lk, [this] {
			return !_threadRunning || !_queue.empty();
		});

		if (!_threadRunning && _queue.empty()) {
			break;
		}

		while (!_queue.empty())
		{
			auto action = std::move(_queue.front());
			_queue.pop();

			//manually unlock mutex
			//the whole write-to-file code below has no need to be locked with the
			//message queue mutex; while writing to the log file, new messages can
			//now be queued
			lk.unlock();
			action();
			lk.lock();
		}
	}
}
