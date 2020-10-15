/*
 * RAMIntegrity.h
 *
 *  Created on: Oct 15, 2020
 *      Author: BFS
 */

#ifndef RAMINTEGRITY_H_
#define RAMINTEGRITY_H_

#define VOLATILE_DEREF(type, variable) (*((volatile type *) variable))

bool RAMIntegrity_Process(void);


#endif /* RAMINTEGRITY_H_ */
