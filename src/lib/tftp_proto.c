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
 * @file tftp_proto.c
 * @brief TFTP protocol library.
 * TFTP protocol machines.
 *
 * @author Stas Kobzar <staskobzar@gmail.com>
 */

#include "tftp_proto.h"
#include "util.h"

/*!
 * Finit State Machine transition table.
 */
static struct trans_table transition[] = {
  {INIT,  E_RRQ,    tftp_proto_rq         },
  {INIT,  E_WRQ,    tftp_proto_rq         },
  {RECV,  E_ERROR,  tftp_proto_error      },
  {RECV,  E_DATA,   tftp_proto_recv_data  },
  {RECV,  E_ACK,    tftp_proto_send_data  },
  {SEND,  E_ACK,    tftp_proto_ack        },
  {SEND,  E_ERROR,  tftp_proto_error      },
  /* sentinel */
  {END,   0,        NULL                  }
};

static struct tftp_machine machine;

apr_status_t tftp_proto_init (apr_pool_t *mp, struct tftp_params *params)
{
  apr_status_t rv; // return value
  apr_int32_t file_open_flag;

  machine.state = INIT;
  machine.tid = 0;      // init transaction id
  machine.action = params->action;
  machine.mode = params->mode;
  DBG("Mode is (%d) %s", machine.mode, mode_str[machine.mode]);
  machine.mp = mp;

  rv = apr_sockaddr_info_get(&machine.sockaddr, params->host, APR_INET, params->port, 0, mp);
  if (rv != APR_SUCCESS) {
    ERR("Failed get socket address info UDP:%s:%d.", params->host, params->port);
    return rv;
  }

  rv = apr_socket_create(&machine.sock, machine.sockaddr->family, SOCK_DGRAM, APR_PROTO_UDP, mp);
  if (rv != APR_SUCCESS) {
    ERR("Failed to create socket (apr_socket_create).");
    return rv;
  }

  if (machine.action == GET) {
    file_open_flag  = APR_FOPEN_CREATE|APR_FOPEN_WRITE|APR_FOPEN_TRUNCATE;
    machine.event   = E_RRQ;
  } else {
    file_open_flag  = APR_FOPEN_READ;
    machine.event   = E_WRQ;
  }
  if (machine.mode == E_OCTET) {
    DBG("Add binary flag to file open function.");
    file_open_flag |= APR_FOPEN_BINARY;
  }

  DBG("Machine event: %s", opcode_str[machine.event]);

  rv = apr_file_open (&machine.local_file, params->local_file,
        file_open_flag, APR_OS_DEFAULT, mp);
  if (rv != APR_SUCCESS) {
    ERR("Failed open file %s", params->local_file);
    return rv;
  }
  DBG("Opened file %s", params->local_file);
  machine.remote_file = params->remote_file;

  machine.buf = apr_palloc(mp, BUF_SIZE);
  DBG("Allocated TFTP message exchange buffer with size %d bytes.", BUF_SIZE);

  return APR_SUCCESS;
}

state tftp_proto_fsm ()
{
  struct trans_table *table = transition;
  state rv = END;
  do {
    if (table->current_state == END) {
      rv = END;
      break;
    }
    if (table->current_state == machine.state && table->event == machine.event) {
      rv = table->action();
      DBG("Next state: %s", state_str[rv]);
      break;
    }
  } while (table++);

  return rv;
}

state tftp_proto_rq (void)
{
  apr_size_t len;
  apr_status_t rv;

  LOG("--> %-5s %s 0x0 %s", opcode_str[machine.event], machine.remote_file, mode_str[machine.mode]);
  struct pack_rq rq = {
    .filename = (char *)machine.remote_file,
    .len_filename = strlen(machine.remote_file),
    .mode = mode_str[machine.mode],
    .len_mode = strlen(mode_str[machine.mode]),
    .e_mode = machine.mode
  };

  len = tftp_req_pack (machine.buf, machine.event, &rq);
  rv = apr_socket_sendto (machine.sock, machine.sockaddr, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to send packet %s", opcode_str[machine.event]);
    machine.state = END;
    return END;
  }
  DBG("Sent packet %s length %lu", opcode_str[machine.event], len);
  len = BUF_SIZE;
  rv = apr_socket_recvfrom (machine.sockaddr, machine.sock, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to receive packet. Stop here.");
    machine.state = END;
    return END;
  }
  DBG("Sent packet length %lu", len);
  apr_socket_addr_get(&machine.sockaddr, APR_REMOTE, machine.sock);
  machine.tid = machine.sockaddr->port;
  DBG("Remote transaction ID (port): %d", machine.tid);

  machine.pack = tftp_packet_read(machine.buf, len, machine.mp);
  machine.state = RECV;
  machine.event = machine.pack->opcode;

  DBG("Event: %s, State: %s", opcode_str[machine.event], state_str[machine.state]);

  return RECV;
}

