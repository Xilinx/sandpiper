/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <cstdlib>
#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "server.hpp"
#include "serial_processor.hpp"

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void Server::start()
{
    std::cout << "Starting complex matrix mult server on port " << port_ << std::endl;

    /*
     * Open the acceptor socket
     */
    acceptor_.open(endpoint_.protocol());

    /*
     * Bind the acceptor socket to the endpoint
     */
    acceptor_.bind(endpoint_);

    /*
     * Listen for incoming connection requests
     */
    acceptor_.listen(boost::asio::socket_base::max_connections);

    start_accept();
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void Server::start_accept()
{
    std::cout << "Accepting connection" << std::endl;

    /*
     * The serial_processor owns the socket
     */
    serial_processor* sp = new serial_processor(io_context_);
    acceptor_.async_accept(sp->get_socket(),
        boost::bind(&Server::handle_accept, this, sp,
          boost::asio::placeholders::error));
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void Server::handle_accept(serial_processor* sp, const boost::system::error_code& error)
{
    if (!error)
    {
      std::cout << "Start new connection" << std::endl;
      sp->start();
    }
    else
    {
      std::cout << "handle_accept:  Delete connection" << std::endl;
      delete sp;
    }
}
