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
 * @file tftp_msg.c
 * @brief TFTP protocol library.
 * Message parsing and creating.
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */
#include "tftp_msg.h"

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
    pack->data->data.length = len;
    memcpy (pack->data->data.data, mark, len + 1);
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
  return tftp_req_pack(buf, E_RRQ, rq);
}

apr_size_t tftp_create_wrq (char *buf, struct pack_rq *rq)
{
  return tftp_req_pack(buf, E_WRQ, rq);
}

apr_size_t tftp_create_data (char *buf, struct pack_data *data)
{
  int len = apr_snprintf (buf, 5, "%c%c%c%c",
    0x0, E_DATA, low_byte(data->block), hi_byte(data->block));

  for (; len < data->length + 4; len++) {
    buf[len] = data->data[len - 4];
  }
  return len;
}

apr_size_t tftp_create_ack (char *buf, int block)
{
  return apr_snprintf (buf, DATA_SIZE, "%c%c%c%c",
    0x0, E_ACK, low_byte(block), hi_byte(block));
}

apr_size_t tftp_create_error (char *buf, struct pack_error *error)
{
  return apr_snprintf (buf, DATA_SIZE, "%c%c%c%c%.*s%c",
    0x0, E_ERROR, 0x0, error->ercode, (int)error->msg_len,
    error->msg, 0x0);
}

apr_size_t tftp_str_ntoh (apr_pool_t *mp, char *buf, apr_size_t len)
{
  char *nbuf = apr_palloc (mp, len);
  apr_size_t i, new_len = 0;
  for (i = 0; i < len; i++) {
    if (buf[i] == APR_ASCII_CR && buf[i + 1] == '\0') {
      i++;
    } else if (buf[i] == APR_ASCII_CR && buf[i + 1] == APR_ASCII_LF) {
      nbuf[new_len++] = APR_ASCII_LF;
      i++;
    } else {
      nbuf[new_len++] = buf[i];
    }
  }
  apr_cpystrn(buf, nbuf, new_len + 1);
  return new_len;
}

apr_size_t tftp_str_hton (apr_pool_t *mp, char *buf, apr_size_t len)
{
  char *nbuf = apr_palloc (mp, len);
  apr_size_t i, new_len = 0;
  for (i = 0; i < len; i++) {
    if (buf[i] == APR_ASCII_CR && buf[i + 1] != APR_ASCII_LF) {
      nbuf[new_len++] = APR_ASCII_CR;
      nbuf[new_len++] = '\0';
    } else if (buf[i] == APR_ASCII_LF && buf[i - 1] != APR_ASCII_CR) {
      nbuf[new_len++] = APR_ASCII_CR;
      nbuf[new_len++] = APR_ASCII_LF;
    } else {
      nbuf[new_len++] = buf[i];
    }
  }
  apr_cpystrn(buf, nbuf, new_len + 1);
  return new_len;
}

