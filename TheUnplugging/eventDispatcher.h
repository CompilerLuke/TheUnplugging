#pragma once
#include <functional>
#include <vector>

template<class T>
struct EventDispatcher {
	std::vector<std::function<void(T)> > listeners;

	void listen(std::function<void(T)> func) {
		listeners.push_back(func);
	}

	void broadcast(T mesg) {
		for (auto func : listeners) {
			func(mesg);
		}
	}
};

