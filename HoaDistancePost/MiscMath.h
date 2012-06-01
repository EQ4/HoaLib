/*
 *  MiscMath.h
 *  signalVec
 *
 *  Created by julien colafrancesco on 15/01/12.
 *  Copyright 2012 Ircam. All rights reserved.
 *
 */

#include "math.h"

inline int factoriel (int n)
{
	int facto, i;
	facto=1;
	i=1;
	
	while (i<=n)
	{
		facto=facto*i;
		i=i+1;
	}
	return facto;
}

inline long nextPow2(long n)
{
	
	n--;
	n |= n >> 1;   // Divide by 2^k for consecutive doublings of k up to 32,
	n |= n >> 2;   // and then or the results.
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
	
	
}