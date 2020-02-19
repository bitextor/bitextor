#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

namespace bitextor {

template <typename T> class blocking_queue
{
public:
	blocking_queue(size_t capacity);
	
	void push(T const &item);
	void push(T &&item);
	T pop(); // TODO: explicit move semantics?
private:
	size_t _size;
	std::queue<T> _buffer;
	std::mutex _mutex;
	std::condition_variable _added;
	std::condition_variable _removed;
};

template <typename T> blocking_queue<T>::blocking_queue(size_t size) : _size(size) {
	//
}

template <typename T> void blocking_queue<T>::push(T &&item) {
	std::unique_lock<std::mutex> mlock(_mutex);

	while (_buffer.size() >= _size)
		_removed.wait(mlock);

	_buffer.push(std::move(item));
	mlock.unlock();
	_added.notify_one();
}

template <typename T> void blocking_queue<T>::push(T const &item) {
	std::unique_lock<std::mutex> mlock(_mutex);
	
	while (_buffer.size() >= _size)
		_removed.wait(mlock);
	
	_buffer.push(item);
	mlock.unlock();
	_added.notify_one();
}

template <typename T> T blocking_queue<T>::pop() {
	std::unique_lock<std::mutex> mlock(_mutex);
	
	while (_buffer.empty())
		_added.wait(mlock);
	
	T value = std::move(_buffer.front());
	_buffer.pop();
	mlock.unlock();
	_removed.notify_one();
	return value;
}

} // namespace bitextor
