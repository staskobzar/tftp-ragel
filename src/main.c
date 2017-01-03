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
 * @file main.c
 * @brief tftpclient main proc
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */
#include "tftp_msg.h"
#include "tftp_proto.h"
#include "util.h"

#include <apr_network_io.h>
#include <apr_file_io.h>
#include <stdio.h> // to del

int main(int argc, const char *argv[])
{
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

//----------------------------------------------------------------
//----------------------------------------------------------------

  struct tftp_params params = {
    .host = "127.0.0.1",
    .port = 69,
    .action = PUT,
    .mode = E_ASCII,
    .local_file = "Makefile",
    .remote_file = "bootstrap.sh",
  };
  if (tftp_proto_init (mp, &params) == APR_SUCCESS) {
    printf("Initiated TFTP proto.\n");
    while(tftp_proto_fsm());
  } else {
    printf("Failed to initiate tftp proto.\n");
  }
//----------------------------------------------------------------
//----------------------------------------------------------------

  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
