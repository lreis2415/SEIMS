/*!
 * \file LoadParallelTasks.h
 * \brief Load parallel task scheduing information.
 *
 * Changelog:
 *   - 1. 2018-06-12  - lj -  Initial implementation.
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_MPI_LOAD_TASK_H
#define SEIMS_MPI_LOAD_TASK_H

#include "invoke.h"
#include "TaskInformation.h"

/*!
 * \brief Management process.
 *        Read reach topology data and scatter to each ranks
 * \ingroup seims_mpi
 * \param[in] mclient MongoDB client
 * \param[in] input_args Input arguments
 * \param[in] size Number of process
 * \param[out] task Task information
 * \return 0 for success
 */
int ManagementProcess(MongoClient* mclient, InputArgs* input_args, int size, TaskInfo* task);

/*!
 * \brief Read reach topology data by master rank and scatter to each ranks.
 * \ingroup seims_mpi
 * \param[in] client MongoDB client
 * \param[in] input_args Input arguments
 * \param[in] size Number of process
 * \param[in] rank Process ID
 * \param[out] task Task information
 * \return 0 for success
 */
int LoadTasks(MongoClient* client, InputArgs* input_args, int size, int rank, TaskInfo* task);

#endif /* SEIMS_MPI_LOAD_TASK_H */
