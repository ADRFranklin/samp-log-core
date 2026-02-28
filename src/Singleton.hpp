#pragma once

#include <mutex>

template<class T>
class Singleton
{
protected:
	static T *_instance;
	static std::mutex _instanceMutex;

public:
	Singleton() { }
	virtual ~Singleton() { }

	inline static T *Get()
	{
		std::lock_guard<std::mutex> lock(_instanceMutex);
		if (_instance == nullptr) {
			_instance = new T;
		}
		return _instance;
	}

	inline static void Destroy()
	{
		std::lock_guard<std::mutex> lock(_instanceMutex);
		if (_instance != nullptr)
		{
			delete _instance;
			_instance = nullptr;
		}
	}
};

template <class T>
T* Singleton<T>::_instance = nullptr;

template <class T>
std::mutex Singleton<T>::_instanceMutex;
