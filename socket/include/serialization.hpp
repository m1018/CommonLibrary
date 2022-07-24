#pragma once

#include <type_traits>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>


template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> N = 0>
void Serialize(std::ostream & os, const T & val)
{
    os.write((const char *)&val, sizeof(T));
}

template <class T, typename std::enable_if_t<
	std::is_same<typename T::iterator, decltype(std::declval<T>().begin())>::value &&
    std::is_same<typename T::iterator, decltype(std::declval<T>().end())>::value &&
    std::is_trivially_copyable<typename T::value_type>::value, int> N = 0>
    void Serialize(std::ostream & os, const T & val)
{
    unsigned int size = val.size();
    os.write((const char *)&size, sizeof(size));
    os.write((const char *)val.data(), size * sizeof(typename T::value_type));
}

template <class T, typename std::enable_if_t<
	std::is_same<typename T::iterator, decltype(std::declval<T>().begin())>::value &&
    std::is_same<typename T::iterator, decltype(std::declval<T>().end())>::value &&
    !std::is_trivially_copyable<typename T::value_type>::value, int> N = 0>
void Serialize(std::ostream & os, const T & val)
{
	unsigned int size = val.size();
	os.write((const char *)&size, sizeof(size));
	for (auto & v : val) { Serialize(os, v); }
}

//  可平凡复制
template <class T, typename std::enable_if_t<std::is_trivially_copyable<T>::value, int> N = 0>
void Deserialize(std::istream & is, T & val)
{
    is.read((char *)&val, sizeof(T));
}

template <class T, typename std::enable_if_t<
	std::is_same<typename T::iterator, decltype(std::declval<T>().begin())>::value &&
    std::is_same<typename T::iterator, decltype(std::declval<T>().end())>::value &&
    std::is_trivially_copyable<typename T::value_type>::value, int> N = 0>
    void Deserialize(std::istream & is, T & val)
{
    unsigned int size = 0;
	is.read((char *)&size, sizeof(unsigned int));
	val.resize(size);
	is.read((char *)val.data(), size * sizeof(typename T::value_type));
}

template <class T, typename std::enable_if_t<
	std::is_same<typename T::iterator, decltype(std::declval<T>().begin())>::value &&
    std::is_same<typename T::iterator, decltype(std::declval<T>().end())>::value &&
    !std::is_trivially_copyable<typename T::value_type>::value, int> N = 0>
void Deserialize(std::istream & is, T & val)
{
	unsigned int size = 0;
	is.read((char *)&size, sizeof(unsigned int));
	val.resize(size);
	for (auto & v : val) { Deserialize(is, v); }
}