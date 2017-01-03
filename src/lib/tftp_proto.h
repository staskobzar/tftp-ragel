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

typedef unsigned char bool;

enum e_state {
  END, INIT, RECV, SEND
};
typedef enum e_state state;

static char *state_str[] = {
  "END", "INIT", "RECV", "SEND"
};

enum file_action {GET, PUT};

struct tftp_machine {
  unsigned int      tid; // transaction id, port of response
  char              *remote_file;
  state             state;
  apr_file_t        *local_file;
  apr_socket_t      *sock;
  apr_sockaddr_t    *sockaddr;
  uint16_t          block;
  enum file_action  action;
  enum opcodes      event;
  apr_pool_t        *mp;
  char              *buf;
  enum mode         mode;
  tftp_pack         *pack;
};

struct tftp_params {
  char *remote_file;
  char *local_file;
  char *host;
  unsigned int port;
  bool verbose;
  enum file_action action;
  enum mode mode;
};

/*!
 * Finit State Machine transition structure.
 */
struct trans_table {
  state   current_state;
  enum    opcodes event;
  state   (*action)(void);
};

apr_status_t tftp_proto_init (apr_pool_t *mp, struct tftp_params *params);

state tftp_proto_fsm ();

state tftp_proto_rq (void);
state tftp_proto_error (void);
state tftp_proto_send_data (void);
state tftp_proto_recv_data (void);
state tftp_proto_ack (void);

#endif
