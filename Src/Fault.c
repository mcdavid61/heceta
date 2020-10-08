/*
 * Fault.c
 *
 *  Created on: Oct 6, 2020
 *      Author: BFS
 */

#include <stdint.h>
#include <stdbool.h>
#include "Fault.h"

static uint16_t m_nFault = 0;

/*
	Function:	Fault_Set
	Description:
		Activates or clears a fault as specified, based on a boolean.
*/
void Fault_Set(Fault_T eFault, bool bActive)
{
	if (bActive)
	{
		Fault_Activate(eFault);
	}
	else
	{
		Fault_Clear(eFault);
	}
}

/*
	Function:	Fault_Activate
	Description:
		Activates a fault as specified.
*/
void Fault_Activate(Fault_T eFault)
{
	m_nFault |= (1 << eFault);
}
/*
	Function:	Fault_Clear
	Description:
		Clears a fault as specified.
*/
void Fault_Clear(Fault_T eFault)
{
	m_nFault &= ~(1 << eFault);
}

bool Fault_Get(Fault_T eFault)
{
	return !!(m_nFault & (1 << eFault));
}
uint16_t Fault_GetAll()
{
	return m_nFault;
}
/*
	Function:	Fault_OK
	Description:
		Returns the overall fault state of the system.
		If there are any faults that are not okay,
*/
bool Fault_OK(void)
{
	return !m_nFault;
}



