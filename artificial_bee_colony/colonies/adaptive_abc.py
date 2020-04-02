import numpy as np
from artificial_bee_colony.util import randrange_except
import random
from copy import deepcopy
from scipy import special

class AdaptiveSource:
    def __init__(self, limit, value):
        self.limit = limit
        self.value = value
        self.fitnesses = value.compute_fitness()
        self.trend = np.ones(len(self.fitnesses)) / len(self.fitnesses)
        self.fitness = np.prod(self.fitnesses)

    def explore(self, buddy, limit):
        disposition = special.softmax(self.fitnesses / self.trend)[0]
        new_value = self.value.explore(buddy.value, disposition)
        new_fitnesses = np.array(new_value.compute_fitness())
        new_fitness = np.prod(new_fitnesses)
        if new_fitness > self.fitness:
            delta = new_fitness - self.fitness
            self.value = new_value
            self.trend = 0.9 * self.trend + 0.1 * new_fitnesses
            self.fitness = new_fitness
            self.fitnesses = new_fitnesses
            self.dispoisiton = new_disposition
            self.limit = limit
            return delta
        else:
            return 0

    def abandon(self, limit):
        self.limit = limit
        self.value = self.value.random(*self.value.params())
        self.fitnesses = value.compute_fitness()
        self.trend = np.ones(len(self.fitnesses)) / len(self.fitnesses)
        self.fitness = np.prod(self.fitnesses)

class AdaptiveBeeColony:
    def __init__(self, target_type, *params):
        self.target_type = target_type
        self.params = params

    def clusterize(self, population_size, iterations, limit):
        sources = [AdaptiveSource(limit, self.target_type.random(*self.params)) for _ in range(population_size)]
        best_source = deepcopy(sources[0])
        all_nectar = sum(source.fitness for source in sources)
        for _ in range(iterations):
            for i, source in enumerate(sources):
                buddy = sources[randrange_except(len(sources), i)]
                all_nectar += source.explore(buddy, limit)
            
            for _ in range(population_size):
                source_idx = random.choices(range(len(sources)), [source.fitness / all_nectar for source in sources])[0]
                source = sources[source_idx]
                buddy = sources[randrange_except(len(sources), source_idx)]
                all_nectar += source.explore(buddy, limit)

            for source in sources:
                if source.fitness > best_source.fitness:
                    best_source = deepcopy(source)
                if source.limit <= 0:
                    source.abandon(limit)
                else:
                    source.limit -= 1
        return best_source
