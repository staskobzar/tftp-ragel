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

#define MODE_OCTET "octet"      /*!< define octet mode */
#define MODE_ASCII "netascii"   /*!< define netascii mode */
#define MODE_MAIL  "mail"       /*!< define mail mode */

/**
 * @enum mode
 * TFTP transfer modes.
 */
enum mode {
  E_OCTET = 0x0a, /*!< octet mode */
  E_ASCII,        /*!< netascii mode */
  E_MAIL          /*!< mail mode */
};

/*!
 * @enum opcodes
 * opcodes of TFTP packet types.
 * see: https://tools.ietf.org/html/rfc1350 section 5
 */
enum opcodes {
  E_RRQ   = 0x01, /*!< Read request */
  E_WRQ   = 0x02, /*!< Write request */
  E_DATA  = 0x03, /*!< Data */
  E_ACK   = 0x04, /*!< Acknowledgment */
  E_ERROR = 0x05, /*!< Error */
};

/**
 * TFTP packet structure for RRQ/WRQ packets without opcode.
 * Structure: filename 0x00 mode 0x00
 */
struct pack_rq {
  char *filename;             /*!< Requested file name */
  unsigned int len_filename;  /*!< File name length */
  char *mode;                 /*!< Transfer mode as string (netascii, octet, mail) */
  unsigned int len_mode;      /*!< Transfer mode string length */
  enum mode e_mode;           /*!< Transfer mode enum value */
};

/**
 * TFTP DATA packet structure without opcode.
 */
struct pack_data {
  uint16_t block;          /*!< block number 2 bytes */
  unsigned char data[512]; /*!< data field up to 512 bytes */
  unsigned int length;     /*!< data 2 bytes field length. When transfer, should be 512 bytes.
                                If less then 512 bytes, then this is the last TFTP packet. */
};

/**
 * TFTP ACK packet structure without opcode.
 */
struct pack_ack {
  uint16_t block; /*!< block number 2 bytes */
};

/**
 * TFTP ERROR packet structure without opcode.
 * Structure: code message 0x00
 */
struct pack_error {
  uint16_t ercode;      /*!< error code (2 bytes) */
  char *msg;            /*!< error message */
  unsigned int msg_len; /*!< error message length */
};

/**
 * TFTP packet union for data
 */
union data {
  struct pack_rq    rq;   /*!< WRQ/RRQ packet */
  struct pack_data  data; /*!< DATA packet */
  struct pack_ack   ack;  /*!< ACK packet */
  struct pack_error error;/*!< ERROR packet */
};

/**
 * TFTP packet structure.
 */
struct tftp_pack_s {
  uint16_t opcode;  /*!< opcode of TFTP packet: WRQ, RRQ, DATA etc. */
  union data *data; /*!< \union TFTP packet structure */
};

/*! TFTP packet type */
typedef struct tftp_pack_s tftp_pack;

/**
 * Read tftp packet received from socket.
 * @param packet TFTP packet
 * @param len    Packet length
 * @param mp     APR memory pool
 * @return tftp_pack struct
 */
tftp_pack* tftp_packet_read (char* packet, apr_size_t len, apr_pool_t *mp);

