/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "serial_processor.hpp"

#include "xrt/xrt.h"
#include "xrt/experimental/xrt_kernel.h"

class Server 
{
public:
  Server(boost::asio::io_context& io_context, unsigned short port)
   : io_context_(io_context),
     endpoint_(boost::asio::ip::address_v4::any(), port),
     acceptor_(io_context_)
  {
       port_ = port;
  }
  void start();
private:
  void start_accept();
  void handle_accept(serial_processor* sp, const boost::system::error_code& error);

  unsigned short port_;
  boost::asio::io_context& io_context_;
  boost::asio::ip::tcp::endpoint endpoint_;
  boost::asio::ip::tcp::acceptor acceptor_;
};
