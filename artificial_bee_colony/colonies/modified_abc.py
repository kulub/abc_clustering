from artificial_bee_colony.util import multi_randrange_except
import random
from copy import deepcopy

class ModSource:
    def __init__(self, limit, value):
        self.limit = limit
        self.value = value
        self.fitness = value.compute_fitness()

    def explore(self, gmax, buddy, rando1, rando2):
        return ModSource(self.limit, self.value.explore(gmax.value, buddy.value, rando1.value, rando2.value))

    def abandon(self, limit):
        self.limit = limit
        self.value = self.value.random(*self.value.params())
        self.fitness = self.value.compute_fitness()

    def replace(self, other, limit):
        self.value = other.value
        self.fitness = other.fitness
        self.limit = limit

class ModBeeColony:
    def __init__(self, target_type, *params):
        self.target_type = target_type
        self.params = params

    def clusterize(self, population_size, iterations, limit):
        sources = [ModSource(limit, self.target_type.random(*self.params)) for _ in range(population_size)]
        best_source = deepcopy(sources[0])
        all_nectar = sum(source.fitness for source in sources)
        for _ in range(iterations):
            for i, source in enumerate(sources):
                buddies = multi_randrange_except(len(sources), 3, i)
                new_source = source.explore(best_source, sources[buddies[0]],  sources[buddies[1]], sources[buddies[2]])
                if new_source.fitness > source.fitness:
                    all_nectar += new_source.fitness - source.fitness
                    source.replace(new_source, limit)
            
            for _ in range(population_size):
                source_idx = random.choices(range(len(sources)), [source.fitness / all_nectar for source in sources])[0]
                source = sources[source_idx]
                buddies = multi_randrange_except(len(sources), 3, source_idx)
                new_source = source.explore(best_source, sources[buddies[0]], sources[buddies[1]], sources[buddies[2]])
                if new_source.fitness > source.fitness:
                    all_nectar += new_source.fitness - source.fitness
                    source.replace(new_source, limit)

            for source in sources:
                if source.fitness > best_source.fitness:
                    best_source = deepcopy(source)
                if source.limit <= 0:
                    all_nectar -= source.fitness
                    source.abandon(limit)
                    all_nectar += source.fitness
                else:
                    source.limit -= 1
        return best_source
