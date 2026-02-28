#pragma once

#include "Singleton.hpp"
#include "samplog/LogLevel.hpp"
#include "FileChangeDetector.hpp"
#include "Logger.hpp"

#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>


using samplog::samplog_LogLevel;


struct samplog_LogLevelConfig
{
	bool PrintToConsole = false;
};

struct GlobalConfig
{
	std::string LogTimeFormat = "%x %X";
	bool DisableDebugInfo = false;
	bool EnableColors = false;
	std::string LogsRootFolder = "logs/";
};

class LogConfig : public Singleton<LogConfig>
{
	friend Singleton<LogConfig>;
public:
	using ConfigUpdateEvent_t = std::function<void(Logger::Config const &)>;

private:
	LogConfig() = default;
	~LogConfig() = default;

private: // variables
	mutable std::mutex _configLock;
	std::unordered_map<std::string, Logger::Config> _loggerConfigs;
	std::unordered_map<std::string, ConfigUpdateEvent_t> _loggerConfigEvents;
	std::map<samplog_LogLevel, samplog_LogLevelConfig> _levelConfigs;
	GlobalConfig _globalConfig;
	std::unique_ptr<FileChangeDetector> _fileWatcher;

private: // functions
	void ParseConfigFile();
	void AddLoggerConfig(std::string const &module_name, Logger::Config &&config)
	{
		auto entry = _loggerConfigs.emplace(module_name, std::move(config));

		// trigger config refresh for logger
		auto it = _loggerConfigEvents.find(module_name);
		if (it != _loggerConfigEvents.end())
			it->second(entry.first->second);
	}

public: // functions
	void Initialize();

	inline void SubscribeLogger(Logger *logger, ConfigUpdateEvent_t &&cb)
	{
		std::lock_guard<std::mutex> lock(_configLock);
		auto e_it = _loggerConfigEvents.emplace(logger->GetModuleName(),
			std::forward<ConfigUpdateEvent_t>(cb));
		auto it = _loggerConfigs.find(logger->GetModuleName());
		if (it != _loggerConfigs.end())
			e_it.first->second(it->second);
	}
	inline void UnsubscribeLogger(Logger *logger)
	{
		std::lock_guard<std::mutex> lock(_configLock);
		_loggerConfigEvents.erase(logger->GetModuleName());
	}

	samplog_LogLevelConfig Getsamplog_LogLevelConfig(samplog_LogLevel level) const
	{
		std::lock_guard<std::mutex> lock(_configLock);
		auto it = _levelConfigs.find(level);
		if (it != _levelConfigs.end()) {
			return it->second;
		}
		return {};
	}
	GlobalConfig GetGlobalConfig() const
	{
		std::lock_guard<std::mutex> lock(_configLock);
		return _globalConfig;
	}
};
