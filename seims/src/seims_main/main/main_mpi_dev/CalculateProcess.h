#ifndef SEIMS_MPI_CALCULATE_PROCESS_H
#define SEIMS_MPI_CALCULATE_PROCESS_H
#include "parallel.h"

/*!
 * \brief Calculation process
 * \param world_rank Rank number
 * \param numprocs Number of all processors, including one management rank and N-1 slave ranks
 * \param input_args Input arguments, \sa InputArgs
 */
void CalculateProcess(int world_rank, int numprocs, InputArgs* input_args);

#endif /* SEIMS_MPI_CALCULATE_PROCESS_H */
