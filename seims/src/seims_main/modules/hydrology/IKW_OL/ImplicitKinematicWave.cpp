#include "ImplicitKinematicWave.h"
#include "ImplicitKinematicWave.h"
#include "text.h"

// using namespace std;  // Avoid this statement! by lj.

ImplicitKinematicWave_OL::ImplicitKinematicWave_OL(void) : m_nCells(-1), m_CellWidth(-1.0f),
                                                           m_s0(NULL), m_n(NULL), m_flowInIndex(NULL), m_flowInFrac(NULL),
                                                           m_flowOutIdx(NULL),m_flowOutFrac(NULL), m_direction(NULL),
                                                           m_routingLayers(NULL), m_nLayers(-1),
                                                           m_q(NULL), m_sr(NULL), m_flowWidth(NULL), m_flowLen(NULL),
                                                           m_alpha(NULL),  m_streamLink(NULL),
                                                           m_sRadian(NULL), m_vel(NULL), m_reInfil(NULL),
                                                           m_idOutlet(-1),
                                                           m_infilCapacitySurplus(NULL), m_accumuDepth(NULL),
                                                           m_infil(NULL), m_dtStorm(-1.0f),m_dem(NULL) {
}

ImplicitKinematicWave_OL::~ImplicitKinematicWave_OL(void) {
    if (m_q != NULL) Release2DArray(m_q);
    if (m_flowWidth != NULL) Release2DArray(m_flowWidth);
    if (m_flowLen != NULL) Release2DArray(m_flowLen);
    if (m_alpha != NULL) Release2DArray(m_alpha);
    if (m_sRadian != NULL) Release2DArray(m_sRadian);
    if (m_vel != NULL) Release2DArray(m_vel);
    if (m_reInfil != NULL) Release1DArray(m_reInfil);
    if (m_s0 != NULL) Release2DArray(m_s0);
}

bool ImplicitKinematicWave_OL::CheckInputData(void) {
    if (m_date <= 0) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "You have not set the Date variable.");
    }

    if (m_nCells <= 0) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The cell number of the input can not be less than zero.");
    }

    if (m_dtStorm <= 0) {
        throw ModelException(M_IKW_OL[0], "CheckInputData",
                             "You have not set the TimeStep variable of the overland flow routing.");
    }

    if (m_CellWidth <= 0) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "You have not set the CellWidth variable.");
    }

    if (m_accumuDepth == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The m_accumuDepth has not been set.");
    }
    //if (m_s0 == NULL) {
    //    throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: slope has not been set.");
    //}
    if (m_n == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: manning's roughness has not been set.");
    }
    if (m_flowInIndex == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: flow in index has not been set.");
    }
    if (m_flowInFrac == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: flow in fraction has not been set.");
    }
    if (m_flowOutIdx == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: flow out index has not been set.");
    }
    if (m_flowOutFrac == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: flow out fraction has not been set.");
    }
    if (m_direction == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: flow direction has not been set.");
    }
    if (m_routingLayers == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: routingLayers has not been set.");
    }
    if (m_streamLink == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: Stream_link has not been set.");
    }
    if (m_sr == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: D_SURU(surface runoff) has not been set.");
    }
    if (m_dem == NULL) {
        throw ModelException(M_IKW_OL[0], "CheckInputData", "The parameter: m_dem has not been set.");
    }

    return true;
}

