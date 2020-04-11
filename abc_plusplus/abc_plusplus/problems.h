#pragma once

#include <vector>
#include <array>
#include <random>
#include <tuple>
#include <numeric>

#include "util.h"

class FuzzyClusteringGene {
public:
	using iterator = std::vector<double>::iterator;
	using const_iterator = std::vector<double>::const_iterator;

	FuzzyClusteringGene(size_t n_clusters):
		weights(n_clusters) {

	}

	void repair();

	size_t size() const noexcept;

	iterator begin();
	iterator end();
	const_iterator cbegin() const;
	const_iterator cend() const;

	double& operator[](size_t index);
	double operator[](size_t index) const;

	FuzzyClusteringGene& operator+=(const FuzzyClusteringGene& other);
	FuzzyClusteringGene& operator-=(const FuzzyClusteringGene& other);
	FuzzyClusteringGene& operator*=(const FuzzyClusteringGene& other);
	FuzzyClusteringGene& operator/=(const FuzzyClusteringGene& other);

	FuzzyClusteringGene& operator+=(double other);
	FuzzyClusteringGene& operator-=(double other);
	FuzzyClusteringGene& operator*=(double other);
	FuzzyClusteringGene& operator/=(double other);

private:
	std::vector<double> weights;
};

FuzzyClusteringGene operator+(FuzzyClusteringGene a, const FuzzyClusteringGene& b);
FuzzyClusteringGene operator-(FuzzyClusteringGene a, const FuzzyClusteringGene& b);
FuzzyClusteringGene operator*(FuzzyClusteringGene a, const FuzzyClusteringGene& b);
FuzzyClusteringGene operator/(FuzzyClusteringGene a, const FuzzyClusteringGene& b);

FuzzyClusteringGene operator+(FuzzyClusteringGene a, double b);
FuzzyClusteringGene operator-(FuzzyClusteringGene a, double b);
FuzzyClusteringGene operator*(FuzzyClusteringGene a, double b);
FuzzyClusteringGene operator/(FuzzyClusteringGene a, double b);

FuzzyClusteringGene operator+(double b, FuzzyClusteringGene a);
FuzzyClusteringGene operator-(double b, FuzzyClusteringGene a);
FuzzyClusteringGene operator*(double b, FuzzyClusteringGene a);
FuzzyClusteringGene operator/(double b, FuzzyClusteringGene a);

template <size_t n_dim>
struct FuzzyClusteringParams {
	size_t n_clusters;
	std::vector<std::array<double, n_dim>>* vectors;
};

template <size_t n_dim>
class FuzzyClustering {
public:
	using gene_type = FuzzyClusteringGene;
	using fitness_type = double;
	using params_type = FuzzyClusteringParams<n_dim>;

	template <typename RNGType>
	FuzzyClustering(FuzzyClusteringParams<n_dim> params, RNGType& rng):
		weights(params.n_clusters * params.vectors->size()),
		n_clusters(params.n_clusters),
		vectors(params.vectors),
		n_vectors(params.vectors->size()) {

		randomize_value(rng);
	}

	fitness_type compute_fitness() const {
		fitness_type result = 0;

		for (size_t cluster_idx = 0; cluster_idx < n_clusters; ++cluster_idx) {
			std::vector<std::array<double, n_dim>> weighted_vectors(n_vectors);
			elementwise_multiply(vectors->cbegin(), weighted_vectors.begin(), weights.cbegin() + cluster_idx * n_vectors, weights.cbegin() + (cluster_idx + 1) * n_vectors);

			std::array<double, n_dim> weighted_vector_sum = std::accumulate(weighted_vectors.cbegin() + 1, weighted_vectors.cend(), weighted_vectors[0], [](const auto& a, const auto& b) { return a + b; });
			double cluster_weight_sum = std::accumulate(weights.cbegin() + cluster_idx * n_vectors + 1, weights.cbegin() + (cluster_idx + 1) * n_vectors, weights[cluster_idx * n_vectors]);

			std::array<double, n_dim> cluster_center = weighted_vector_sum / cluster_weight_sum;

			for (size_t vector_idx = 0; vector_idx < n_vectors; ++vector_idx) {
				result += weights[cluster_idx * n_vectors + vector_idx] * euclidean_dist((*vectors)[vector_idx].cbegin(), (*vectors)[vector_idx].cend(), cluster_center.cbegin());
			}
		}

		return 1 / result;
	}

	gene_type get_gene(size_t index) const noexcept {
		gene_type result(n_clusters);
		auto iter = result.begin();
		for (size_t i = index; i < weights.size(); i += n_vectors) {
			*iter = weights[i];
			++iter;
		}

		return result;
	}

	void set_gene(size_t index, gene_type new_value) noexcept {
		auto iter = new_value.cbegin();
		for (size_t i = index; i < weights.size(); i += n_vectors) {
			weights[i] = *iter;
			++iter;
		}
	}

	size_t gene_count() const noexcept {
		return n_vectors;
	}

	const std::vector<double>& get_value() const {
		return weights;
	}

	size_t get_n_clusters() const {
		return n_clusters;
	}

	template <typename RNGType>
	void randomize_value(RNGType& rng) {
		std::uniform_real_distribution<double> dist(0, 1);
		for (size_t gene_index = 0; gene_index < n_vectors; ++gene_index) {
			FuzzyClusteringGene gene(n_clusters);
			double sum = 0;
			for (double& value : gene) {
				value = dist(rng);
				sum += value;
			}
			gene /= sum;

			set_gene(gene_index, gene);
		}
	}

private:
	std::vector<std::array<double, n_dim>>* vectors;
	std::vector<double> weights;
	size_t n_clusters;
	size_t n_vectors;
};
