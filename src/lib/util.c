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
 * @file util.c
 * @brief tftpclient utilities functions
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#include "util.h"
#include <stdlib.h>
#include <stdio.h>

static const apr_getopt_option_t options[] = {
  { "help",     'h',  FALSE,  "Print usage and breif help message."   },
  { "port",     'P',  TRUE,   "Server port. Default: 69."             },
  { "put",      'p',  FALSE,  "Put file to remote tftp server."       },
  { "get",      'g',  FALSE,  "Get file from remote tftp server."     },
  { "mode",     'm',  TRUE,   "Transfer mode. Value: octet or ascii. "
                              "If not set, then default is 'ascii'."  },
  { "verbose",  'v',  FALSE,  "Print additional infomation during transfer."},
  { "debug",    'd',  FALSE,  "Print lots of debug data."             },
  { "version",  'V',  FALSE,  "Print version."                        },
  /* sentinel */
  { NULL, 0, 0, NULL },
};

/*! File actions string representation. */
char *file_action_str[] = {"GET", "PUT"};

static bool verbose = FALSE;
static bool debug   = FALSE;

apr_status_t parse_args (apr_pool_t *mp, struct tftp_params *params, int argc, const char **argv)
{
  apr_status_t rv;
  apr_getopt_t *getopt;
  int optch;
  const char *optarg;
  char *endptr;
  unsigned int port = 0;

  // Init default parameters
  params->port = TFTP_PORT;
  params->action = GET;
  params->mode = E_ASCII;

  apr_getopt_init(&getopt, mp, argc, argv);

  //getopt->errfn = NULL; // disable default APR errors on invalid options
  while ((rv = apr_getopt_long(getopt, options, &optch, &optarg)) == APR_SUCCESS) {
    switch (optch) {
      case 'V':               // print version and exit
        copyright ();
        return 1;
        break;
      case 'h':               // print usage/help and exit
        usage ();
        return 1;
        break;
      case 'P':               // set TFTP server port
        port = strtol(optarg, &endptr, 10);
        if (port < 1 || port > 65535) {
          ERR("Invalid port value: %s", optarg);
          return APR_BADARG;
        }
        break;
      case 'p':               // put file to TFTP server
        params->action = PUT;
        break;
      case 'g':               // get file from TFTP server
        params->action = GET;
        break;
      case 'v':               // enable verbosity
        verbose = TRUE;
        break;
      case 'd':               // enable debug output
        debug = TRUE;
        break;
      case 'm':               // set transfer mode
        if (apr_strnatcasecmp (optarg, "ascii") == 0) {
          params->mode = E_ASCII;
        } else if (apr_strnatcasecmp (optarg, "octet") == 0) {
          params->mode = E_OCTET;
        } else {
          ERR("Invalid mode: %s", optarg);
          return APR_BADARG;
        }
        break;
      default:
        return APR_BADARG;
        break;
    }
  }
  if (rv != APR_EOF) {
    return rv;
  }
  // set host
  if (getopt->ind < argc) {
    params->host = getopt->argv[getopt->ind];
    DBG("TFTP host: %s", params->host);
    getopt->ind++;
  } else {
    ERR("Missing TFTP server host/IP address.");
    return APR_BADARG;
  }
  // set remote file
  if (getopt->ind < argc) {
    params->remote_file = getopt->argv[getopt->ind];
    DBG("Remote file: %s", params->remote_file);
    getopt->ind++;
  } else {
    ERR("Missing remote file name.");
    return APR_BADARG;
  }
  // set local file
  if (getopt->ind < argc) {
    params->local_file = getopt->argv[getopt->ind];
    getopt->ind++;
  } else {
    params->local_file = params->remote_file;
  }
  DBG("Local file: %s", params->remote_file);

  if (getopt->ind < argc) {
    for (;getopt->ind < argc; getopt->ind++)
      ERR("Unknown parameter: %s", getopt->argv[getopt->ind]);
    return APR_BADARG;
  }

  LOG("%s file %s TFTP server %s:%d/%s %s %s",
               file_action_str[params->action],
               params->action == GET ? "from" : "to",
               params->host, params->port, params->remote_file,
               params->action == GET ? "to" : "from",
               params->local_file);

  return APR_SUCCESS;
}

void log_print(char *file, int line, enum loglvl level, char *fmt, ...)
{
  va_list args;
  switch (level) {
    case DEBUG:
      if (!debug) return;
      printf("[DEBUG] %s:%d: ", file, line);
      break;
    case LOG:
      if (!verbose) return;
      printf("[INFO]  ");
      break;
    case ERROR:
      printf("ERROR:  ");
      break;
  }
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  printf("\n");
}


void usage ()
{
  const apr_getopt_option_t *opts = options;
  printf("Usage: " PACKAGE " [OPTION] HOST REMOTE_FILE [LOCAL_FILE]\n");
  printf("Get file from TFTP server or put file to TFTP server.\n");
  printf("HOST        - Hostname or IP address of TFTP server.\n");
  printf("REMOTE_FILE - Source file. When  getting  file, then  it is  remote  file name.\n");
  printf("              When sending file to  remote server, this is name of  local file.\n");
  printf("LOCAL_FILE  - Destination file. When  getting file, then it is  local file name\n");
  printf("              or path where to copy  file from  TFTP server. When  sending file\n");
  printf("              to remote server, this is name of file to store on remote server.\n");
  printf("\n");
  printf("Mandatory arguments to long options are mandatory for short options too.\n");
  printf("\n");
  printf("Options:\n");
  while (opts->optch != 0) {
    printf("  -%c, --%s ", opts->optch, opts->name);
    if (opts->has_arg) printf("[VALUE]");
    printf("\n");
    printf("        %s\n", opts->description);
    opts++;
  }

  printf("\n");
  copyright();
}

void copyright()
{
  printf( PACKAGE_STRING " Copyright (C) 2017  " PACKAGE_BUGREPORT "\n"
          "This program comes with ABSOLUTELY NO WARRANTY.\n"
          "This is free software, and you are welcome to redistribute it\n"
          "under conditions describer in COPYRIGHT file.\n");
}

