/*
 * Private include files and interfaces for the WebAuth library.
 *
 * Written by Roland Schemers
 * Copyright 2002, 2009 Board of Trustees, Leland Stanford Jr. University
 *
 * See LICENSE for licensing terms.
 */

#ifndef _WEBAUTHP_H
#define _WEBAUTHP_H

#include "config.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif

#include <assert.h>

#include <lib/webauth.h>

#endif /* !_WEBAUTHP_H */