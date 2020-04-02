from artificial_bee_colony.util import randrange_except
import random
from copy import deepcopy

class GLSource:
    def __init__(self, limit, value):
        self.limit = limit
        self.value = value
        self.g_fitness = value.compute_global_fitness()
        self.l_fitness = value.compute_local_fitness()

    def explore(self, buddy, limit, phase):
        new_value = self.value.explore(buddy.value)
        if phase == 'g':
            new_g_fitness = new_value.compute_global_fitness()
            if new_g_fitness > self.g_fitness:
                g_delta = new_g_fitness - self.g_fitness
                self.value = new_value
                self.g_fitness = new_g_fitness
                new_l_fitness = new_value.compute_local_fitness()
                l_delta = new_l_fitness - self.l_fitness
                self.l_fitness = new_l_fitness
                self.limit = limit
                return g_delta, l_delta
            else:
                return 0, 0
        elif phase == 'l':
            new_l_fitness = new_value.compute_local_fitness()
            if new_l_fitness > self.l_fitness:
                l_delta = new_l_fitness - self.l_fitness
                self.value = new_value
                self.l_fitness = new_l_fitness
                new_g_fitness = new_value.compute_global_fitness()
                g_delta = new_g_fitness - self.g_fitness
                self.g_fitness = new_g_fitness
                self.limit = limit
                return g_delta, l_delta
            else:
                return 0, 0

    def abandon(self, limit):
        self.limit = limit
        self.value = self.value.random(*self.value.params())
        old_g_fitness = self.g_fitness
        self.g_fitness = self.value.compute_global_fitness()
        old_l_fitness = self.l_fitness
        self.l_fitness = self.value.compute_local_fitness()
        return self.g_fitness - old_g_fitness, self.l_fitness - old_l_fitness

class GLBeeColony:
    def __init__(self, target_type, *params):
        self.target_type = target_type
        self.params = params

    def clusterize(self, population_size, iterations, limit):
        sources = [GLSource(limit, self.target_type.random(*self.params)) for _ in range(population_size)]
        best_local_source = deepcopy(sources[0])
        best_global_source = deepcopy(sources[0])
        g_nectar = sum(source.g_fitness for source in sources)
        l_nectar = sum(source.l_fitness for source in sources)
        for iteration in range(iterations):
            #p = (0.05 / (0.1 + (0.4) * (iteration / iterations)) - 0.1) * 0.9 / 0.4 + 0.05
            p = iteration / iterations
            phase = random.choices(['g', 'l'], [p, 1 - p])[0]
            
            for i, source in enumerate(sources):
                buddy = sources[randrange_except(len(sources), i)]
                g_delta, l_delta = source.explore(buddy, limit, phase)
                g_nectar += g_delta
                l_nectar += l_delta
            
            for _ in range(population_size):
                if phase == 'g':
                    source_idx = random.choices(range(len(sources)), [source.g_fitness / g_nectar for source in sources])[0]
                elif phase == 'l':
                    source_idx = random.choices(range(len(sources)), [source.l_fitness / l_nectar for source in sources])[0]
                source = sources[source_idx]
                buddy = sources[randrange_except(len(sources), source_idx)]
                g_delta, l_delta = source.explore(buddy, limit, phase)
                g_nectar += g_delta
                l_nectar += l_delta

            for source in sources:
                if phase == 'l':
                    if source.l_fitness > best_local_source.l_fitness:
                        best_local_source = deepcopy(source)
                elif phase == 'g':
                    if source.g_fitness > best_global_source.g_fitness:
                        best_global_source = deepcopy(source)
                if source.limit <= 0:
                    g_delta, l_delta = source.abandon(limit)
                    g_nectar += g_delta
                    l_nectar += l_delta
                else:
                    source.limit -= 1
        return best_local_source, best_global_source
