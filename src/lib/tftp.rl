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
#include <stdio.h>

/* TODO: convert to macros */
static apr_size_t tftp_rq ( char *buf,
                            enum opcodes opcode,
                            struct pack_rq *rq)
{
/*
  2 bytes     string    1 byte     string   1 byte
 ------------------------------------------------
| Opcode |  Filename  |   0  |    Mode    |   0  |
------------------------------------------------
*/
  return apr_snprintf (buf, DATA_SIZE + 4, "%c%c%.*s%c%.*s%c",
    0x0, opcode, rq->len_filename, rq->filename,
    0x0, rq->len_mode, rq->mode, 0x0
  );
}

/**
 * Ragel Finit State Machine
 */
%%{
  machine tftp;

  action filename {
    pack->data->rq.len_filename = fpc - mark;
    pack->data->rq.filename = apr_pstrmemdup(mp, mark, pack->data->rq.len_filename);
    mark = p + 1;
  }

  action mode {
    pack->data->rq.len_mode = fpc - mark;
    pack->data->rq.mode = apr_pstrmemdup(mp, mark, pack->data->rq.len_mode);
    if(apr_strnatcasecmp (pack->data->rq.mode, MODE_OCTET) == 0) {
      pack->data->rq.e_mode = E_OCTET;
    } else if( apr_strnatcasecmp (pack->data->rq.mode, MODE_ASCII) == 0) {
      pack->data->rq.e_mode = E_ASCII;
    } else {
      pack->data->rq.e_mode = E_MAIL;
    }
  }

  action block {
    unsigned char low = *mark & 0xff;       // get lower byte and make sure it is 1 byte
    unsigned char hi  = *(mark +1) & 0xff;  // get higher byte and make sure it is 1 byte
    block_num = hi | (low << 8);
    mark = p;
  }

  action pack_data {
    pack->opcode = E_DATA;
    apr_size_t len = fpc - mark;
    pack->data->data.block = block_num;
    apr_cpystrn (pack->data->data.data, mark, len + 1);
    pack->data->data.length = len;
  }

  action pack_ack {
    pack->opcode = E_ACK;
    pack->data->ack.block = block_num;
  }

  action pack_error {
    pack->opcode = E_ERROR;
    pack->data->error.ercode = block_num;
    pack->data->error.msg_len = fpc - mark - 1; // -1 for last 0x0 byte
    pack->data->error.msg = apr_pstrmemdup(mp, mark, pack->data->error.msg_len);
  }

  MODE_OCTET  = /octet/i;
  MODE_ASCII  = /netascii/i;
  MODE_MAIL   = /mail/i;
  MODE        = MODE_OCTET | MODE_ASCII | MODE_MAIL;
  BLOCK       = extend extend %block;
  ERCODE      = BLOCK;
  ASCII       = 1..127;

  RQ    = ASCII+ 0x0 @filename MODE 0x0 @mode;
  RRQ   = 0x00 0x01 >{pack->opcode = E_RRQ;} RQ;
  WRQ   = 0x00 0x02 >{pack->opcode = E_WRQ;} RQ;
  DATA  = 0x00 0x03 BLOCK extend{1,512}  %pack_data;
  ACK   = 0x00 0x04 BLOCK                %pack_ack;
  ERROR = 0x00 0x05 ERCODE ASCII+ 0x0    %pack_error;

  tftp := (RRQ | WRQ | DATA | ACK | ERROR);

}%%

%%write data;

tftp_pack* tftp_packet_read (char* packet, apr_size_t len, apr_pool_t *mp)
{
  int cs;
  char *p     = packet;
  char *pe    = p + len;
  char *eof   = pe;
  char *mark  = p + 2;
  uint16_t block_num;
  tftp_pack *pack = (tftp_pack *) apr_palloc (mp, sizeof(tftp_pack));
  pack->data = (union data*) apr_palloc (mp, sizeof(union data));

  %%write init;
  %%write exec;

  if ( cs < tftp_first_final )
    return NULL;
  return pack;
}

apr_size_t tftp_create_rrq (char *buf, struct pack_rq *rq)
{
  return tftp_rq (buf, E_RRQ, rq);
}

apr_size_t tftp_create_wrq (char *buf, struct pack_rq *rq)
{
  return tftp_rq (buf, E_WRQ, rq);
}

apr_size_t tftp_create_data (char *buf, struct pack_data *data)
{
  unsigned char low = (data->block >> 8) & 0xff;
  unsigned char hi  = (data->block - (low << 8)) & 0xff;

  return apr_snprintf (buf, DATA_SIZE + 5, "%c%c%c%c%.*s",
    0x0, E_DATA, low, hi, data->length, data->data
  );
}

apr_size_t tftp_create_ack (char *buf, int block)
{
  unsigned char low = (block >> 8) & 0xff;
  unsigned char hi  = (block - (low << 8)) & 0xff;

  return apr_snprintf (buf, DATA_SIZE, "%c%c%c%c",
    0x0, E_ACK, low, hi);
}

apr_size_t tftp_create_error (char *buf, struct pack_error *error)
{
  return apr_snprintf (buf, DATA_SIZE, "%c%c%c%c%.*s%c",
    0x0, E_ERROR, 0x0, error->ercode, error->msg_len,
    error->msg, 0x0);
}
