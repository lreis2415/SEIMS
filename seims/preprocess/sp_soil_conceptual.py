from preprocess.sp_soil_base import SoilPropertyBase
from preprocess.text import ParamAbstractionTypes



class SoilPropertyConceptual(SoilPropertyBase):
    def __init__(self, seq_num, seq_name):
        super().__init__(seq_num, seq_name)
        self.SOL_POROSITY = list()
        self.GR4J_X2 = list()
        self.GR4J_X3 = list()

    @staticmethod
    def soil_param_type():
        return ParamAbstractionTypes.CONCEPTUAL
