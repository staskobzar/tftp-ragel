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

char raw_rrq[] = {0x00,0x01,0x66,0x6f,0x6f,
0x2e,0x63,0x00,0x6e, 0x65,0x74,0x61,0x73,0x63,
0x69,0x69,0x00};

char raw_data[] = {  0x00,0x03,0x40,0xe7, 0x20,0x20,0x54,0x68, 0x65,0x20,0x47,0x4e, 0x55,0x20,0x47,0x65, 0x6e,0x65,0x72,0x61, 0x6c,0x20,0x50,0x75, 0x62,0x6c,0x69,0x63,
0x20,0x4c,0x69,0x63, 0x65,0x6e,0x73,0x65, 0x20,0x69,0x73,0x20, 0x61,0x20,0x66,0x72, 0x65,0x65,0x2c,0x20, 0x63,0x6f,0x70,0x79, 0x6c,0x65,0x66,0x74, 0x20,0x6c,0x69,0x63,
0x65,0x6e,0x73,0x65, 0x20,0x66,0x6f,0x72, 0x0a,0x73,0x6f,0x66, 0x74,0x77,0x61,0x72, 0x65,0x20,0x61,0x6e, 0x64,0x20,0x6f,0x74, 0x68,0x65,0x72,0x20, 0x6b,0x69,0x6e,0x64,
0x73,0x20,0x6f,0x66, 0x20,0x77,0x6f,0x72, 0x6b,0x73,0x2e,0x0a, 0x0a,0x20,0x20,0x54, 0x68,0x65,0x20,0x6c, 0x69,0x63,0x65,0x6e, 0x73,0x65,0x73,0x20, 0x66,0x6f,0x72,0x20,
0x6d,0x6f,0x73,0x74, 0x20,0x73,0x6f,0x66, 0x74,0x77,0x61,0x72, 0x65,0x20,0x61,0x6e, 0x64,0x20,0x6f,0x74, 0x68,0x65,0x72,0x20, 0x70,0x72,0x61,0x63, 0x74,0x69,0x63,0x61,
0x6c,0x20,0x77,0x6f, 0x72,0x6b,0x73,0x20, 0x61,0x72,0x65,0x20, 0x64,0x65,0x73,0x69, 0x67,0x6e,0x65,0x64, 0x0a,0x74,0x6f,0x20, 0x74,0x61,0x6b,0x65, 0x20,0x61,0x77,0x61,
0x79,0x20,0x79,0x6f, 0x75,0x72,0x20,0x66, 0x72,0x65,0x65,0x64, 0x6f,0x6d,0x20,0x74, 0x6f,0x20,0x73,0x68, 0x61,0x72,0x65,0x20, 0x61,0x6e,0x64,0x20, 0x63,0x68,0x61,0x6e,
0x67,0x65,0x20,0x74, 0x68,0x65,0x20,0x77, 0x6f,0x72,0x6b,0x73, 0x2e,0x20,0x20,0x42, 0x79,0x20,0x63,0x6f, 0x6e,0x74,0x72,0x61, 0x73,0x74,0x2c,0x0a, 0x74,0x68,0x65,0x20,
0x47,0x4e,0x55,0x20, 0x47,0x65,0x6e,0x65, 0x72,0x61,0x6c,0x20, 0x50,0x75,0x62,0x6c, 0x69,0x63,0x20,0x4c, 0x69,0x63,0x65,0x6e, 0x73,0x65,0x20,0x69, 0x73,0x20,0x69,0x6e,
0x74,0x65,0x6e,0x64, 0x65,0x64,0x20,0x74, 0x6f,0x20,0x67,0x75, 0x61,0x72,0x61,0x6e, 0x74,0x65,0x65,0x20, 0x79,0x6f,0x75,0x72, 0x20,0x66,0x72,0x65, 0x65,0x64,0x6f,0x6d,
0x20,0x74,0x6f,0x0a, 0x73,0x68,0x61,0x72, 0x65,0x20,0x61,0x6e, 0x64,0x20,0x63,0x68, 0x61,0x6e,0x67,0x65, 0x20,0x61,0x6c,0x6c, 0x20,0x76,0x65,0x72, 0x73,0x69,0x6f,0x6e,
0x73,0x20,0x6f,0x66, 0x20,0x61,0x20,0x70, 0x72,0x6f,0x67,0x72, 0x61,0x6d,0x2d,0x2d, 0x74,0x6f,0x20,0x6d, 0x61,0x6b,0x65,0x20, 0x73,0x75,0x72,0x65, 0x20,0x69,0x74,0x20,
0x72,0x65,0x6d,0x61, 0x69,0x6e,0x73,0x20, 0x66,0x72,0x65,0x65, 0x0a,0x73,0x6f,0x66, 0x74,0x77,0x61,0x72, 0x65,0x20,0x66,0x6f, 0x72,0x20,0x61,0x6c, 0x6c,0x20,0x69,0x74,
0x73,0x20,0x75,0x73, 0x65,0x72,0x73,0x2e, 0x20,0x20,0x57,0x65, 0x2c,0x20,0x74,0x68, 0x65,0x20,0x46,0x72, 0x65,0x65,0x20,0x53, 0x6f,0x66,0x74,0x77, 0x61,0x72,0x65,0x20,
0x46,0x6f,0x75,0x6e, 0x64,0x61,0x74,0x69, 0x6f,0x6e,0x2c,0x20, 0x75,0x73,0x65,0x20, 0x74,0x68,0x65,0x0a, 0x47,0x4e,0x55,0x20, 0x47,0x65,0x6e,0x65, 0x72,0x61,0x6c,0x20,
0x50,0x75,0x62,0x6c, 0x69,0x63,0x20,0x4c, 0x69,0x63,0x65,0x6e, 0x73,0x65,0x20,0x66, 0x6f,0x72,0x20,0x6d, 0x6f,0x73,0x74,0x20, 0x6f,0x66,0x20,0x6f, 0x75,0x72,0x20,0x73,
0x6f,0x66,0x74,0x77, 0x61,0x72,0x65,0x3b };

int main(int argc, const char *argv[])
{
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

  tftp_pack *pack = tftp_packet_read(raw_data, sizeof(raw_data), mp);
  printf ("OPCODE = %d size = %lu\n", pack->opcode, sizeof(raw_data));
  //printf ("filename : %s\n", pack->data->rq.filename);
  //printf ("mode : %s\n", pack->data->rq.mode);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
