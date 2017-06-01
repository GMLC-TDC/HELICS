/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"

#include <cassert> /* for assert */
#include <cstdlib> /* for size_t */
#include <string>

#include <zmq.h>

#include "helics/core/zmq/zmq-helper.h"
static int _zmqx_interrupted_in_process = 0;
int   _zmqx_sigfunc_context = 0;
void *_zmqx_sigfunc_object = NULL;
int _zmqx_interrupted = 0;

zmqx_sigfunc _zmqx_sigfunc=NULL;

#ifndef _WIN32
void zmqx_register_handler(zmqx_sigfunc function, int context, void *object)
{
    _zmqx_sigfunc = function;
    _zmqx_sigfunc_context = context;
    _zmqx_sigfunc_object = object;
    //reset signal checks
    _zmqx_interrupted_in_process=0;
    _zmqx_interrupted=0;
}


void zmqx_signal_handler (int signal_value)
{
    _zmqx_interrupted = 1;
}


void zmqx_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = zmqx_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}

#endif

void zmqx_interrupt_check()
{
/* check for interrupt but don't recheck if already processing interrupt */
    if (_zmqx_interrupted && !_zmqx_interrupted_in_process) {
        _zmqx_interrupted_in_process = 1;
        if (_zmqx_sigfunc != NULL) {
            _zmqx_sigfunc(_zmqx_sigfunc_object);
        }
    }
}


int zmqx_recv(void *socket, std::string &buf) {
    char buffer [ZMQ_RECV_MAX];
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv (socket, buffer, ZMQ_RECV_MAX-1, 0);
    if (size >= 0) {
        if (size > ZMQ_RECV_MAX-1) {
            size = ZMQ_RECV_MAX-1;
        }
        buffer[size] = 0;
        buf.assign(buffer);
    }
    else if (size == -1) {
        assert(errno == EINTR);
    }
    else {
        assert(0);
    }

    zmqx_interrupt_check();
    return size;
}


int zmqx_recv(void *socket, uint8_t* &buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    buf = new uint8_t[buf_size];
    size = zmq_recv(socket, buf, buf_size, 0);
    assert((size >= 0 && uint32_t(size) <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_irecv(void *socket, std::string &buf) {
    char buffer [ZMQ_RECV_MAX];
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, buffer, ZMQ_RECV_MAX-1, ZMQ_DONTWAIT);
    if (size >= 0) {
        if (size > ZMQ_RECV_MAX-1) {
            size = ZMQ_RECV_MAX-1;
        }
        buffer[size] = 0;
        buf.assign(buffer);
    }
    else if (size == -1) {
        assert(errno == EINTR || errno == EAGAIN);
        buf.clear();
    }
    else {
        assert(0);
    }

    zmqx_interrupt_check();
    return size;
}


int zmqx_send(void *socket, const std::string &s) {
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, s.data(), s.size(), 0);
  
      assert((size >= 0 && size_t(size) == s.size())
	      || (size == -1 && errno == EINTR)
	      );

      zmqx_interrupt_check();
    }while(size < 0);
    return size;
}


int zmqx_send(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, buf, buf_size, 0);
  
    assert((size >= 0 && uint32_t(size) <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_sendmore(void *socket, const std::string &s) {
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
      assert((size >= 0 && size_t(size) == s.size())
	      || (size == -1 && errno == EINTR)
	      );

      zmqx_interrupt_check();
      //if we are here then we ignored the signal.
    }while(size < 0);
    return size;
}


int zmqx_sendmore(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, buf, buf_size, ZMQ_SNDMORE);
  
    assert((size >= 0 && uint32_t(size) == buf_size)
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_send(void *socket, int context, const std::string &command) {
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_send    (socket, command);

    zmqx_interrupt_check();
    return totalSize;
 
}


int zmqx_sendmore(void *socket, int context, const std::string &command) {
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);

    zmqx_interrupt_check();
    return totalSize;
}

int zmqx_poll(zmq_pollitem_t* socks, uint32_t buff_size)
{

   zmqx_interrupt_check();
   int rc = zmq_poll(socks, buff_size, 0);
   assert(rc >= 0
            || (rc == -1 && errno == EINTR)
            );
   zmqx_interrupt_check();
   return rc;
}

