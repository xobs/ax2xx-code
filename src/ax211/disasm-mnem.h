/* global.h
 *
 * Data shared by all modules
 *
 * Copyright 2001, 2002 by David Sullins
 *
 * This file is part of Dis51.
 * 
 * Dis51 is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 2 of the License.
 * 
 * Dis51 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * Dis51; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 * You may contact the author at davesullins@earthlink.net.
 */

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* global data */
extern int Lst;
extern unsigned short lbl[65536];
extern const char mnemonic[256][20];
extern const char op_format[256];
extern const char sfrname[128][5];
extern const char sfbitname[128][6];

#endif /* _GLOBAL_H_ */
