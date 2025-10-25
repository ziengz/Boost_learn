#include <iostream>
#include <mutex>
#include <memory>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
using namespace std;

template <typename T>
class Singleton
{
protected:
	Singleton() = default;
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static std::shared_ptr<T> _instance;
public:
	static std::shared_ptr<T> GetInstance()
	{
		static once_flag s_flag;
		call_once(s_flag, [&]() {
			_instance = std::shared_ptr<T>(new T);
			});
			return _instance;
	}
	void PainterAddress() {
		cout << _instance.get() << endl;
	}
	~Singleton() {
		cout << "this is singleton destruct" << endl;
	}

};

template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;