/**
 * tftpclient -- TFTP client application.
 * Copyright (C) 2016, Stas Kobzar <staskobzar@modulis.ca>
 *
 * This file is part of tftpclient.
 *
 * tftpclient is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * tftpclient is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with tftpclient.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file util.h
 * @brief tftpclient utilities function
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#ifndef __UTIL_H
#define __UTIL_H

#include <config.h>
#include <apr_getopt.h>
#include "tftp_proto.h"

/*! @def DBG(..)
 * Print debug message when enabled.
 */
#define DBG(...) log_print(__FILE__, __LINE__, DEBUG, __VA_ARGS__)
/*! @def LOG(..)
 * Print log message when verbosity enabled.
 */
#define LOG(...) log_print(__FILE__, __LINE__, LOG,   __VA_ARGS__)
/*! @def ERR(..)
 * Print error message.
 */
#define ERR(...) log_print(__FILE__, __LINE__, ERROR, __VA_ARGS__)

/*! @enum loglvl
 * Logging level.
 */
enum loglvl {DEBUG, LOG, ERROR};

/**
 * Print log message.
 * @param file    File name. For debuggin messages.
 * @param line    Line name where debug output.
 * @param loglvl  Logging level.
 * @param fmt     Output string format.
 * @param ...     Formatting arguments.
 */
void log_print (char *file, int line, enum loglvl loglvl, char *fmt, ...);

/**
 * Parse command line arguments.
 * @param mp      APR memory pool.
 * @param params  TFTP command parameters.
 * @param argc    Arguments count.
 * @param argv    Arguments vector.
 * @return APR status.
 */
apr_status_t parse_args (apr_pool_t *mp, struct tftp_params *params, int argc, const char **argv);

/**
 * Print usage help message.
 */
void usage (void);

/**
 * Print version and copyright message.
 */
void copyright (void);

#endif
