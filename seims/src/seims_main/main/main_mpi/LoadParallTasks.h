/*!
 * \brief Load parallel task scheduing information.
 * \author Liangjun Zhu
 * \changelog  2018-06-12  - lj -  Initial implementation.\n
 */
#ifndef SEIMS_MPI_LOAD_TASK_H
#define SEIMS_MPI_LOAD_TASK_H

#include "invoke.h"
#include "TaskInformation.h"

/*!
 * \brief Management process.
 *        Read reach topology data and scatter to each ranks
 * \param[in] mclient MongoDB client, \sa MongoClient
 * \param[in] input_args Input arguments, \sa InputArgs
 * \param[in] size Number of process
 * \param[out] task Task information, \sa TaskInfo
 * \return 0 for success
 */
int ManagementProcess(MongoClient* mclient, InputArgs* input_args, int size, TaskInfo* task);

/*!
 * \brief Read reach topology data by master rank and scatter to each ranks.
 * \param[in] client MongoDB client, \sa MongoClient
 * \param[in] input_args Input arguments, \sa InputArgs
 * \param[in] size Number of process
 * \param[in] rank Process ID
 * \param[out] task Task information, \sa TaskInfo
 * \return 0 for success
 */
int LoadTasks(MongoClient* client, InputArgs* input_args, int size, int rank, TaskInfo* task);

#endif /* SEIMS_MPI_LOAD_TASK_H */
