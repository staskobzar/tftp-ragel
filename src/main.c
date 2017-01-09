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

/**
 * TFTPClient main proc.
 */
int main(int argc, const char *argv[])
{
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

  struct tftp_params params;

  if (parse_args (mp, &params, argc, argv) != APR_SUCCESS) {
    printf("Run \"%s --help\" for options list.\n", argv[0]);
    goto done;
  }

  DBG("Init TFTP protocol machine.");
  if (tftp_proto_init (mp, &params) != APR_SUCCESS) {
    ERR("Failed to initiate tftp proto.");
    goto done;
  }

  // running TFTP protocol finit state machine
  DBG("Start TFTP Finit State Machine.");
  while(tftp_proto_fsm());

done:
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
