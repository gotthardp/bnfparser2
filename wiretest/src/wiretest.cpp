/*
 * bnfparser2 - Generic BNF-adaptable parser
 * http://bnfparser2.sourceforge.net
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      version 2, as published by the Free Software Foundation.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 * Copyright (c) 2007 ANF DATA spol. s r.o.
 *
 * $Id$
 */

#include <stdexcept>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include <nids.h>

#include "config.h"

void udp_callback(struct tuple4 *addr, u_char *data, int len, struct ip *pkt)
{
  fprintf(stderr, "%s:%i ->%s:%i: UDP data\n",
    inet_ntoa(*(struct in_addr*)&(addr->saddr)), addr->source,
    inet_ntoa(*(struct in_addr*)&(addr->daddr)), addr->dest);

  // *** received data stored in data[len]
  // TODO: Integrate with the parser library
  write(2,data,len);
}

int handle_tcp_connect(struct tcp_stream *ts)
{
  fprintf(stderr, "%s:%i - %s:%i: TCP connection established\n",
    inet_ntoa(*(struct in_addr*)&(ts->addr.saddr)), ts->addr.source,
    inet_ntoa(*(struct in_addr*)&(ts->addr.daddr)), ts->addr.dest);

  // collect client and server data
  ts->client.collect++;
  ts->server.collect++;

  return 0;
}

int handle_tcp_close(struct tcp_stream *ts)
{
  fprintf(stderr, "%s:%i - %s:%i: TCP connection closed\n",
    inet_ntoa(*(struct in_addr*)&(ts->addr.saddr)), ts->addr.source,
    inet_ntoa(*(struct in_addr*)&(ts->addr.daddr)), ts->addr.dest);

  return 0;
}

int handle_tcp_reset(struct tcp_stream *ts)
{
  fprintf(stderr, "%s:%i - %s:%i: TCP connection reset\n",
    inet_ntoa(*(struct in_addr*)&(ts->addr.saddr)), ts->addr.source,
    inet_ntoa(*(struct in_addr*)&(ts->addr.daddr)), ts->addr.dest);

  return 0;
}

int handle_tcp_client_data(struct tcp_stream *ts, struct half_stream *hlf)
{
  fprintf(stderr, "%s:%i<- %s:%i: TCP data for client\n",
    inet_ntoa(*(struct in_addr*)&(ts->addr.saddr)), ts->addr.source,
    inet_ntoa(*(struct in_addr*)&(ts->addr.daddr)), ts->addr.dest);

  // *** received data stored in hlf->data[hlf->count_new]
  // TODO: Integrate with the parser library
  write(2,hlf->data,hlf->count_new);

  return 0;
}

int handle_tcp_server_data(struct tcp_stream *ts, struct half_stream *hlf)
{
  fprintf(stderr, "%s:%i ->%s:%i: TCP data for server\n",
    inet_ntoa(*(struct in_addr*)&(ts->addr.saddr)), ts->addr.source,
    inet_ntoa(*(struct in_addr*)&(ts->addr.daddr)), ts->addr.dest);

  // *** received data stored in hlf->data[hlf->count_new]
  // TODO: Integrate with the parser library
  write(2,hlf->data,hlf->count_new);

  return 0;
}

void tcp_callback(struct tcp_stream *ts, void **param)
{
  switch(ts->nids_state)
  {
  case NIDS_JUST_EST:
    // connection has been established
    handle_tcp_connect(ts);
    break;

  case NIDS_CLOSE:
    // connection has been closed normally
    handle_tcp_close(ts);
    break;

  case NIDS_RESET:
    // connection has been closed by RST
    handle_tcp_reset(ts);
    break;

  case NIDS_DATA:
    if (ts->client.count_new)
    {
      // new data for client
      handle_tcp_client_data(ts, &ts->client);
    }
    else
    {
      // new data for server
      handle_tcp_server_data(ts, &ts->server);
    }
    break;
  }
}

