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
#include "tftp.h"
#include "util.h"

#include <stdio.h> // to del

int main(int argc, const char *argv[])
{
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

  char raw_rrq[] = {0x00,0x01,0x66,0x6f,0x6f,
    0x2e,0x63,0x00,0x6e, 0x65,0x74,0x61,0x73,0x63,
    0x69,0x69,0x00};

  tftp_pack *pack = tftp_packet_read(raw_rrq, sizeof(raw_rrq), mp);
  printf ("OPCODE = %d size = %u\n", pack->opcode, sizeof(raw_rrq));
  printf ("filename : %s\n", pack->data->rq.filename);
  printf ("mode : %s\n", pack->data->rq.mode);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
