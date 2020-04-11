#include <vector>
#include<array>

#include "problems.h"
#include "colonies.h"

int main() {
	std::vector<std::array<double, 2>> vec;
	vec.push_back({ 1, 2 });
	vec.push_back({ 5, 5 });
	vec.push_back({ 6, 5 });
	vec.push_back({ 0, 1 });
	vec.push_back({ 19, 20 });
	vec.push_back({ 26, 21 });
	vec.push_back({ -1, -2 });
	vec.push_back({ -3, -2 });

	FuzzyClusteringParams<2> params;
	params.n_clusters = 4;
	params.vectors = &vec;
	ArtificialBeeColony<FuzzyClustering<2>, ClassicMixingStrategy<FuzzyClustering<2>>, std::mt19937_64> colony{ params, 20, 200, std::mt19937_64() };
	colony.optimize(1000);
}