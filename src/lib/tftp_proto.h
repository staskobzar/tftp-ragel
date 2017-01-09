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
 * @file tftp_proto.h
 * @brief TFTP protocol library.
 * TFTP protocol machines.
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#ifndef __TFTP_PROTO_H
#define __TFTP_PROTO_H

#include <apr_general.h>
#include <apr_network_io.h>
#include <apr_file_io.h>

#include "tftp_msg.h"

/*! Default TFTP port */
#define TFTP_PORT 69

/*! Boolean type. */
typedef unsigned char bool;

/*! @enum e_state TFTP machine states. */
enum e_state { END, INIT, RECV, SEND };

/*! Machine state type. */
typedef enum e_state state;

/*!
 * State strings representation.
 */
static char *state_str[] = { "END", "INIT", "RECV", "SEND" };

/*! @enum file_action Action GET or PUT */
enum file_action {GET, PUT};

/*!
 * TFTP Finit State Machine structure.
 */
struct tftp_machine {
  unsigned int      tid;          /*!< Transaction id, port of response. */
  const char        *remote_file; /*!< Remote file name. */
  apr_file_t        *local_file;  /*!< Local file descriptor. */
  apr_socket_t      *sock;        /*!< Socket structure. */
  apr_sockaddr_t    *sockaddr;    /*!< Socket address structure. */
  uint16_t          block;        /*!< Packet block number. */
  enum file_action  action;       /*!< File action GET or PUT. */
  state             state;        /*!< Machine state. */
  enum opcodes      event;        /*!< Machine event. */
  apr_pool_t        *mp;          /*!< APR memory pool. */
  char              *buf;         /*!< Packet exchange buffer. */
  enum mode         mode;         /*!< Transaction mode: ascii or octet. */
  tftp_pack         *pack;        /*!< TFTP packet structure (see tftp_msg.h) */
};

/**
 * TFTP client command parameters.
 */
struct tftp_params {
  const char *remote_file;  /*!< Remote file name. */
  const char *local_file;   /*!< Local file name. */
  const char *host;         /*!< TFTP server host. */
  unsigned int port;        /*!< TFTP server port. */
  bool verbose;             /*!< Verbosity enable. */
  enum file_action action;  /*!< File action PUT or GET. */
  enum mode mode;           /*!< Transfer mode. */
};

/*!
 * Finit State Machine transition structure.
 */
struct trans_table {
  state   current_state;    /*!< State */
  enum    opcodes event;    /*!< Event */
  state   (*action)(void);  /*!< Pointer to func to execute. */
};

/**
 * Inititate TFTP protocol machine.
 * @param mp      APR memory pool.
 * @param params  Command paramters
 * @return APR status
 */
apr_status_t tftp_proto_init (apr_pool_t *mp, struct tftp_params *params);

/**
 * Run TFTP Finit State Machine.
 * @return Current State.
 */
state tftp_proto_fsm ();

/**
 * Create and send WRQ or RRQ packet.
 * @return Current State.
 */
state tftp_proto_rq (void);

/**
 * Process ERROR packet.
 * @return Current State.
 */
state tftp_proto_error (void);

/**
 * Create and send DATA packet.
 * @return Current State.
 */
state tftp_proto_send_data (void);

/**
 * Receive DATA packet.
 * @return Current State.
 */
state tftp_proto_recv_data (void);

/**
 * Process ACK packet.
 * @return Current State.
 */
state tftp_proto_ack (void);

#endif
