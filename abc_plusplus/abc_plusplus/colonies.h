#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <random>

#include "util.h"

template <typename ProblemType, typename RNGType>
std::vector<Bee<ProblemType>> generate_population(typename ProblemType::params_type params, size_t limit, size_t size, RNGType& rng) {
	std::vector<Bee<ProblemType>> result;
	for (size_t i = 0; i < size; ++i) {
		result.emplace_back(limit, ProblemType(params, rng));
		result.emplace_back(limit, ProblemType(params, rng));
	}
	return result;
}

template <typename ProblemType>
class Bee {
public:
	Bee(size_t limit, ProblemType problem):
		problem(problem),
		limit(limit),
		remaining_cycles(limit),
		fitness(problem.compute_fitness()) {
	}

	template <typename RNGType>
	typename ProblemType::fitness_type explore(const Bee<ProblemType>& buddy, RNGType& rng) {
		std::uniform_int_distribution<size_t> gene_select_dist(0, problem.gene_count() - 1);
		std::uniform_real_distribution<double> coeff_dist(-1.0, 1.0);

		size_t mixed = gene_select_dist(rng);
		ProblemType hybrid = problem;
		typename ProblemType::gene_type mixed_gene = hybrid.get_gene(mixed);
		typename ProblemType::gene_type buddy_gene = buddy.get_state().get_gene(mixed);
		typename ProblemType::gene_type new_gene = mixed_gene + coeff_dist(rng) * (mixed_gene - buddy_gene);
		new_gene.repair();
		hybrid.set_gene(mixed, new_gene);

		typename ProblemType::fitness_type new_fitness = hybrid.compute_fitness();
		if (new_fitness > fitness) {
			problem.set_gene(mixed, new_gene);
			remaining_cycles = limit;

			typename ProblemType::fitness_type delta = new_fitness - fitness;
			fitness = new_fitness;
			return delta;
		}
		else {
			return 0;
		}
	}

	typename ProblemType::fitness_type get_fitness() const noexcept {
		return fitness;
	}

	template <typename RNGType>
	typename ProblemType::fitness_type tire(RNGType& rng) {
		if (remaining_cycles == 0) {
			problem.randomize_value(rng);
			typename ProblemType::fitness_type old_fitness = fitness;
			fitness = problem.compute_fitness();
			remaining_cycles = limit;

			return fitness - old_fitness;
		}
		else {
			--remaining_cycles;
			return 0;
		}
	}

	const ProblemType& get_state() const {
		return problem;
	}

private:
	ProblemType problem;;
	size_t limit;
	size_t remaining_cycles;
	typename ProblemType::fitness_type fitness;
};

template <typename ProblemType, typename RNGType>
class ArtificialBeeColony {
public:
	ArtificialBeeColony(typename ProblemType::params_type problem_params, size_t population, size_t limit, RNGType&& rng):
		rng(std::move(rng)),
		problem_params(problem_params),
		bees(generate_population<ProblemType, RNGType>(problem_params, limit, population, rng)),
		champion(*std::max_element(bees.cbegin(), bees.cend(), [](const Bee<ProblemType>& a, const Bee<ProblemType>& b) { return a.get_fitness() < b.get_fitness(); })),
		all_nectar(std::accumulate(bees.cbegin(), bees.cend(), 0.0, [](typename ProblemType::fitness_type a, const Bee<ProblemType>& b) { return a + b.get_fitness(); })) {
	}

	void optimize(size_t max_iterations) {
		for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
			for (size_t i = 0; i < bees.size(); ++i) {
				size_t buddy_index = uniform_int_except(0, bees.size() - 1, i, rng);
				all_nectar += bees[i].explore(bees[buddy_index], rng);
			}

			for (size_t i = 0; i < bees.size(); ++i) {
				std::uniform_real_distribution<double> roulette_distribution(0.0, all_nectar);

				size_t source_index = roulette(roulette_distribution(rng), bees.cbegin(), bees.cend(), [](const Bee<ProblemType>& bee) { return bee.get_fitness(); });
				size_t buddy_index = uniform_int_except(0, bees.size() - 1, source_index, rng);
				all_nectar += bees[i].explore(bees[buddy_index], rng);
			}

			for (Bee<ProblemType>& bee: bees) {
				if (bee.get_fitness() > champion.get_fitness()) {
					champion = bee;
				}

				bee.tire(rng);
			}
		}
	}

	const Bee<ProblemType>& get_champion() const noexcept {
		return champion;
	}

private:
	const typename ProblemType::params_type problem_params;
	RNGType rng;
	std::vector<Bee<ProblemType>> bees;
	Bee<ProblemType> champion;
	typename ProblemType::fitness_type all_nectar;
};
