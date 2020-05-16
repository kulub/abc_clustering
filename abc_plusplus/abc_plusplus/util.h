/*
Auxiliary functions
*/
#pragma once

#include <random>
#include <array>
#include <cmath>
#include <algorithm>

template <typename RNGType>
size_t uniform_int_except(size_t min, size_t max, size_t excluded, RNGType rng) {
	std::uniform_int_distribution<size_t> dist(min, max - 1);
	size_t random = dist(rng);
	return random < excluded ? random : random + 1;
}

template <size_t count, typename RNGType>
std::array<size_t, count> uniform_ints(size_t min, size_t max, RNGType rng) {
	static std::vector<size_t> numbers;
	if (numbers.size() != max - min + 1) {
		numbers.resize(max - min + 1);
		for (size_t i = 0; i < numbers.size(); ++i) {
			numbers[i] = i;
		}
	}

	std::array<size_t, count> result;
	for (size_t i = 0; i < count; ++i) {
		std::uniform_int_distribution<size_t> dist(i, max - min);
		size_t random = dist(rng);

		result[i] = numbers[random];
		std::swap(numbers[random], numbers[i]);
	}

	return result;
}

template <typename RNGType>
std::vector<size_t> uniform_ints(size_t min, size_t max, size_t count, RNGType rng) {
	static std::vector<size_t> numbers;
	if (numbers.size() != max - min + 1) {
		numbers.resize(max - min + 1);
		for (size_t i = 0; i < numbers.size(); ++i) {
			numbers[i] = i;
		}
	}

	std::vector<size_t> result;
	result.resize(count);
	for (size_t i = 0; i < count; ++i) {
		std::uniform_int_distribution<size_t> dist(i, max - min);
		size_t random = dist(rng);

		result[i] = numbers[random];
		std::swap(numbers[random], numbers[i]);
	}

	return result;
}

template <size_t count, typename RNGType>
std::array<size_t, count> uniform_ints_except(size_t min, size_t max, size_t excluded, RNGType rng) {
	std::array<size_t, count> result = uniform_ints<count, RNGType>(min, max - 1, rng);
	for (size_t& number: result) {
		if (number >= excluded) {
			++number;
		}
	}

	return result;
}

template <typename ValueType, typename IterType, typename TransformOpType>
size_t roulette(ValueType target, IterType begin, IterType end, TransformOpType transform_op) {
	size_t winner = 0;
	while (begin != end) {
		target -= transform_op(*begin);
		if (target <= 0.0) {
			return winner;
		}
		++winner;
		++begin;
	}

	return winner - 1;
}

template <size_t size>
std::array<double, size>& operator+=(std::array<double, size>& a, const std::array<double, size>& b) {
	std::transform(a.begin(), a.end(), b.cbegin(), a.begin(), [](double x, double y) { return x + y; });
	return a;
}

template <size_t size>
std::array<double, size> operator+(std::array<double, size> a, const std::array<double, size>& b) {
	a += b;
	return a;
}

template <size_t size>
std::array<double, size>& operator-=(std::array<double, size>& a, const std::array<double, size>& b) {
	std::transform(a.begin(), a.end(), b.cbegin(), a.begin(), [](double x, double y) { return x - y; });
	return a;
}

template <size_t size>
std::array<double, size> operator-(std::array<double, size> a, const std::array<double, size>& b) {
	a -= b;
	return a;
}

template <size_t size>
std::array<double, size>& operator*=(std::array<double, size>& a, const std::array<double, size>& b) {
	std::transform(a.begin(), a.end(), b.cbegin(), a.begin(), [](double x, double y) { return x * y; });
	return a;
}

template <size_t size>
std::array<double, size> operator*(std::array<double, size> a, const std::array<double, size>& b) {
	a *= b;
	return a;
}

template <size_t size>
std::array<double, size>& operator/=(std::array<double, size>& a, const std::array<double, size>& b) {
	std::transform(a.begin(), a.end(), b.cbegin(), a.begin(), [](double x, double y) { return x / y; });
	return a;
}

template <size_t size>
std::array<double, size> operator/(std::array<double, size> a, const std::array<double, size>& b) {
	a /= b;
	return a;
}

template <size_t size>
std::array<double, size>& operator*=(std::array<double, size>& a, double b) {
	std::transform(a.begin(), a.end(), a.begin(), [b](double x) { return x * b; });
	return a;
}

template <size_t size>
std::array<double, size> operator*(std::array<double, size> a, double b) {
	a *= b;
	return a;
}

template <size_t size>
std::array<double, size>& operator/=(std::array<double, size>& a, double b) {
	std::transform(a.begin(), a.end(), a.begin(), [b](double x) { return x / b; });
	return a;
}

template <size_t size>
std::array<double, size> operator/(std::array<double, size> a, double b) {
	a /= b;
	return a;
}

template <typename ArrayIterType, typename OutputIterType, typename CoeffIterType>
void elementwise_multiply(ArrayIterType array_begin, OutputIterType output_begin, CoeffIterType coeffs_begin, CoeffIterType coeffs_end) {
	while (coeffs_begin != coeffs_end) {
		*output_begin = *array_begin * *coeffs_begin;
		++array_begin;
		++output_begin;
		++coeffs_begin;
	}
}

template <typename IterType1, typename IterType2>
double euclidean_dist(IterType1 begin1, IterType1 end1, IterType2 begin2) {
	double result = 0.0;
	while (begin1 != end1) {
		double diff = *begin1 - *begin2;
		result += diff * diff;

		++begin1;
		++begin2;
	}

	return std::sqrt(result);
}
