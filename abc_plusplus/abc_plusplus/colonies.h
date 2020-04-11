#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <random>

#include "util.h"

template <typename ProblemType>
class ClassicMixingStrategy {
public:
	template <typename BeeType, typename RNGType>
	ProblemType mutate(size_t bee_idx, const std::vector<BeeType>& swarm, const BeeType& champion, RNGType& rng) {
		size_t buddy_index = uniform_int_except(0, swarm.size() - 1, bee_idx, rng);
		const BeeType& buddy = swarm[buddy_index];

		return mix(swarm[bee_idx].get_state(), buddy.get_state(), rng);
	}

	template <typename RNGType>
	ProblemType mix(ProblemType problem, const ProblemType& buddy, RNGType& rng) {
		std::uniform_int_distribution<size_t> gene_select_dist(0, problem.gene_count() - 1);
		std::uniform_real_distribution<double> coeff_dist(-1.0, 1.0);

		size_t mixed = gene_select_dist(rng);
		typename ProblemType::gene_type new_gene = problem.get_gene(mixed);
		new_gene += coeff_dist(rng) * (new_gene - buddy.get_gene(mixed));
		new_gene.repair();
		problem.set_gene(mixed, new_gene);

		return problem;
	}
};

template <typename ProblemType>
class DEMixingStrategy {
public:
	DEMixingStrategy(double f, double mr) :
		f(f),
		mr(mr) {

	}

	template <typename BeeType, typename RNGType>
	ProblemType mutate(size_t bee_idx, const std::vector<BeeType>& swarm, const BeeType& champion, RNGType& rng) {
		std::array<size_t, 3> buddies = uniform_ints_except<3, RNGType>(0, swarm.size() - 1, bee_idx, rng);

		return mix(champion.get_state(), swarm[bee_idx].get_state(), swarm[buddies[0]].get_state(), swarm[buddies[1]].get_state(), swarm[buddies[2]].get_state(), rng);
	}

	template <typename RNGType>
	ProblemType mix(ProblemType problem, const ProblemType& buddy1, const ProblemType& buddy2, const ProblemType& buddy3, const ProblemType& buddy4, RNGType& rng) {
		std::uniform_real_distribution<double> gene_select_dist(0.0, 1.0);

		std::vector<double> gene_selections;
		gene_selections.reserve(problem.gene_count());
		for (size_t gene_idx = 0; gene_idx < problem.gene_count(); ++gene_idx) {
			gene_selections.push_back(gene_select_dist(rng));
		}

		//always do at least 1 mutation
		if (std::find_if(gene_selections.cbegin(), gene_selections.cend(), [this](double val) { return val <= this->mr; }) == gene_selections.cend()) {
			std::uniform_int_distribution<size_t> emergency_gene_select_dist(0, gene_selections.size() - 1);
			gene_selections[emergency_gene_select_dist(rng)] = 0.0;
		}

		for (size_t gene_idx = 0; gene_idx < gene_selections.size(); ++gene_idx) {
			if (gene_selections[gene_idx] <= mr) {
				typename ProblemType::gene_type new_gene = problem.get_gene(gene_idx);
				new_gene += f * (buddy1.get_gene(gene_idx) - buddy2.get_gene(gene_idx) + buddy3.get_gene(gene_idx) - buddy4.get_gene(gene_idx));
				new_gene.repair();
				problem.set_gene(gene_idx, new_gene);
			}
		}

		return problem;
	}

private:
	double f;
	double mr;
};

template <typename ProblemType, typename MixingStrategy>
class Bee {
public:
	Bee(size_t limit, ProblemType problem, MixingStrategy mixing_strategy):
		problem(problem),
		limit(limit),
		remaining_cycles(limit),
		fitness(problem.compute_fitness()),
		mixing_strategy(mixing_strategy) {
	}
	
	template <typename RNGType>
	typename ProblemType::fitness_type explore(size_t my_idx, const std::vector<Bee<ProblemType, MixingStrategy>>& swarm, const Bee<ProblemType, MixingStrategy>& champion, RNGType& rng) {
		ProblemType hybrid = mixing_strategy.mutate(my_idx, swarm, champion, rng);

		typename ProblemType::fitness_type new_fitness = hybrid.compute_fitness();
		if (new_fitness > fitness) {
			problem = std::move(hybrid);
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
	MixingStrategy mixing_strategy;
};

template <typename ProblemType, typename MixingStrategy, typename RNGType>
std::vector<Bee<ProblemType, MixingStrategy>> generate_population(typename ProblemType::params_type params, size_t limit, size_t size, MixingStrategy mixing_strategy, RNGType& rng) {
	std::vector<Bee<ProblemType, MixingStrategy>> result;
	for (size_t i = 0; i < size; ++i) {
		result.emplace_back(limit, ProblemType(params, rng), mixing_strategy);
	}
	return result;
}

template <typename ProblemType, typename MixingStrategy, typename RNGType>
class ArtificialBeeColony {
public:
	ArtificialBeeColony(typename ProblemType::params_type problem_params, size_t population, size_t limit, MixingStrategy mixing_strategy, RNGType&& rng):
		rng(std::move(rng)),
		problem_params(problem_params),
		bees(generate_population<ProblemType, MixingStrategy, RNGType>(problem_params, limit, population, mixing_strategy, rng)),
		champion(*std::max_element(bees.cbegin(), bees.cend(), [](const auto& a, const auto& b) { return a.get_fitness() < b.get_fitness(); })),
		all_nectar(std::accumulate(bees.cbegin(), bees.cend(), 0.0, [](typename ProblemType::fitness_type a, const auto& b) { return a + b.get_fitness(); })) {
	}

	void optimize(size_t max_iterations) {
		for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
			for (size_t i = 0; i < bees.size(); ++i) {
				all_nectar += bees[i].explore(i, bees, champion, rng);
			}

			for (size_t i = 0; i < bees.size(); ++i) {
				std::uniform_real_distribution<double> roulette_distribution(0.0, all_nectar);
				size_t source_index = roulette(roulette_distribution(rng), bees.cbegin(), bees.cend(), [](const auto& bee) { return bee.get_fitness(); });

				all_nectar += bees[source_index].explore(source_index, bees, champion, rng);
			}

			for (Bee<ProblemType, MixingStrategy>& bee: bees) {
				if (bee.get_fitness() > champion.get_fitness()) {
					champion = bee;
				}

				all_nectar += bee.tire(rng);
			}
		}
	}

	const Bee<ProblemType, MixingStrategy>& get_champion() const noexcept {
		return champion;
	}

private:
	const typename ProblemType::params_type problem_params;
	RNGType rng;
	std::vector<Bee<ProblemType, MixingStrategy>> bees;
	Bee<ProblemType, MixingStrategy> champion;
	typename ProblemType::fitness_type all_nectar;
};


template <size_t dim>
using ABCFuzzyClustering = ArtificialBeeColony<FuzzyClustering<dim>, ClassicMixingStrategy<FuzzyClustering<dim>>, std::mt19937_64>;

template <size_t dim>
using ModABCFuzzyClustering = ArtificialBeeColony<FuzzyClustering<dim>, DEMixingStrategy<FuzzyClustering<dim>>, std::mt19937_64>;
