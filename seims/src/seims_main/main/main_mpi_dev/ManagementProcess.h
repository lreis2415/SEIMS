#ifndef SEIMS_MPI_MANAGEMENT_PROCESS_H
#define SEIMS_MPI_MANAGEMENT_PROCESS_H
#include "invoke.h"

/*!
 * \brief Management process.
 *        Read reach topology data and scatter to each ranks
 * \param input_args Input arguments, \sa InputArgs
 * \return 0 for success
 */
int ManagementProcess(InputArgs* input_args);

#endif /* SEIMS_MPI_MANAGEMENT_PROCESS_H */
