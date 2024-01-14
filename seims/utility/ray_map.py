# See example (GA) onemax_ray.py
# See example (GP) symbreg_ray.py
# See example (GP) symbreg_numpy_ray.py

# Derek M Tishler
# Jul 23 2020

'''
A replacement for map using Ray that automates batching to many processors/cluster. Fixes pickle issues such as
DeltaPenalty and addEphemeralConstant when working at scale(many processes on machine or cluster of nodes)
'''
import logging
import math
from time import sleep
from time import time

import ray
from ray.util import ActorPool


@ray.remote(num_cpus=1,scheduling_strategy="SPREAD")
class Ray_Deap_Map():
    def __init__(self, creator_setup=None, pset_creator=None):
        # issue 946? Ensure non trivial startup to prevent bad load balance across a cluster
        sleep(0.01)

        # recreate scope from global
        # For GA no need to provide pset_creator. Both needed for GP
        self.creator_setup = creator_setup
        if creator_setup is not None:
            self.creator_setup()

        self.pset_creator = pset_creator
        if pset_creator is not None:
            self.pset_creator()

    def ray_remote_eval_batch(self, f, zipped_input):
        iterable, id_ = zipped_input
        # attach id so we can reorder the batches
        return [(f(i), id_) for i in iterable]


class Ray_Deap_Map_Manager():
    def __init__(self, creator_setup=None, pset_creator=None, workers=None, cpus_per_worker=1):

        # Can adjust the number of processes in ray.init or when launching cluster
        self.ncpus = int(ray.cluster_resources()['CPU'])
        self.cpus_per_worker = cpus_per_worker
        logging.info(f"Ray_Deap_Map_Manager: ncpus={self.ncpus}, cpus_per_worker={self.cpus_per_worker}")
        if workers:
            self.n_workers = workers
        else:
            self.n_workers = self.ncpus // cpus_per_worker
        # recreate scope from global (for ex need toolbox in gp too)
        self.creator_setup = creator_setup
        self.pset_creator = pset_creator

    def map(self, func, iterable):

        if self.n_workers == 1:
            # only 1 worker, normal listcomp/map will work fine. Useful for testing code?
            ##results = [func(item) for item in iterable]
            results = list(map(func, iterable))  # forced eval to time it
        else:
            # many workers, lets use ActorPool

            if len(iterable) < self.n_workers:
                n_workers = len(iterable)
            else:
                n_workers = self.n_workers

            n_per_batch = math.ceil(len(iterable) / n_workers)
            batches = [iterable[i:i + n_per_batch] for i in range(0, len(iterable), n_per_batch)]
            id_for_reorder = range(len(batches))
            logging.info(f"Ray_Deap_Map_Manager: n_workers={n_workers}, n_per_batch={n_per_batch}, len(batches)={len(batches)}")
            eval_pool = ActorPool([Ray_Deap_Map.options(num_cpus=self.cpus_per_worker).remote(self.creator_setup, self.pset_creator) for _ in range(n_workers)])

            unordered_results = list(eval_pool.map_unordered(lambda actor, input_tuple: actor.ray_remote_eval_batch.remote(func, input_tuple),
                                                             zip(batches, id_for_reorder)))

            # ensure order of batches
            # ordered_batch_results = [batch for batch_id in id_for_reorder for batch in unordered_results if batch_id == batch[0][1]]
            unordered_results.sort(key=lambda x: x[0][1])
            # flatten batches to list of fitnes
            results = [item[0] for sublist in unordered_results for item in sublist]

        return results


# This is what we register as map in deap toolbox.
# For GA no need to provide pset_creator. Both needed for GP
def ray_deap_map(func, pop, creator_setup=None, pset_creator=None, workers=None, cpus_per_worker=1):
    # Manager will determine if batching is needed and crate remote actors to do work
    map_ray_manager = Ray_Deap_Map_Manager(creator_setup=creator_setup, pset_creator=pset_creator, workers=workers, cpus_per_worker=cpus_per_worker)
    results = map_ray_manager.map(func, pop)

    return results
