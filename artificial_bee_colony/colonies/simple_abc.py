from artificial_bee_colony.util import randrange_except
import random
from copy import deepcopy

class Source:
    def __init__(self, limit, value):
        self.limit = limit
        self.value = value
        self.fitness = value.compute_fitness()

    def explore(self, buddy):
        return Source(self.limit, self.value.explore(buddy.value))

    def abandon(self, limit):
        self.limit = limit
        self.value = self.value.random(*self.value.params())
        self.fitness = self.value.compute_fitness()

    def replace(self, other, limit):
        self.value = other.value
        self.fitness = other.fitness
        self.limit = limit

class BeeColony:
    def __init__(self, target_type, *params):
        self.target_type = target_type
        self.params = params

    def clusterize(self, population_size, iterations, limit):
        sources = [Source(limit, self.target_type.random(*self.params)) for _ in range(population_size)]
        best_source = deepcopy(sources[0])
        all_nectar = sum(source.fitness for source in sources)
        for _ in range(iterations):
            for i, source in enumerate(sources):
                buddy = sources[randrange_except(len(sources), i)]
                new_source = source.explore(buddy)
                if new_source.fitness > source.fitness:
                    all_nectar += new_source.fitness - source.fitness
                    source.replace(new_source, limit)
            
            for _ in range(population_size):
                source_idx = random.choices(range(len(sources)), [source.fitness / all_nectar for source in sources])[0]
                source = sources[source_idx]
                buddy = sources[randrange_except(len(sources), source_idx)]
                new_source = source.explore(buddy)
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
