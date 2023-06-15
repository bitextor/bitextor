#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <ostream>
#include <stdexcept>

namespace bitextor {

template <typename K, typename T>
class HashMap {
	std::vector<T> backing_;

	using const_iterator = typename std::vector<T>::const_iterator;

public:
	HashMap(size_t size)
	: backing_(size, 0) {
		//
	}

	T &operator[](K const &key) {
		return backing_[std::hash<K>{}(key) % backing_.size()];
	}

	const_iterator find(K const &key) const {
		return backing_.begin() + (std::hash<K>{}(key) % backing_.size());
	}

	const_iterator end() const {
		return backing_.end();
	}

	void add(HashMap const &other, float alpha = 1.0f) {
		if (backing_.size() != other.backing_.size())
			throw std::logic_error("HashMap::add can only add hashmaps of the same size");

		for (size_t i = 0; i < backing_.size(); ++i)
			backing_[i] += other.backing_[i] * alpha;
	}
};

} // namespace bitextor
