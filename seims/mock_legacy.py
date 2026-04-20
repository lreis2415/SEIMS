import numpy as np
import random
from types import SimpleNamespace


# Mock Config Parser
def get_config_parser(): return "MockRawConfigParser"


# Mock SAConfig and subclasses
class SAConfig:
    def __init__(self, cf):
        self.bmps_cfg_unit = "SLPPOS"
        self.bmps_cfg_method = "SUIT"
        self.enable_implementation_order = False
        self.genes_num = 10
        self.opt = SimpleNamespace(npop=10, ngens=5, rcross=0.8, rmut=0.1, pmut=0.1)
        self.units_infos = {}
        self.gene_to_unit = {}
        self.unit_to_gene = {}
        self.change_times = 5
        self.users = {
            "user1": {'history_good': [], 'history_bad': [], 'history_bad_reasons': [], 'preference_param': {}}}

    def construct_indexes_units_gene(self): pass


class SASlpPosConfig(SAConfig): pass


class SAConnFieldConfig(SAConfig): pass


# Mock SUScenario
class SUScenario:
    def __init__(self, cfg):
        self.cfg = cfg
        self.ID = random.randint(1000, 9999)
        self.economy = 0
        self.environment = 0
        self.fitness = SimpleNamespace(values=(100, 0.5))
        self.suit_bmps = {}
        self.bmps_grade = {}

    def set_unique_id(self): return self.ID

    def decoding(self): pass

    def decoding_with_bmps_order(self): pass

    def export_to_mongodb(self): pass

    def execute_seims_model(self): pass

    def clean(self, **kwargs): pass

    @property
    def satisfy_investment_constraints(self):
        return True, (np.array([10.]), np.array([5.]), np.array([2.]))

    def calculate_economy(self): self.economy = random.uniform(50000, 100000)

    def calculate_environment(self): self.environment = random.uniform(0.1, 0.9)

    def calculate_economy_bmps_order(self, c, m, i): self.economy = random.uniform(60000, 120000)

    def calculate_environment_bmps_order(self): self.environment = random.uniform(0.2, 0.8)


# Mock Functions
def scenario_effectiveness(cfg, ind):
    sce = SUScenario(cfg)
    sce.calculate_economy()
    sce.calculate_environment()
    ind.fitness.values = (sce.economy, sce.environment)
    return ind


def scenario_effectiveness_with_bmps_order(cfg, ind):
    sce = SUScenario(cfg)
    sce.calculate_economy_bmps_order(None, None, None)
    sce.calculate_environment_bmps_order()
    ind.fitness.values = (sce.economy, sce.environment)
    return ind


def initialize_scenario(cfg): return [0] * cfg.genes_num


def initialize_scenario_s_t(cfg): return [0] * cfg.genes_num


# Mock Operators
def crossover_slppos(ind1, ind2, **kwargs): return ind1, ind2


def crossover_updown(ind1, ind2, **kwargs): return ind1, ind2


def crossover_rdm(ind1, ind2): return ind1, ind2


def mutate_rule_s(ind, **kwargs): return ind,


def mutate_rule_s_t(ind, **kwargs): return ind,


def check_individual_diff(ind1, ind2): return False


# Mock Interactive Tools
def interactive_selection(pop, *args): return pop[:len(pop) // 2], pop[len(pop) // 2:], [], 1


def update_preference_params(*args): return {}


def merge_multiuser_prefs(*args): return {}
