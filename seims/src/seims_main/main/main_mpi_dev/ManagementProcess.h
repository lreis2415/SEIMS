#ifndef SEIMS_MPI_MANAGEMENT_PROCESS_H
#define SEIMS_MPI_MANAGEMENT_PROCESS_H
#include "invoke.h"

/*!
 * \brief Management process.
 *        Read reach topology data and scatter to each ranks
 * \param[in] input_args Input arguments, \sa InputArgs
 * \param[in] size Number of process
 * \param[in] max_task_len Maximum tasks number of all groups
 * \param group_ids
 * \param subbasin_ids
 * \param lyr_ids
 * \param downstream_ids
 * \param upstream_count
 * \param upstream_ids
 * \return 0 for success
 */
int ManagementProcess(InputArgs* input_args, int size, int& max_task_len, int*& group_ids,
                      int*& subbasin_ids, int*& lyr_ids, int*& downstream_ids,
                      int*& upstream_count, int*& upstream_ids);

#endif /* SEIMS_MPI_MANAGEMENT_PROCESS_H */