void ImplicitKinematicWave_OL:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_IKW_OL[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (m_q == NULL) {
        CheckInputData();
        m_q = new float*[m_nCells];
        m_sRadian = new float*[m_nCells];
        m_vel = new float*[m_nCells];
        m_flowWidth = new float*[m_nCells];
        m_flowLen = new float*[m_nCells];
        m_alpha = new float*[m_nCells];
        //m_alpha_avg = new float[m_nCells];
        m_reInfil = new float[m_nCells];
        m_s0 = new float*[m_nCells];
//#pragma omp parallel for
        for (int i = 0; i < m_nCells; ++i) {
            int numOutflows = m_flowOutIdx[i][0];

            m_q[i] = new float[numOutflows + 1];
            m_sRadian[i] = new float[numOutflows + 1];
            m_flowLen[i] = new float[numOutflows + 1];
            m_flowWidth[i] = new float[numOutflows + 1];
            m_alpha[i] = new float[numOutflows + 1];
            m_vel[i] = new float[numOutflows + 1];
            m_s0[i] = new float[numOutflows + 1];

            //m_q[i] = 0.0f;
            m_reInfil[i] = 0.f;

            // flow width

            //
            for (int j = 1; j <= numOutflows; ++j) {
                int dir = m_direction[i][j];
                m_flowWidth[i][j] = m_CellWidth;


                //if ((int) m_diagonal[dir] == 1) {
                if (DiagonalCCW[dir] == 1) {
                    m_flowWidth[i][j] = m_CellWidth / SQ2;
                }
                if (m_streamLink[i] > 0) {
                    m_flowWidth[i][j] -= m_chWidth[i];
                }
            }

 

            //calculate slope from DEM
            for (int j = 1; j <= numOutflows; ++j) {
                int nextCell = m_flowOutIdx[i][j];
                float s0 = 0.0f;

                if (m_dem[i] <= m_dem[nextCell]) {
                    s0 = MIN_SLOPE;
                }

                float deltaZ = m_dem[i] - m_dem[nextCell];

                float horizontalDist = m_CellWidth;
                int dir = m_direction[i][j];
                if (DiagonalCCW[dir] == 1) {
                    horizontalDist = m_CellWidth * SQ2;
                }

                if (horizontalDist > 0) {
                    s0 = deltaZ / horizontalDist;
                }

                if (FloatEqual(s0, 0.0f)) {
                    s0 = MIN_SLOPE;
                }

                m_s0[i][j] = s0;
                m_sRadian[i][j] = atan(s0);


            }


            //float s0 = m_s0[i];
            //if (FloatEqual(s0, 0.0f)) {
            //    s0 = MINI_SLOPE;
            //}
            //m_sRadian[i] = atan(s0);

            // flow length needs to be corrected by slope angle
            for (int j = 1; j <= numOutflows; ++j) {
                int dir = m_flowOutIdx[i][j];
                float dx = m_CellWidth / cos(m_sRadian[i][j]);
                //if ((int) m_diagonal[dir] == 1) {
                if (DiagonalCCW[dir] == 1) {
                    dx = SQ2 * dx;
                }
                m_flowLen[i][j] = dx;
            }
            
        }
    }
}


//---------------------------------------------------------------------------
// modified from OpenLISEM
/** Newton Rapson iteration for new water flux in cell, based on Ven Te Chow 1987
\param qIn      summed Q new from upstream
\param qLast    current discharge in the cell
\param surplus        infiltration surplus flux (in m2/s), has value <= 0
\param alpha    alpha calculated in LISEM from before kinematic wave
\param dt   timestep
\param dx   length of the cell corrected for slope
*/
float ImplicitKinematicWave_OL::GetNewQ(float qIn, float qLast, float surplus, float alpha, float dt, float dx) {
    /* Using Newton-Raphson Method */
    float ab_pQ, dtX, C;  //auxillary vars
    int count;
    float Qkx; //iterated discharge, becomes Qnew
    float fQkx; //function
    float dfQkx;  //derivative
    const float _epsilon = 1e-12f;
    const float beta = 0.6f;

    /* if no input then output = 0 */
    if ((qIn + qLast) <= -surplus * dx) //0)
    {
        //itercount = -1;
        return (0);
    }

    /* common terms */
    ab_pQ = alpha * beta * CalPow(((qLast + qIn) / 2), beta - 1);
    // derivative of diagonal average (space-time)

    dtX = dt / dx;
    C = dtX * qIn + alpha * CalPow(qLast, beta) + dt * surplus;
    //dt/dx*Q = m3/s*s/m=m2; a*Q^b = A = m2; surplus*dt = s*m2/s = m2
    //C is unit volume of water
    // first gues Qkx
    Qkx = (dtX * qIn + qLast * ab_pQ + dt * surplus) / (dtX + ab_pQ);

    // VJ 050704, 060830 infil so big all flux is gone
    //VJ 110114 without this de iteration cannot be solved for very small values
    if (Qkx < MIN_FLUX) {
        //itercount = -2;
        return (0);
    }

    Qkx = MAX(Qkx, MIN_FLUX);

    count = 0;
    do {
        fQkx = dtX * Qkx + alpha * Power(Qkx, beta) - C;   /* Current k */
        dfQkx = dtX + alpha * beta * Power(Qkx, beta - 1);  /* Current k */
        Qkx -= fQkx / dfQkx;                                /* Next k */
        Qkx = Max(Qkx, MIN_FLUX);
        count++;
        //qDebug() << count << fQkx << Qkx;
    } while (Abs(fQkx) > _epsilon && count < MAX_ITERS_KW);

    if (Qkx != Qkx) {
        throw ModelException(M_IKW_OL[0], "GetNewQ", "Error in iteration!");
    }

    //itercount = count;
    return Qkx;
}

