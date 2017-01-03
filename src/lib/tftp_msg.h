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
 * @file tftp_msg.h
 * @brief TFTP protocol library
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#ifndef __TFTP_MSG_H
#define __TFTP_MSG_H

#include <stdint.h>
#include <apr_general.h>
#include <apr_strings.h>

#define MODE_OCTET "octet"      /*!< define octet mode */
#define MODE_ASCII "netascii"   /*!< define netascii mode */

// TODO: remove mode mail
// From RFC1350 : The mail mode is obsolete and should not be implemented or used.
#define MODE_MAIL  "mail"       /*!< define mail mode */

#define DATA_SIZE  512          /*!< TFTP data block size as defined in RFC1350 */

#define BUF_SIZE   DATA_SIZE + 4 /*!< TFTP packet buffer size */

/*! @def tftp_req_pack(buf, opcode, pack)
 * Create request packet as char array.
 * @param buf     Result buffer as char array
 * @param opcode  TFTP packet opcode. E_RRQ or E_WRQ.
 * @param pack    Packet structure pack_rq
 */
#define tftp_req_pack(buf, opcode, pack) apr_snprintf ((buf),     \
                DATA_SIZE + 4, "%c%c%.*s%c%.*s%c", 0x0, (opcode), \
                (pack)->len_filename, (pack)->filename, 0x0,      \
                (pack)->len_mode, (pack)->mode, 0x0 )

/*! @def low_byte(num)
 * Get lower byte of short int
 * @param num   16 bit integer
 */
#define low_byte(num) ( (num) >> 8 )

/*! @def hi_byte(num)
 * Get higher byte of short int
 * @param num   16 bit integer
 */
#define hi_byte(num)  ( (num) & 0x00ff )

/**
 * @enum mode
 * TFTP transfer modes.
 */

enum mode {
  E_OCTET,        /*!< octet mode */
  E_ASCII,        /*!< netascii mode */
  E_MAIL          /*!< mail mode */
};

/*!
 * Mode to string.
 */
static char *mode_str[] = {
  MODE_OCTET, MODE_ASCII, MODE_MAIL
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

/*! String representation of opcode. */
static char *opcode_str[] = {
  "UNDEFINED", "RRQ", "WRQ", "DATA", "ACK", "ERROR"
};

/*!
 * @enum ercode
 * Error Codes of TFTP packet types.
 * see: https://tools.ietf.org/html/rfc1350
 */
enum ercode {
   ERR_UNDEF,     /*!< Not defined, see error message (if any). */
   ERR_NOTFOUND,  /*!< File not found. */
   ERR_ACCESS,    /*!< Access violation. */
   ERR_DISKFULL,  /*!< Disk full or allocation exceeded. */
   ERR_ILLEGAL,   /*!< Illegal TFTP operation. */
   ERR_XFERID,    /*!< Unknown transfer ID. */
   ERR_EXISTS,    /*!< File already exists. */
   ERR_NOUSER     /*!< No such user. */
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
  unsigned char data[DATA_SIZE]; /*!< data field up to 512 bytes */
  apr_size_t length;       /*!< data 2 bytes field length. When transfer, should be 512 bytes.
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
  enum ercode ercode;      /*!< error code (2 bytes) */
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
  union data *data; /*!< TFTP packet structure */
};

/*! TFTP packet type */
typedef struct tftp_pack_s tftp_pack;

/**
 * Read tftp packet received from socket.
 * @param packet TFTP packet
 * @param len    Packet length
 * @param mp     APR memory pool
 * @return tftp_pack struct or NULL if packet if failed to parse
 */
tftp_pack* tftp_packet_read (char* packet, apr_size_t len, apr_pool_t *mp);

/**
 * Create TFTP RRQ packet.
 * @param buf   Buffer where the result TFTP packet will be stored as char array.
 * @param rq    RRQ packet structure
 * @return Packet length
 */
apr_size_t tftp_create_rrq (char *buf, struct pack_rq *rq);

/**
 * Create TFTP WRQ packet.
 * @param buf   Buffer where the result TFTP packet will be stored as char array.
 * @param rq    RRQ packet structure
 * @return Packet length
 */
apr_size_t tftp_create_wrq (char *buf, struct pack_rq *rq);

/**
 * Create TFTP DATA packet.
 * @param buf   Buffer where the result TFTP packet will be stored as char array.
 * @param data  DATA packet structure
 * @return Packet length
 */
apr_size_t tftp_create_data (char *buf, struct pack_data *data);

/**
 * Create TFTP ACK packet.
 * @param buf   Buffer where the result TFTP packet will be stored as char array.
 * @param block Block number
 * @return Packet length
 */
apr_size_t tftp_create_ack (char *buf, int block);

/**
 * Create TFTP ERROR packet.
 * @param buf   Buffer where the result TFTP packet will be stored as char array.
 * @param error ERROR packet structure
 * @return Packet length
 */
apr_size_t tftp_create_error (char *buf, struct pack_error *error);

/**
 * Helper function for ASCII mode transfer.
 * Converts string received from network to host as
 * specified in "Telnet Protocol Specification", see RFC1350 section 1.
 * Converting CR, NUL -> CR and CR, LF -> LF.
 * @param mp    APR memory pool pointer
 * @param buf   String to convert
 * @param len   String length
 * @return String length after convertion
 */
apr_size_t tftp_str_ntoh (apr_pool_t *mp, char *buf, apr_size_t len);

/**
 * Helper function for ASCII mode transfer.
 * Converts string to network before send it as
 * specified in "Telnet Protocol Specification", see RFC1350 section 1.
 * Converting CR, NUL -> CR and CR, LF -> LF.
 * Converting CR -> CR, NUL and LF -> CR, LF.
 * @param mp    APR memory pool pointer
 * @param buf   String to convert
 * @param len   String length
 * @return String length after convertion
 */
apr_size_t tftp_str_hton (apr_pool_t *mp, char *buf, apr_size_t len);

#endif
