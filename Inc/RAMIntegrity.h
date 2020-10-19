/*
 * RAMIntegrity.h
 *
 *  Created on: Oct 15, 2020
 *      Author: BFS
 */

#ifndef RAMINTEGRITY_H_
#define RAMINTEGRITY_H_

#define VOLATILE_DEREF(type, variable) (*((volatile type *) variable))

void RAMIntegrity_Process(void);
bool RAMIntegrity_StartupTasksComplete(void);

#endif /* RAMINTEGRITY_H_ */