/* Configures nids to not perform a checksum check for
 * packets sent from local interfaces.
 */
static int set_ignore_local_checksums()
{
  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (sock <= 0)
    throw std::runtime_error("socket error");

  // step 1: obtain list of available interfaces
  int ifaces_size = 8;
  struct ifreq *ifaces = new struct ifreq[ifaces_size];

  struct ifconf param;
  while(1)
  {
    param.ifc_len = ifaces_size*sizeof(struct ifreq);
    param.ifc_req = ifaces;

    if (ioctl(sock, SIOCGIFCONF, &param))
      throw std::runtime_error("ioctl error");
    // success
    if (param.ifc_len < ifaces_size*sizeof(struct ifreq))
      break;
    // not enough memory; allocate 2x more and try again
    delete[] ifaces;
    ifaces_size *= 2;
    ifaces = new struct ifreq[ifaces_size];
  }

  close(sock);
  int ifaces_count = param.ifc_len / sizeof(struct ifreq);

  if( ifaces_count <= 0 )
    return 0;

  // step 2: register local addresses to not make checksums
  struct nids_chksum_ctl *ctlptr = new struct nids_chksum_ctl[ifaces_count];
  int nctl = 0;

  for (int i = 0; i < ifaces_count; i++)
  {
    if (ifaces[i].ifr_addr.sa_family != AF_INET)
      continue;

    ctlptr[nctl].netaddr = ((struct sockaddr_in *) &(ifaces[i].ifr_addr))->sin_addr.s_addr;
    ctlptr[nctl].mask = (in_addr_t)0xffffffff;
    ctlptr[nctl].action = NIDS_DONT_CHKSUM;
    nctl++;
  }

  delete[] ifaces;
  nids_register_chksum_ctl(ctlptr, nctl);

  return 0;
}

void usage( const char* program_name, const int status )
{
  printf( "Usage: %s [OPTION]...\n", program_name );

  if( status != 0 )
    fprintf( stderr, "Try '%s --help' for more information.\n", program_name );
  else
  {
    printf(
      "Check the traffic on a network against a BNF syntax specification.\n"
      "See http://bnfparser2.sourceforge.net for more information.\n"
      "\n"
      "  -i <if>, --interface=<if>     listen on interface (by default 'any')\n"
      "  -r <infile>, --read=<infile>  read packets from a .pcap file\n"
      "  --help                        display this help and exit\n"
      "\n"
      "Report bugs to <%s>.\n", PACKAGE_BUGREPORT );
  }

  exit( status );
}

static struct option const long_options[] =
{
  { "interface", required_argument, NULL, 'i' },
  { "read", required_argument, NULL, 'r' },
  { "help", no_argument, NULL, 'h' },
  { NULL, 0, NULL, 0 }
};

int main(int argc, char **argv)
{
  int optc;
  const char* program_name = argv[0];

  while ((optc = getopt_long (argc, argv, "i:f:h", long_options, NULL)) != -1)
  {
    switch (optc)
    {
    case 'i':
      nids_params.device = optarg;
      break;
    case 'r':
      nids_params.filename = optarg;
      break;
    case 'h':
      usage(program_name, EXIT_SUCCESS);
      break;
    default:
      usage(program_name, 1);
    }
  }

  if (!nids_init())
  {
    fprintf(stderr,"%s\n",nids_errbuf);
    exit(1);
  }	

  set_ignore_local_checksums();
  nids_register_udp( (void(*))udp_callback );
  nids_register_tcp( (void(*))tcp_callback );

  int fd = nids_getfd();
  while(1)
  {
    fd_set rset;
    FD_ZERO (&rset);

    FD_SET (fd, &rset);

    if (select(fd+1, &rset, 0, 0, NULL))
    {
      if (FD_ISSET(fd,&rset))
      {
        if (!nids_next()) break;
      }
    }
  }

  return 0;
}

// end of file