// end code form LISEM

void ImplicitKinematicWave_OL::OverlandFlow(int id) {
    const float beta = 0.6f;
    float beta1 = 1.0f / beta;
    float h = m_sr[id] / 1000.f;

    ////debug
    //const int DEBUG_ID = 4197; 
    //bool isDebugCell = (id == DEBUG_ID);
    //if (isDebugCell) {
    //    std::cout << std::fixed << std::setprecision(8);
    //}

    int numOutflows = m_flowOutIdx[id][0];
    // calculate  weighted average alpha and flow length for the cell

    ////debug
    //if (isDebugCell) {
    //    std::cout << "--- DEBUG ID: " << id << " (OverlandFlow Start) ---" << std::endl;
    //    std::cout << "  Initial h (m): " << h << std::endl;
    //    std::cout << "  Initial m_sr[id] (mm): " << m_sr[id] << std::endl;
    //    std::cout << "  NumOutflows: " << numOutflows << std::endl;
    //}


    for (int j = 1; j <= numOutflows; ++j) {
        float Perim_j = 2.f * h + m_flowWidth[id][j];
        float r_j = 0;
        if (Perim_j > 0) {
            r_j = h * m_flowWidth[id][j] / Perim_j;
        }

        float sSin_j = CalSqrt(sin(m_sRadian[id][j]));
        m_alpha[id][j] = (sSin_j > 0) ? CalPow(m_n[id] / sSin_j * CalPow(Perim_j, _2div3), beta):0.f;

        ////debug
        //if (isDebugCell) {
        //    std::cout << "  Loop 1 (j=" << j << "): m_alpha = " << m_alpha[id][j] << std::endl;
        //}

    }


    //m_vel[id] = CalPow(r, _2div3) * sSin / m_n[id];

    //float flowWidth = m_flowWidth[id]; //(little question: why not use m_flowWidth[[id]? by Gao)
    //float flowLen = m_flowLen[id];

    //sum the upstream overland flow
    float qUp = 0.0f;
    for (int k = 1; k <= m_flowInIndex[id][0]; ++k) {
        int flowInID = m_flowInIndex[id][k];
        if (m_streamLink[flowInID] <= 0) { // if the upstream cell is not a channel cell
            int numDownstreamofUpstream = m_flowOutIdx[flowInID][0];
            if (numDownstreamofUpstream >= 1) {
                for (int j = 1; j <= numDownstreamofUpstream; ++j) {
                    if (m_flowOutIdx[flowInID][j] == id) {
                        qUp += m_q[flowInID][j];
                        //if (isDebugCell) {
                        //    std::cout << flowInID <<"  m_q[flowInID][j]: " << m_q[flowInID][j] << std::endl;
                        //}
                    }
                }
            }
        }
    }

    ////debug
    //if (id == 4290 && qUp < 0) {
    //    std::cout << id <<":  qUp (m3/s): " << qUp << std::endl;
    //}
    //if (isDebugCell) {
    //    std::cout << "  qUp (m3/s): " << qUp << std::endl;
    //}

    // if the channel width is greater than the cell width
    if (m_streamLink[id] >= 0 && m_flowWidth[id] <= 0) {
        for (int j = 1; j <= numOutflows; ++j) {
            m_q[id][j] = 0.f;
        }
        m_q[id][0] = qUp;
        m_sr[id] = 0.f;
        return;
    }

    // check whether overland flow routing is needed
    if (qUp < MIN_FLUX && h < MIN_DEPTH) {
        m_sr[id] = 0.f;
        m_q[id][0] = 0.f;
        for (int j = 1; j <= numOutflows; ++j) {
            m_q[id][j] = 0.f;
        }
        if (m_reInfil != NULL) m_reInfil[id] = 0.f;
        return;
    }

    //// calcluate infiltration surplus (m2/s)
    //float surplus = 0.f;
    //if (m_infilCapacitySurplus != NULL) {
    //    surplus = -m_infilCapacitySurplus[id] / 1000.f * m_flowWidth[id] / m_dtStorm;
    //}

    //float flowLen_avg = 0.f;
    //for (int j = 1; j <= numOutflows; ++j) {
    //    flowLen_avg += m_flowLen[id][j] * m_flowOutFrac[id][j];
    //}



    float qNewTotal = 0.f;
    float totalFinalVolume = 0.f;
    float totalLeftoverVolume = 0.f;
    //float totalInflowVolume = 0.f;


    float cellArea = m_CellWidth * m_CellWidth;


    float initialVolume = h * cellArea;

    float potentialInfilVol = 0.f;
    if (m_infilCapacitySurplus != NULL && m_infilCapacitySurplus[id] > 0) {
        potentialInfilVol = m_infilCapacitySurplus[id] / 1000.f * cellArea;
    }

    ////debug
    //if (isDebugCell) {
    //    std::cout << "  initialVolume (m3): " << initialVolume << std::endl;
    //}

    for (int j = 1; j <= numOutflows; ++j) {
        float qUp_j = qUp * m_flowOutFrac[id][j];

        // calcluate infiltration surplus (m2/s)
        float surplus = 0.f;
        if (m_infilCapacitySurplus != NULL) {
            //if (isDebugCell) {
            //    std::cout << "  m_infilCapacitySurplus: " << m_infilCapacitySurplus[id] << std::endl;
            //}
            float validCapacity = (m_infilCapacitySurplus[id] > 0.f) ? m_infilCapacitySurplus[id] : 0.f;
            surplus = -validCapacity / 1000.f * m_flowWidth[id][j] / m_dtStorm;
        }

        float surplus_j = surplus * m_flowOutFrac[id][j];


        //calculate total potential outflow qIn based on Manning's equation using average alpha
        float qLast_j = 0.f;
        if (m_alpha[id][j] > 0) {
            qLast_j = CalPow((m_flowWidth[id][j] * h) / m_alpha[id][j], beta1);
        }
        else {
            qLast_j = 0;
        }
        float inflowVolume_j = qUp_j * m_dtStorm;
        //float surplusVolume_j = surplus_j * m_flowLen[id][j] * m_dtStorm;
        float initialVolume_j = initialVolume * m_flowOutFrac[id][j];
        float allocatedVolume_j = initialVolume_j + inflowVolume_j;

        m_q[id][j] = GetNewQ(qUp_j, qLast_j, surplus_j, m_alpha[id][j], m_dtStorm, m_flowLen[id][j]);
        //if (isDebugCell) {
        //    std::cout << "  m_q[id][j]: " << m_q[id][j] << std::endl;
        //}
        float actualOutflowVolume_j = m_q[id][j] * m_dtStorm;
        float leftoverVolume_j = allocatedVolume_j - actualOutflowVolume_j;
        if (actualOutflowVolume_j > allocatedVolume_j)
        {
            m_q[id][j] = allocatedVolume_j / m_dtStorm;
            actualOutflowVolume_j = allocatedVolume_j;
            leftoverVolume_j = 0;
        }

        ////debug
        //if (isDebugCell) {
        //    std::cout << "  Loop 2 (j=" << j << "):" << std::endl;
        //    std::cout << "    qUp_j: " << qUp_j << ", qLast_j: " << qLast_j << ", surplus_j: " << surplus_j << std::endl;
        //    std::cout << "    allocatedVolume_j: " << allocatedVolume_j << std::endl;
        //    std::cout << "    m_q[id][j] (outflow): " << m_q[id][j] << std::endl;
        //    std::cout << "    actualOutflowVolume_j: " << actualOutflowVolume_j << std::endl;
        //}
        //if (isDebugCell) {
        //    std::cout << "    leftoverVolume_j: " << leftoverVolume_j << std::endl;
        //}
        

        qNewTotal += m_q[id][j];

        totalLeftoverVolume += leftoverVolume_j;
        //totalInflowVolume += inflowVolume_j + surplusVolume_j;

    }

    m_q[id][0] = qNewTotal;

    // re-infiltraction
    float reInfilVol = 0.f;
    if (potentialInfilVol > 0 && totalLeftoverVolume > 0) {
        if (totalLeftoverVolume >= potentialInfilVol) {
            reInfilVol = potentialInfilVol;
            totalLeftoverVolume -= potentialInfilVol;
        }
        else {
            reInfilVol = totalLeftoverVolume;
            totalLeftoverVolume = 0.f;
        }
    }

    if (totalLeftoverVolume < 0.f) totalLeftoverVolume = 0.f;

    float hNew = (cellArea > 0) ? (totalLeftoverVolume / cellArea) : 0.f;
    m_sr[id] = hNew * 1000.f;

    float totalOutflowVolume = qNewTotal * m_dtStorm;


    float reInfil = (cellArea > 0) ? (reInfilVol / cellArea * 1000.f) : 0.f;

    ////debug
    //if (isDebugCell) {
    //    std::cout << "  --- DEBUG ID: " << id << " (OverlandFlow End) ---" << std::endl;
    //    std::cout << "  qNewTotal (m3/s): " << qNewTotal << std::endl;
    //    std::cout << "  totalLeftoverVolume (m3): " << totalLeftoverVolume << std::endl;
    //    std::cout << "  hNew (m): " << hNew << std::endl;
    //    std::cout << "  New m_sr[id] (mm): " << m_sr[id] << std::endl;
    //    std::cout << "  reInfilVol : " << reInfilVol << std::endl;
    //    std::cout << "----------------------------------------------" << std::endl;
    //}

    //float hNew = (m_alpha_avg[id] > 0) ? (m_alpha_avg[id] * CalPow(m_q[id][0], 0.6f)) / m_flowWidth[id] : 0.f; // unit m
    ////float hTest = h + (qUp - m_q[id])*m_dtStorm/(flowWidth*flowLen);
    //m_sr[id] = hNew * 1000.f;

    //float reInfil = (qUp - m_q[id][0]) * m_dtStorm / (flowWidth * flowLen_avg) + h - hNew;
    //reInfil *= 1000.f;

    //if (abs(reInfil) < 0.001f)
    //	reInfil = 0.f;
    //else if(abs(reInfil - m_infilCapacitySurplus[id]) < 0.001f)
    //	reInfil = m_infilCapacitySurplus[id];

    //if (reInfil < 0.f || reInfil > m_infilCapacitySurplus[id])
    //	throw  ModelException(M_IKW_OL[0], "OverlandFlow", "Reinfiltration exceeded range!");

    if (reInfil < 0.f) {
        reInfil = 0.f;
    }

    m_infil[id] += reInfil;
    if (m_accumuDepth != NULL) {
        m_accumuDepth[id] += reInfil;
    }

    if (m_infilCapacitySurplus != NULL) {
        m_infilCapacitySurplus[id] -= reInfil;
        if (m_infilCapacitySurplus[id] < 0) m_infilCapacitySurplus[id] = 0.f;
    }

    m_reInfil[id] = reInfil;

    ////debug
    //if (isDebugCell) {
    //    std::cout << "  --- DEBUG ID: " << id << " (OverlandFlow End) ---" << std::endl;
    //    std::cout << "  reInfil : " << reInfil << std::endl;
    //    std::cout << "----------------------------------------------" << std::endl;
    //}    ////debug
    //if (isDebugCell) {
    //    std::cout << "  --- DEBUG ID: " << id << " (OverlandFlow End) ---" << std::endl;
    //    std::cout << "  reInfil : " << reInfil << std::endl;
    //    std::cout << "----------------------------------------------" << std::endl;
    //}

    // compute to channel flow
    // In this modification, the hillslope routing module does not consider channel flow. (by Fan xinyi)
    //if (m_streamLink[id] > 0) {
    //    float fractiontochannel = Min(m_dtStorm * m_vel[id] / (0.5f * flowWidth), 1.0f);
    //    float Volume = m_sr[id] / 1000.f * m_flowWidth[id] * flowLen;

    //    //if (id == m_idOutlet)// in catchment outlet cell, throw everything in channel
    //    fractiontochannel = 1.0f;

    //    m_q[id] += fractiontochannel * Volume / m_dtStorm; // water diverted to the channel
    //    m_sr[id] *= (1.f - fractiontochannel);

    //}

}

