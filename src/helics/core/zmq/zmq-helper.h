/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _ZMQHELPER_H_
#define _ZMQHELPER_H_

#include <stdint.h> /* for uint8_t, uint32_t */

#include <cassert> /* for assert */
#include <cstdlib> /* for size_t */
#include <string>

#include <zmq.h>

#define ZMQ_RECV_MAX 256

typedef void(*zmqx_sigfunc)(void*);

#ifndef _WIN32

// Signal handling
//
// Call zmqx_catch_signals() in your application at startup, and then
// exit your main loop if _zmqx_interrupted is ever 1. Works especially
// well with zmq_poll.

#include <csignal> /* for sigaction */


extern zmqx_sigfunc _zmqx_sigfunc;
extern int   _zmqx_sigfunc_context;
extern void *_zmqx_sigfunc_object;
extern int _zmqx_interrupted;

void zmqx_register_handler(zmqx_sigfunc function, int context, void *object);

void zmqx_signal_handler (int signal_value);

void zmqx_catch_signals (void);

#endif

void zmqx_interrupt_check();

int zmqx_recv(void *socket, std::string &buf);

int zmqx_recv(void *socket, uint8_t* &buf, uint32_t buf_size);

int zmqx_poll(zmq_pollitem_t *socks,uint32_t buff_size);

template <typename T>
int zmqx_recv(void *socket, T &buf)
{
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), 0);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_irecv(void *socket, std::string &buf);

template <typename T>
int zmqx_irecv(void *socket, T &buf)
{
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
    assert(size == sizeof(T)
            || (size == -1 && errno == EAGAIN)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_send(void *socket, const std::string &s);

int zmqx_send(void *socket, uint8_t *buf, uint32_t buf_size);

template <typename T>
int zmqx_send(void *socket, const T &what)
{
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, &what, sizeof(T), 0);
      assert(size == sizeof(T)
	      || (size == -1 && errno == EINTR)
	    );

      zmqx_interrupt_check();
      //if we are here we ignored the signal.
    }while(size <0);
    return size;
}

int zmqx_sendmore(void *socket, const std::string &s);

int zmqx_sendmore(void *socket, uint8_t *buf, uint32_t buf_size);

template <typename T>
int zmqx_sendmore(void *socket, const T &what)
{
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
 
      assert(size == sizeof(T)
	      || (size == -1 && errno == EINTR)
	    );

      zmqx_interrupt_check();
      //if we loop again then we received a signal that we ignore.
    }while(size < 0);
    return size;
}

int zmqx_send(void *socket, int context, const std::string &command);

template <typename T>
int zmqx_send(void *socket, int context, const std::string &command, const T &t)
{
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);
    totalSize += zmqx_send    (socket, t);

    zmqx_interrupt_check();
    return totalSize;
}

int zmqx_sendmore(void *socket, int context, const std::string &command);

template <typename T>
int zmqx_sendmore(void *socket, int context, const std::string &command, const T &t)
{
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);
    totalSize += zmqx_sendmore(socket, t);

    zmqx_interrupt_check();
    return totalSize;
}

#endif /* _ZMQHELPER_H_ */
