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

#include <apr_network_io.h>
#include <apr_file_io.h>
#include <stdio.h> // to del

int main(int argc, const char *argv[])
{
  apr_pool_t *mp;

  apr_initialize();
  apr_pool_create(&mp, NULL);

//----------------------------------------------------------------
//----------------------------------------------------------------

  apr_status_t rv;
  apr_socket_t *sock;
  apr_sockaddr_t *sockaddr;
  apr_size_t len;
  tftp_pack *pack;
  unsigned int port  = 0;
  unsigned int block = 0;
  apr_file_t *fp;
  char *filename = "bootstrap.js";
  char *fileout = "/tmp/tftp.out";
  apr_size_t buf_size = DATA_SIZE + 4;
  char *buf = apr_palloc(mp, buf_size);

  rv = apr_sockaddr_info_get(&sockaddr, "127.0.0.1", APR_INET, 69, 0, mp);
  rv = apr_socket_create(&sock, sockaddr->family, SOCK_DGRAM, APR_PROTO_UDP, mp);
  rv = apr_file_open (&fp, fileout,
      APR_FOPEN_CREATE|APR_FOPEN_WRITE|APR_FOPEN_TRUNCATE,
      APR_OS_DEFAULT, mp);
  if (rv != APR_SUCCESS) {
    printf("ERROR: Failed open file.\n");
    return rv;
  }

  struct pack_rq rrq = {
    .filename = filename,
    .len_filename = strlen(filename),
    .mode = MODE_ASCII,
    .len_mode = strlen(MODE_ASCII),
    .e_mode = E_ASCII
  };
  len = tftp_create_rrq (buf, &rrq);
  printf("SEND: RRQ to 127.0.0.1:69/%s mode: %s\n", rrq.filename, rrq.mode);
  rv = apr_socket_sendto (sock, sockaddr, 0, buf, &len);
  len = buf_size;
  rv = apr_socket_recvfrom (sockaddr, sock, 0, buf, &len);
  apr_socket_addr_get(&sockaddr, APR_REMOTE, sock);
  port = sockaddr->port;
  printf("      received from port: %d; packet length: %d\n", port, len);
  pack = tftp_packet_read(buf, len, mp);
  block = pack->data->data.block;
  len = tftp_str_ntoh (mp, pack->data->data.data, pack->data->data.length);

  rv = apr_file_write(fp, pack->data->data.data, &len);

  printf("RECV: packet: %d\n", pack->opcode);

  do {
    len = tftp_create_ack (buf, block);
    printf("SEND: ACK block: %d\n", block);
    rv = apr_socket_sendto (sock, sockaddr, 0, buf, &len);
    len = buf_size;
    rv = apr_socket_recvfrom (sockaddr, sock, 0, buf, &len);
    pack = tftp_packet_read(buf, len, mp);
    block = pack->data->data.block;

    len = tftp_str_ntoh (mp, pack->data->data.data, pack->data->data.length);
    rv = apr_file_write(fp, pack->data->data.data, &len);

    printf("RECV: packet: %d; length: %d\n", pack->opcode, pack->data->data.length);
  } while(pack->data->data.length == 512);

  len = tftp_create_ack (buf, block);
  printf("SEND: ACK block: %d\n", block);
  rv = apr_socket_sendto (sock, sockaddr, 0, buf, &len);
  printf("      done x-fer.\n");

  apr_file_close(fp);
//----------------------------------------------------------------
//----------------------------------------------------------------

  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