int ImplicitKinematicWave_OL::Execute() {
    InitialOutputs();
    /*std::cout << "    m_date " << m_date << std::endl;*/
    for (int iLayer = 0; iLayer < m_nLayers; ++iLayer) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nCells = (int) m_routingLayers[iLayer][0];
        //SetOpenMPThread(2);
//#pragma omp parallel for
        for (int iCell = 1; iCell <= nCells; ++iCell) {
            int id = (int) m_routingLayers[iLayer][iCell];
            OverlandFlow(id);
        }
    }

    return 0;
}

bool ImplicitKinematicWave_OL::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        //StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            std::ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n <<
                    ". The origin size is " << m_nCells << ".\n";
            throw ModelException(M_IKW_OL[0], "CheckInputSize", oss.str());
        }
    }

    return true;
}

void ImplicitKinematicWave_OL::SetValue(const char *key, FLTPT data) {
    string sk(key);
    if  (StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = data;
    }else {
        throw ModelException(M_IKW_OL[0], "SetSingleData", "Parameter " + sk
                             + " does not exist.");
    }

}

void ImplicitKinematicWave_OL::SetValue(const char* key, int data) {
    string sk(key);
    if (m_stormMode && StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dtStorm = data;
    }
    else if (m_stormMode && StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = data;
    }
    else {
        throw ModelException(M_IKW_OL[0], "SetSingleData", "Parameter " + sk
            + " does not exist.");
    }

}