state tftp_proto_error (void)
{
  ERR("Transfer error: [%d] %s\n", machine.pack->data->error.ercode, machine.pack->data->error.msg);
  return machine.state = END;
}

state tftp_proto_recv_data (void)
{
  apr_size_t len;
  apr_status_t rv;

  machine.block = machine.pack->data->data.block;
  if (machine.mode == E_ASCII) {
    len = tftp_str_ntoh (machine.mp,
                         machine.pack->data->data.data,
                         machine.pack->data->data.length);
  } else {
    len = machine.pack->data->data.length;
  }
  LOG("<-- %-5s block# %05d [%d bytes]", opcode_str[machine.pack->opcode],
      machine.block, machine.pack->data->data.length);

  rv = apr_file_write(machine.local_file, machine.pack->data->data.data, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to write to file");
    return rv;
  }

  // last data packet
  if (machine.pack->data->data.length < DATA_SIZE) {
    len = tftp_create_ack (machine.buf, machine.block);
    machine.state = END;
    LOG("--> %-5s block# %05d <last data>", opcode_str[E_ACK], machine.block);
    rv = apr_socket_sendto (machine.sock, machine.sockaddr, 0, machine.buf, &len);
    if (rv != APR_SUCCESS) {
      ERR("Failed to send ACK");
    }
  } else {
    machine.state = SEND;
  }
  machine.event = E_ACK;
  DBG("Event: %s, State: %s", opcode_str[machine.event], state_str[machine.state]);
  return machine.state;
}

state tftp_proto_send_data (void)
{
  apr_size_t len;
  apr_status_t rv;

  machine.block = machine.pack->data->ack.block + 1;

  struct pack_data data = {
    .block = machine.block,
    .length = DATA_SIZE
  };
  rv = apr_file_read (machine.local_file, (void*) data.data, &data.length);
  if (rv != APR_SUCCESS) {
    char error[1024];
    apr_strerror(rv, error, 1024);
    ERR("[%d] %s", rv, error);
    return END;
  }
  DBG("Read data from file.");

  len = tftp_create_data (machine.buf, &data);
  LOG("--> %-5s block# %05d [%d bytes]", opcode_str[E_DATA], machine.block, len);
  rv = apr_socket_sendto (machine.sock, machine.sockaddr, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to send DATA block #%d.", data.block);
    return machine.state = END;
  }

  len = BUF_SIZE;
  rv = apr_socket_recvfrom (machine.sockaddr, machine.sock, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to receive packet on response to %s.", opcode_str[machine.event]);
    return machine.state = END;
  }
  DBG("Recved packet len: %lu", len);
  machine.pack = tftp_packet_read(machine.buf, len, machine.mp);
  LOG("<-- %-5s block# %05d", opcode_str[machine.pack->opcode], machine.block);
  if (data.length < DATA_SIZE) {
    DBG("Last packet detected.");
    return machine.state = END;
  }
  machine.event = machine.pack->opcode;
  machine.state = RECV;

  DBG("Event: %s, State: %s", opcode_str[machine.event], state_str[machine.state]);
  return machine.state;
}

state tftp_proto_ack (void)
{
  apr_size_t len;
  apr_status_t rv;
  len = tftp_create_ack (machine.buf, machine.block);
  LOG("--> %-5s block# %05d", opcode_str[E_ACK], machine.block, len);
  rv = apr_socket_sendto (machine.sock, machine.sockaddr, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to send ACK.");
    return END;
  }
  DBG("Sent to server %lu bytes.", len);

  len = BUF_SIZE;
  rv = apr_socket_recvfrom (machine.sockaddr, machine.sock, 0, machine.buf, &len);
  if (rv != APR_SUCCESS) {
    ERR("Failed to receive packet after ACK.");
    return machine.state = END;
  }
  DBG("Recv packet len: %lu", len);

  machine.pack = tftp_packet_read(machine.buf, len, machine.mp);
  machine.event = machine.pack->opcode;
  machine.state = RECV;

  DBG("Event: %s, State: %s", opcode_str[machine.event], state_str[machine.state]);
  return machine.state;
}

