#ifndef QUEUE_H
#define QUEUE_H

#include <Arduino.h>

template<class T>
class Queue {
	private:
	int _front, _back, _count;
	T *_data;
	int _maxitems;
	public:
	Queue(int maxitems = 256) {
		_front = 0;
		_back = 0;
		_count = 0;
		_maxitems = maxitems;
		_data = new T[maxitems + 1];
	}
	~Queue() {
		delete[] _data;
	}
	inline int count();
	inline int front();
	inline int back();
	void push(const T &item);
	T peekCurrent();
	T pop();
	T peekPrevious();
	T peekAt(int i);
	void clear();
};

template<class T>
inline int Queue<T>::count()
{
	return _count;
}

template<class T>
inline int Queue<T>::front()
{
	return _front;
}

template<class T>
inline int Queue<T>::back()
{
	return _back;
}

template<class T>
void Queue<T>::push(const T &item)
{
	if(_count < _maxitems) { // Drops out when full
		_data[_back++]=item;
		++_count;
		// Check wrap around
		if (_back > _maxitems)
		_back -= (_maxitems + 1);
	}
	else
	{
		for(byte i=_maxitems; i > 0; i--)
		{
			_data[i] = _data[i-1];
		}
		_data[0] = item;
	}
}

template<class T>
T Queue<T>::pop() {
	if(_count <= 0) return T(); // Returns empty
	else {
		T result = _data[_front];
		_front++;
		--_count;
		// Check wrap around
		if (_front > _maxitems)
		_front -= (_maxitems + 1);
		return result;
	}
}

template<class T>
T Queue<T>::peekCurrent() {
	if(_count <= 0) return T(); // Returns empty
	else return _data[_back];
}

template<class T>
T Queue<T>::peekPrevious() {
	if(_count <= 0) return T(); // Returns empty
	else return _data[_back - 1];
}

template<class T>
T Queue<T>::peekAt(int i) {
	if(_count <= 0) return T(); // Returns empty
	else return _data[i];
}

template<class T>
void Queue<T>::clear()
{
	_front = _back;
	_count = 0;
}

#endif