void ImplicitKinematicWave_OL::Set1DData(const char *key, int n, FLTPT *data) {
    //check the input data
    CheckInputSize(key, n);
    string sk(key);
/*    if (StringMatch(sk, VAR_SLOPE[0])) {
        m_s0 = data;
    } else */if (StringMatch(sk, VAR_MANNING[0])) {
        m_n = data;
    } else if (StringMatch(sk, VAR_SURU[0])) {
        m_sr = data;
    } else if (StringMatch(sk, VAR_INFILCAPSURPLUS[0])) {
        m_infilCapacitySurplus = data;
    } else if (StringMatch(sk, VAR_INFIL[0])) {
        m_infil = data;
    } else if (StringMatch(sk, VAR_ACC_INFIL[0])) {
        m_accumuDepth = data;
    } else if (StringMatch(sk, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else if (StringMatch(sk, VAR_DEM[0])) {
        m_dem = data;
    }
    else {
        throw ModelException(M_IKW_OL[0], "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }

}

void ImplicitKinematicWave_OL::Set1DData(const char* key, int n, int* data) {
    //check the input data
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_STREAM_LINK[0])) {
     m_streamLink = data;
    }

    else {
    throw ModelException(M_IKW_OL[0], "Set1DData", "Parameter " + sk
        + " does not exist. Please contact the module developer.");
    }

}

void ImplicitKinematicWave_OL::GetValue(const char *key, float *data) {
    string sk(key);
    if (StringMatch(sk, VAR_ID_OUTLET[0])) {
        *data = (float) m_idOutlet;
    } else {
        throw ModelException(M_IKW_OL[0], "GetValue", "Output " + sk
                             + " does not exist.");
    }

}

void ImplicitKinematicWave_OL::Get1DData(const char *key, int *n, float **data) {
    InitialOutputs();

    string sk(key);
    *n = m_nCells;
    /*if (StringMatch(sk, VAR_QOVERLAND[0])) {
        *data = m_q;
    } else */if (StringMatch(sk, VAR_Reinfiltration[0])) {
        *data = m_reInfil;
    }/* else if (StringMatch(sk, VAR_RadianSlope[0])) {
        *data = m_sRadian;
    } */else if (StringMatch(sk, "ChWidth")) {   //FlowLen   TODO WHY TO DO SO?
        *data = m_chWidth;                 //add by Wu Hui
    } else {
        throw ModelException(M_IKW_OL[0], "Get1DData", "Output " + sk
                             + " does not exist.");
    }
}

void ImplicitKinematicWave_OL::Get2DData(const char* key, int* nrows, int* ncols, float*** data) {
    string sk(key);
    *nrows = m_nCells; //uncertain!!!
    if (StringMatch(sk, VAR_QOVERLAND[0])) {
        *data = m_q;
    }
    else if (StringMatch(sk, VAR_RadianSlope[0])) {
        *data = m_sRadian;
    }
    else if (StringMatch(sk, VAR_FLOWWIDTH[0])) {
        *data = m_flowWidth;
    }
    else {
        throw ModelException(M_IKW_OL[0], "Get2DData",
            "Output " + sk + " does not exist.");
    }

}



void ImplicitKinematicWave_OL::Set2DData(const char *key, int nrows, int ncols, FLTPT **data) {
    //check the input data
    //m_nLayers = nrows;
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_FRACTION[0])) {
        m_flowInFrac = data;
    }
        else if (StringMatch(sk, Tag_FLOWOUT_FRACTION[0])) {
        m_flowOutFrac = data;
    }

    /*if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_routingLayers = data;
        m_nLayers = nrows;
    } else if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIndex = data;
    } else {
        throw ModelException(M_IKW_OL[0], "Set2DData", "Parameter " + sk
                             + " does not exist.");
    }*/
}

void ImplicitKinematicWave_OL::Set2DData(const char* key, int nrows, int ncols, int** data) {
    //check the input data
    //m_nLayers = nrows;
    string sk(key);
    if (StringMatch(sk, Tag_ROUTING_LAYERS[0])) {
        m_routingLayers = data;
        m_nLayers = nrows;
    }
    else if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIndex = data;
    }
    else if (StringMatch(sk, VAR_FLOWOUT_DIRADJ[0])) {
        m_direction = data;
    }
    else if (StringMatch(sk, Tag_FLOWOUT_INDEX[0])) {
        m_flowOutIdx = data;
        for (int i = 0; i < m_nCells; i++) {
            if (m_flowOutIdx[i][0] == 0 && m_flowOutIdx[i][1] < 0) {

                m_idOutlet = i;
                if (m_idOutlet < 0)
                {
                    throw ModelException(M_CH_DW[0], "Set2DData",
                        "m_idOutlet does not exist.");
                }
                break;
            }
        }
    }//
    else {
        throw ModelException(M_IKW_OL[0], "Set2DData", "Parameter " + sk
            + " does not exist.");
    }
}
