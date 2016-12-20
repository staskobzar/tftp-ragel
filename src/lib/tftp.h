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
 * @file tftp.h
 * @brief TFTP protocol library
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#include <stdint.h>
#include <apr_general.h>
#include <apr_strings.h>

#define MODE_OCTET "octet"
#define MODE_ASCII "netascii"
#define MODE_MAIL  "mail"

enum opcodes {
  E_RRQ   = 0x01,
  E_WRQ   = 0x02,
  E_DATA  = 0x03,
  E_ACK   = 0x04,
  E_ERROR = 0x05,
};
/*
   Type   Op #     Format without header
          2 bytes    string   1 byte     string   1 byte
          -----------------------------------------------
   RRQ/  | 01/02 |  Filename  |   0  |    Mode    |   0  |
   WRQ    -----------------------------------------------
*/
struct pack_rq {
  char *filename;
  unsigned int len_filename;
  char *mode;
  unsigned int len_mode;
};

/*
   Type   Op #     Format without header
          2 bytes    2 bytes       n bytes
          ---------------------------------
   DATA  | 03    |   Block #  |    Data    |
          ---------------------------------
*/
struct pack_data {
  uint16_t block;
  unsigned char data[512];
  unsigned int length;
};

/*
   Type   Op #     Format without header
          2 bytes    2 bytes
          -------------------
   ACK   | 04    |   Block #  |
          --------------------
*/
struct pack_ack {
  uint16_t block;
};

/*
   Type   Op #     Format without header
          2 bytes  2 bytes        string    1 byte
          ----------------------------------------
   ERROR | 05    |  ErrorCode |   ErrMsg   |   0  |
          ----------------------------------------
*/
struct pack_error {
  uint16_t ercode;
  char *msg;
  unsigned int msg_len;
};

union data {
  struct pack_rq    rq;
  struct pack_data  data;
  struct pack_ack   ack;
  struct pack_error error;
};

/**
 * TFTP packet structure.
 */
struct tftp_pack_s {
  uint16_t opcode;
  union data *data;
};

typedef struct tftp_pack_s tftp_pack;

/**
 * Read tftp packet received from socket.
 * @param packet TFTP packet
 * @param len    Packet length
 * @param mp     APR memory pool
 * @return tftp_pack struct
 */
tftp_pack* tftp_packet_read (char* packet, apr_size_t len, apr_pool_t *mp);

