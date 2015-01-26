/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */
 
/** 
* @file message_allocator.h
* @brief Header files for utility functions
* @author Andrea Montefusco IW0HDV
* @version 0.0
* @date 2013-09-23
*/

#if !defined	__MESSAGE_ALLOCATOR_H__
#define			__MESSAGE_ALLOCATOR_H__


struct MsgAllocatorImpl;

/**
 * class MsgAllocator
 *
 * manages intra window custom messages
 */
 
class MsgAllocator {
public:
	MsgAllocator();
	~MsgAllocator();
	char * xstrdup(const char *);
	void xstrdel(const char *, int);
private:
	MsgAllocatorImpl *pi;
};

#endif	/* _UTIL_H */

