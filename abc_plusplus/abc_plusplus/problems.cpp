#include "problems.h"

#include <numeric>
#include <algorithm>

size_t FuzzyClusteringGene::size() const noexcept {
	return weights.size();
}

FuzzyClusteringGene::iterator FuzzyClusteringGene::begin() {
	return weights.begin();
}

FuzzyClusteringGene::iterator FuzzyClusteringGene::end() {
	return weights.end();
}

FuzzyClusteringGene::const_iterator FuzzyClusteringGene::cbegin() const {
	return weights.cbegin();
}

FuzzyClusteringGene::const_iterator FuzzyClusteringGene::cend() const {
	return weights.cend();
}

double& FuzzyClusteringGene::operator[](size_t index) {
	return weights[index];
}

double FuzzyClusteringGene::operator[](size_t index) const {
	return weights[index];
}

FuzzyClusteringGene& FuzzyClusteringGene::operator+=(const FuzzyClusteringGene& other) {
	std::transform(weights.begin(), weights.end(), other.weights.cbegin(), weights.begin(), [](double a, double b) {return a + b; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator-=(const FuzzyClusteringGene& other) {
	std::transform(weights.begin(), weights.end(), other.weights.cbegin(), weights.begin(), [](double a, double b) {return a - b; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator*=(const FuzzyClusteringGene& other) {
	std::transform(weights.begin(), weights.end(), other.weights.cbegin(), weights.begin(), [](double a, double b) {return a * b; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator/=(const FuzzyClusteringGene& other) {
	std::transform(weights.begin(), weights.end(), other.weights.cbegin(), weights.begin(), [](double a, double b) {return a / b; });
	return *this;
}


FuzzyClusteringGene operator+(FuzzyClusteringGene a, const FuzzyClusteringGene& b) {
	a += b;
	return a;
}

FuzzyClusteringGene operator-(FuzzyClusteringGene a, const FuzzyClusteringGene& b) {
	a -= b;
	return a;
}

FuzzyClusteringGene operator*(FuzzyClusteringGene a, const FuzzyClusteringGene& b) {
	a *= b;
	return a;
}

FuzzyClusteringGene operator/(FuzzyClusteringGene a, const FuzzyClusteringGene& b) {
	a /= b;
	return a;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator+=(double other) {
	std::transform(weights.begin(), weights.end(), weights.begin(), [other](double a) {return a + other; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator-=(double other) {
	std::transform(weights.begin(), weights.end(), weights.begin(), [other](double a) {return a - other; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator*=(double other) {
	std::transform(weights.begin(), weights.end(), weights.begin(), [other](double a) {return a * other; });
	return *this;
}

FuzzyClusteringGene& FuzzyClusteringGene::operator/=(double other) {
	std::transform(weights.begin(), weights.end(), weights.begin(), [other](double a) {return a / other; });
	return *this;
}


FuzzyClusteringGene operator+(FuzzyClusteringGene a, double b) {
	a += b;
	return a;
}

FuzzyClusteringGene operator-(FuzzyClusteringGene a, double b) {
	a -= b;
	return a;
}

FuzzyClusteringGene operator*(FuzzyClusteringGene a, double b) {
	a *= b;
	return a;
}

FuzzyClusteringGene operator/(FuzzyClusteringGene a, double b) {
	a /= b;
	return a;
}


FuzzyClusteringGene operator+(double b, FuzzyClusteringGene a) {
	return a + b;
}

FuzzyClusteringGene operator-(double b, FuzzyClusteringGene a) {
	return a - b;
}

FuzzyClusteringGene operator*(double b, FuzzyClusteringGene a) {
	return a * b;
}

FuzzyClusteringGene operator/(double b, FuzzyClusteringGene a) {
	return a / b;
}


void FuzzyClusteringGene::repair() {
	std::transform(weights.begin(), weights.end(), weights.begin(), [](const double weight) { return std::clamp(weight, 0.0, 1.0); });
	double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
	std::transform(weights.begin(), weights.end(), weights.begin(), [sum](const double weight) { return weight / sum; });
}
