#pragma once

#include "colonies.h"
#include "problems.h"

template <size_t dim>
using ABCFuzzyClustering = ArtificialBeeColony<FuzzyClustering<dim>, ClassicMixingStrategy<FuzzyClustering<dim>>, RouletteSelectionStrategy, std::mt19937_64>;

template <size_t dim>
using ModABCFuzzyClustering = ArtificialBeeColony<FuzzyClustering<dim>, DEMixingStrategy<FuzzyClustering<dim>>, RouletteSelectionStrategy, std::mt19937_64>;
