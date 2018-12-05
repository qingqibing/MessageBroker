#pragma once
#include <memory>

//make make_unique in c++ 11
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename Container, typename Element>
bool append_unique(Container& c, const Element& ele) {
	if (find(begin(c), end(c), ele) == end(c)) {
		c.emplace_back(ele);
		return true;
	}
	assert(!c.empty());
	return false;
}

template<typename Container, typename Element>
bool remove_first(Container& c, const Element& ele) {
	auto it = find(begin(c), end(c), ele);
	if (it != end(c)) {
		c.erase(it);
		return true;
	}
	return false;
}
