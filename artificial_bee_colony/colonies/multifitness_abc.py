from artificial_bee_colony.util import randrange_except
import random
from copy import deepcopy

class MultiSource:
    def __init__(self, limit, value):
        self.limit = limit
        self.value = value
        self.fitness = [facet[0](value) for facet in value.facets()]

    def explore(self, buddy, limit):
        old_fitness = self.fitness[0]
        for i, facet in enumerate(self.value.facets()):
            facet_value = facet[1](self.value, buddy.value)
            facet_fitness = facet[0](facet_value)
            if facet_fitness > self.fitness[i]:
                facet[2](self.value, facet_value)
                self.fitness[i] = facet_fitness
                self.limit = limit
            
        return self.fitness[0] - old_fitness

    def abandon(self, limit):
        self.limit = limit
        self.value = self.value.random(*self.value.params())
        self.fitness = [facet[0](self.value) for facet in self.value.facets()]

class MultiBeeColony:
    def __init__(self, target_type, *params):
        self.target_type = target_type
        self.params = params

    def clusterize(self, population_size, iterations, limit):
        sources = [MultiSource(limit, self.target_type.random(*self.params)) for _ in range(population_size)]
        best_source = deepcopy(sources[0])
        all_nectar = sum(source.fitness[0] for source in sources)
        for _ in range(iterations):
            for i, source in enumerate(sources):
                buddy = sources[randrange_except(len(sources), i)]
                all_nectar += source.explore(buddy, limit)
            
            for _ in range(population_size):
                source_idx = random.choices(range(len(sources)), [source.fitness[0] / all_nectar for source in sources])[0]
                source = sources[source_idx]
                buddy = sources[randrange_except(len(sources), source_idx)]
                all_nectar += source.explore(buddy, limit)

            for source in sources:
                if source.fitness[0] > best_source.fitness[0]:
                    best_source = deepcopy(source)
                if source.limit <= 0:
                    source.abandon(limit)
                else:
                    source.limit -= 1
        return best_source
