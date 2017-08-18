#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base classes of user defined tools for NSAG-II.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""

from scenario import *
import uuid
from math import *

# Model
def __uniqueid__():
    id = int(str(uuid.uuid4().fields[-1])[:8])
    while True:
        yield id
        id += 1

def calBenefitandCost(individual):
    Sce = Scenario()

    # random.seed()
    # ms = float(random.randint(0, 1000))
    # time.sleep(ms / 1000.)

    #Sce.getIdfromMongo()
    Sce.setId(__uniqueid__().next())
    Sce.attributes = individual
    Sce.decoding()
    Sce.importoMongo(HOSTNAME, PORT, BMPScenarioDBName)
    # Calculate benefit and cost
    Sce.cost()
    Sce.benefit()
    # Save scenarios information in file
    Sce.saveInfo(scenariosInfo)
    f1 = Sce.cost_eco
    f2 = Sce.benefit_env
    return f1, f2

def test(individual):
    g = sum(individual) / len(individual)
    f1 = sum(individual)
    f2 = sin(g) * (1 - sqrt(f1))
    return f1, f2

###### Get all BMPs in each slope position ######
def posBMPsInfo(dataDir):
    ## Get all BMPs in each slope position
    SmtBMPs = PosBMPs(dataDir)[0]
    SmtBMPs.append(0)
    SlpBMPs = PosBMPs(dataDir)[1]
    SlpBMPs.append(0)
    VlyBMPs = PosBMPs(dataDir)[2]
    VlyBMPs.append(0)
    return (SmtBMPs, SlpBMPs, VlyBMPs)

####### Read Json file ######
def posFlowInfo(fileDir):
    jsonData = json.load(file(fileDir))
    file(fileDir).close
    return jsonData

###### position ID ######
def posID(jsondata):
    idArr = []
    for i in range(len(jsondata['info'])):
        idArr.append(int(jsondata['info'][i]['id']))
    return idArr

pos_BMPs = posBMPsInfo(MODEL_DIR)
BMPs = posFlowInfo(MODEL_DIR + os.sep + "BMPConfig" + os.sep + "BMPs.json")
Smt_Slp = posFlowInfo(MODEL_DIR + os.sep + "FlowDir" + os.sep + "Smt_Slp.json")
Slp_Vly = posFlowInfo(MODEL_DIR + os.sep + "FlowDir" + os.sep + "Slp_Vly.json")
Up_Down = posFlowInfo(MODEL_DIR + os.sep + "FlowDir" + os.sep + "Up_Down.json")
smtID = posID(Smt_Slp)
slpID = posID(Slp_Vly)

###############
## Crossover ##
###############
def crossOver_slppos(ind1, ind2):
    """
    :param ind1: The first individual participating in the crossover.
    :param ind2: The second individual participating in the crossover.
    :returns: A tuple of two individuals.
    This function uses the :func:`~random.randint` function from the
    python base :mod:`random` module.
    """
    size = min(len(ind1), len(ind2))
    cxpoint = random.randint(1, size - 1)
    ind1[cxpoint:], ind2[cxpoint:] = ind2[cxpoint:], ind1[cxpoint:]

    # Checkout BMPs relationship
    ind1 = CheckCrossover(ind1, BMPs, Smt_Slp, Slp_Vly, smtID, slpID)
    ind2 = CheckCrossover(ind2, BMPs, Smt_Slp, Slp_Vly, smtID, slpID)

    # outlet
    # ind1[450] = 0
    # ind2[450] = 0

    return ind1, ind2

###### Check the results of crossover ######
def CheckCrossover(ipop, BMPs, Smt_Slp, Slp_Vly, smtID, slpID):
    for i in range(len(ipop)):
        ## To judge "i" belongs to summit, slope or valley
        ## summit
        if i in smtID:
            continue
        ## slope
        elif i in slpID:
            c_value = BMPsconfRule(ipop[i], ipop, i, BMPs, Smt_Slp, 'slope')
            ipop[i] = c_value
        ## valley
        else:
            c_value = BMPsconfRule(ipop[i], ipop, i, BMPs, Slp_Vly, 'valley')
            ipop[i] = c_value
    return ipop


######################################
# GA Mutations                       #
######################################

# def mutModel(individual, indpb):
#     sceSize = farm_Num + point_cattle_Num + point_pig_Num + point_sewage_Num
#     field_index = farm_Num - 1
#     point_cattle_index = point_cattle_Num + field_index
#     point_pig_index = point_pig_Num + point_cattle_index
#     # point_sewage_index = point_sewage_Num + point_pig_index
#     if random.random() < indpb:
#         mpoint_num = int(len(individual) / 10)
#         if mpoint_num == 0:
#             mpoint_num = 1
#         for _ in range(mpoint_num):
#             mpoint = random.randint(0, sceSize - 1)
#             if mpoint <= field_index:
#                 individual[mpoint] = selectBMPatRandom(bmps_farm)
#             elif mpoint <= point_cattle_index:
#                 individual[mpoint] = selectBMPatRandom(bmps_cattle)
#             elif mpoint <= point_pig_index:
#                 individual[mpoint] = selectBMPatRandom(bmps_pig)
#             else:
#                 for i in range(len(bmps_sewage)):
#                     if individual[mpoint] in bmps_sewage[i]:
#                         b_s_index = i
#                         break
#                 individual[mpoint] = selectBMPatRandom(bmps_sewage[b_s_index])
#     return individual


