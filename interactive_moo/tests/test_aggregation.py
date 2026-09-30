import unittest

from deap import base, creator

from interactive_moo.aggregation import aggregate_preferences, select_population
from interactive_moo.model import Preference, Stakeholder


if not hasattr(creator, "InteractiveMOOFitness"):
    creator.create("InteractiveMOOFitness", base.Fitness, weights=(-1.0, 1.0))
    creator.create("InteractiveMOOIndividual", list, fitness=creator.InteractiveMOOFitness)


class AggregationTest(unittest.TestCase):
    def setUp(self):
        self.population = []
        for economy, environment in ((10, 1), (20, 4), (40, 9), (50, 10)):
            individual = creator.InteractiveMOOIndividual([economy, environment])
            individual.economy = economy
            individual.environment = environment
            individual.fitness.values = (economy, environment)
            self.population.append(individual)
        self.users = [
            Stakeholder("cost", {"economy": Preference(20, "less", 30)}),
            Stakeholder("environment", {"environment": Preference(8, "greater", 8)}),
        ]

    def test_function_aggregation(self):
        selected = select_population(self.population, 2, self.users, "Func_agg")
        self.assertEqual(len(selected), 2)
        self.assertTrue(hasattr(selected[0].fitness, "preference_score"))

    def test_elite_aggregation(self):
        selected = select_population(self.population, 2, self.users, "Elite_agg")
        self.assertEqual(len(selected), 2)
        self.assertEqual(len({id(item) for item in selected}), 2)

    def test_seims_legacy_bridge_uses_explicit_mode(self):
        import sys
        if "seims" not in sys.path:
            sys.path.insert(0, "seims")
        from scenario_analysis.interactive_algorithm import InteractiveAlgorithm

        users = {
            "cost": {"preference_param": {"economy": [20, "less", 30]}},
            "environment": {"preference_param": {"environment": [8, "greater", 8]}},
        }
        algorithm = InteractiveAlgorithm(True, 10, users, "Elite_agg")
        toolbox = base.Toolbox()
        from deap import tools
        toolbox.register("select", tools.selNSGA2)
        algorithm.register_to_toolbox(toolbox)
        self.assertEqual(algorithm.preference_fusion_strategy, "Elite_agg")
        self.assertEqual(len(algorithm.select_population(toolbox, self.population, 2)), 2)

    def test_conflicting_functions_are_rejected(self):
        users = [
            Stakeholder("a", {"x": Preference(1, "less", 1)}),
            Stakeholder("b", {"x": Preference(1, "greater", 1)}),
        ]
        with self.assertRaises(ValueError):
            aggregate_preferences(users)


if __name__ == "__main__":
    unittest.main()
