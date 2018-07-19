/*!
 * \brief Perform calculation on each rank.
 * \author Liangjun Zhu
 * \changelog  2018-06-12  - lj -  Initial implementation.\n
 */
#ifndef SEIMS_MPI_CALCULATE_PROCESS_H
#define SEIMS_MPI_CALCULATE_PROCESS_H

#include "invoke.h"

/*!
 * \brief Calculation process
 * \param input_args Input arguments, \sa InputArgs
 * \param rank Rank number
 * \param size Number of all processors, including one management rank and N-1 slave ranks
 */
void CalculateProcess(InputArgs* input_args, int rank, int size);

#endif /* SEIMS_MPI_CALCULATE_PROCESS_H */