def mutModel_slppos(individual, indpb):
    # mutation Gene number are 1/20 of individual length
    mut_Num = random.randint(1, int(len(individual) / 50))
    for m in range(mut_Num):
        individual = mutation(individual, indpb, pos_BMPs, BMPs, Smt_Slp, Slp_Vly, smtID, slpID)
    # outlet
    # individual[450] = 0
    return individual


def mutModel_random(individual, indpb):
    # mutation Gene number are 1/50 of individual length
    mut_Num = random.randint(1, int(len(individual) / 50))
    for m in range(mut_Num):
        individual = mutation_random(individual, indpb)
    return individual


##############
## Mutation ##
##############
def mutation(individual, pm, pos_BMPs, BMPs, Smt_Slp, Slp_Vly, smtID, slpID):
    # for i in range(len(individual)):
    if (random.random() < pm):
        mpoint = random.randint(0, len(individual) - 1)
        individual[mpoint] = RuleinMutation(individual, mpoint, pos_BMPs, BMPs, Smt_Slp, Slp_Vly, smtID, slpID)
        # print pop[i][mpoint]
    return individual


def mutation_random(individual, pm):
    # for i in range(len(individual)):
    if (random.random() < pm):
        mpoint = random.randint(0, len(individual) - 1)
        bmps_mut_target = bmps_areal_struct[:]
        bmps_mut_target.append(0)
        ind = random.randint(0, len(bmps_mut_target) - 1)
        individual[mpoint] = bmps_mut_target[ind]
    return individual


###### Rules in mutation #######
def RuleinMutation(ipop, m_index, pos_BMPs, BMPs, Smt_Slp, Slp_Vly, smtID, slpID):
    SmtBMPs = pos_BMPs[0]
    SlpBMPs = pos_BMPs[1]
    VlyBMPs = pos_BMPs[2]
    ## To judge it belongs to summit, slope or valley
    ## summit
    if m_index in smtID:
        m_value = SmtBMPs[random.randint(0, len(SmtBMPs) - 1)]
        return m_value
    ## slope
    elif m_index in slpID:
        m_value = SlpBMPs[random.randint(0, len(SlpBMPs) - 1)]
        if m_value != 0:
            m_value = BMPsconfRule(m_value, ipop, m_index, BMPs, Smt_Slp, 'slope')
            return m_value
        else:
            return 0
    ## valley
    else:
        m_value = VlyBMPs[random.randint(0, len(VlyBMPs) - 1)]
        if m_value != 0:
            m_value = BMPsconfRule(m_value, ipop, m_index, BMPs, Slp_Vly, 'valley')
            return m_value
        else:
            return 0


###### BMPs configure rules ######
def BMPsconfRule(m_value, ipop, m_index, BMPs, flowDir, pos):
    ## search upstream unit
    upSmtUnit = []
    for i in range(len(flowDir['info'])):
        if len(flowDir['info'][i][pos]) > 0:
            for ii in range(len(flowDir['info'][i][pos])):
                if flowDir['info'][i][pos][ii] == m_index:
                    upSmtUnit.append(flowDir['info'][i]['id'])
    # print upSmtUnit
    ## search incompat BMPs in upstream unit
    if len(upSmtUnit) > 0:
        incmtbmp = []
        for j in range(len(upSmtUnit)):
            for jj in range(len(BMPs['BMPs'])):
                if ipop[upSmtUnit[j]] == BMPs['BMPs'][jj]['id']:
                    if len(BMPs['BMPs'][jj]['incompat']) > 0:
                        incmtbmp.extend(BMPs['BMPs'][jj]['incompat'])
        # print int(m_value),incmtbmp
        if int(m_value) in incmtbmp:
            return 0
        else:
            return m_value
    else:
        return m_value


###################################################################
### Crossover and mutation (BMPs spatial units: field by Wuhui) ###
################################################################3##
def crossOver_fields(ind1, ind2):
    size = min(len(ind1), len(ind2))
    cxpoint = random.randint(1, size - 1)
    ind1[cxpoint:], ind2[cxpoint:] = ind2[cxpoint:], ind1[cxpoint:]
    # Checkout BMPs relationship
    # ind1 = check_pop_field(ind1)
    # ind2 = check_pop_field(ind2)
    return ind1, ind2

def mutation_fields(individual, indpb):
    if (random.random() < indpb):
        mut_Num = random.randint(1, int(len(individual) / 50))
        for _ in range(mut_Num):
            mpoint = random.randint(0, len(individual) - 1)
            bmps_mut_target_f = bmps_areal_struct[:]
            bmps_mut_target_f.append(0)
            mbmp = random.randint(0, len(bmps_mut_target_f) - 1)
            individual[mpoint] = bmps_mut_target_f[mbmp]
        # print pop[i][mpoint]
    # check_pop_field(individual)
    return individual

def check_pop_field(ind):
    fieldNum = len(ind)
    # print fieldNum
    for i in range(fieldNum):
        # fieldId = Up_Down['info'][i]['id']
        downslpIdList = Up_Down['info'][i]['downslope']
        if len(downslpIdList) > 0:
            for ds in downslpIdList:
                if ind[int(ds)] != 0:
                    ind[i] = 0
                    break
    return ind

# if __name__ == "__main__":
#     ind = [1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]
#     print check_pop_field(ind)
