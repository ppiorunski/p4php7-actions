/*
 * undefdups.h
 *
 *  Created on: 3 Sep 2009
 *      Author: sknop
 */

#ifndef UNDEFDUPS_H_
#define UNDEFDUPS_H_

#undef HAVE_GETHOSTNAME
#undef HAVE_UTIME
#undef HAVE_FORK
#undef HAVE_MMAP
#undef HAVE_UTIMES

#ifdef ZEND_DEBUG
#define ZEND_WIN32_KEEP_INLINE
#endif

#endif /* UNDEFDUPS_H_ */
