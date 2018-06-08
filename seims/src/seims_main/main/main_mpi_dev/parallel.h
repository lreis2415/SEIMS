/*!
 * \brief Header of MPI version of SEIMS framework
 * \author Junzhi Liu, Liangjun Zhu
 * \changelog 2018-05-31 - lj - Separate the original header to headers by functionality.\n
 */
#ifndef SEIMS_MPI_PARALLEL_BASIC_H
#define SEIMS_MPI_PARALLEL_BASIC_H

#ifdef MSVC
// Ignore warning on Windows MSVC compiler caused by MPI.
#pragma warning(disable: 4819)
#endif /* MSVC */
#include "mpi.h"

#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1 ///< Rank of this slave processor in SlaveGroup is 0
#define MAX_UPSTREAM 4
#define MSG_LEN 5
#define MCW MPI_COMM_WORLD

#endif /* SEIMS_MPI_PARALLEL_BASIC_H */
