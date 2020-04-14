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
	ModABCFuzzyClustering<2>colony{ params, 20, 200, DEMixingStrategy<FuzzyClustering<2>>(0.8, 0.1), std::mt19937_64() };
	colony.optimize(1000);
}