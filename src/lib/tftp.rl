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
 * @file tftp.c
 * @brief TFTP protocol library
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */
#include "tftp.h"
#include <apr_strings.h>
#include <stdio.h>

%%{
  machine tftp;

  action filename {
    pack->data->rq.filename = apr_pstrmemdup(mp, mark, fpc - mark);
    mark = p + 1;
  }

  action mode { pack->data->rq.mode = mark; }

  MODE_OCTET  = /octet/i;
  MODE_ASCII  = /netascii/i;
  MODE_MAIL   = /mail/i;
  MODE        = MODE_OCTET | MODE_ASCII | MODE_MAIL;
  BLOCK       = extend extend;
  ERCODE      = BLOCK;
  ASCII       = 1..127;

  RQ    = ASCII+ 0x0 @filename MODE 0x0 @mode;
  RRQ   = 0x00 0x01 >{pack->opcode = E_RRQ; } RQ;
  WRQ   = 0x00 0x02 >{pack->opcode = E_WRQ; } RQ;
  DATA  = 0x00 0x03 >{pack->opcode = E_DATA;} BLOCK any{1,512};
  ACK   = 0x00 0x04 BLOCK;
  ERROR = 0x00 0x05 ERCODE ASCII+ 0x0;
  tftp := (RRQ | WRQ | DATA | ACK | ERROR );

}%%

%%write data;

tftp_pack* tftp_packet_read (char* packet, apr_size_t len, apr_pool_t *mp)
{
  int cs;
  char *p  = packet;
  char *pe = p + len;
  char *mark = p + 2;
  tftp_pack *pack = (tftp_pack *) apr_palloc (mp, sizeof(tftp_pack));
  pack->data = (union data*) apr_palloc (mp, sizeof(union data));

  %%write init;
  %%write exec;

  if ( cs < tftp_first_final )
    printf ("ERROR PARSING cs = %d, tftp_first_final = %d\n", cs, tftp_first_final);
  return pack;
}

