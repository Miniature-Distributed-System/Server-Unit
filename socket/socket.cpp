//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

//------------------------------------------------------------------------------
//
// Example: WebSocket server, asynchronous
//
//------------------------------------------------------------------------------

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "../include/debug_rp.hpp"
#include "../include/packet.hpp"
#include "../include/logger.hpp"
#include "../sink/sink_stack.hpp"
#include "../packet_processor/packet_validator.hpp"
#include "../worker_node/worker_registry.hpp"
#include "../packet_processor/packet_constructor.hpp"
#include "prevalidate_json.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Echoes back all received WebSocket messages
class session : public std::enable_shared_from_this<session>
{
    websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;

public:
    // Take ownership of the socket
    explicit session(tcp::socket &&socket)
        : ws_(std::move(socket))
    {
    }

    // Get on the correct executor
    void
    run()
    {
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch(ws_.get_executor(),
                      beast::bind_front_handler(
                          &session::on_run,
                          shared_from_this()));
    }

    // Start the asynchronous operation
    void
    on_run()
    {
        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::server));

        // Set a decorator to change the Server of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type &res)
            {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-server-async");
            }));
        // Accept the websocket handshake
        ws_.async_accept(
            beast::bind_front_handler(
                &session::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec)
    {
        if (ec)
            return fail(ec, "accept");
        do_read();
    }

    void
    do_read()
    {
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void
    on_read(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        std::string workerUid;
        Flag isNewWorker;
        Worker *worker = NULL;
        boost::ignore_unused(bytes_transferred);

        // This indicates that the session was closed
        if (ec == websocket::error::closed)
            return;

        if (ec)
            return fail(ec, "read");

        // Echo the message
        std::string out(boost::asio::buffer_cast<const char *>(buffer_.data()),
                        buffer_.size());
        Log().info(__func__,"Read message from client:-", out);
        JsonPrevalidator jsonPrevalidator(out);
        isNewWorker.initFlag(false);
        if (jsonPrevalidator.validateJson())
        {
            json packet = jsonPrevalidator.getJson();
            if(packet["id"].empty()){
                Log().info(__func__, " id feild empty");
                workerUid = globalWorkerRegistry.generateWorkerUid();
                isNewWorker.setFlag();
            } else {
                worker = globalWorkerRegistry.getWorkerFromUid(packet["id"]);
                if(worker == NULL){
                    Log().info(__func__, "could not find the worker:", packet["id"], " in worker list");
                    workerUid = globalWorkerRegistry.generateWorkerUid();
                    isNewWorker.setFlag();
                } else {
                    Log().info(__func__, "worker:", packet["id"], " successfully identified");
                    if(jsonPrevalidator.checkQuickSendBit()) 
                        worker->setQuickSendMode();
                    else worker->resetQuickSendMode();
                    JsonExport *jsonExportObject = new JsonExport(packet);
                    globalReceiverSink->pushObject(jsonExportObject, DEFAULT_PRIORITY);
                    //Schedule a packet processor for every packet in receiver sink
                    init_validator();
                }
            }     
        } else {
            isNewWorker.setFlag();
        }
        buffer_.consume(buffer_.size());
        json outPacket;
        if(isNewWorker.isFlagSet()){
            outPacket = PacketConstructor().create(SP_HANDSHAKE, workerUid);
        } else {
            Log().info(__func__, "wait for packet qsend:", worker->isQuickSendMode());
            while(outPacket.empty()){
                outPacket = worker->getQueuedPacket();
                if(outPacket.empty() && worker->isQuickSendMode()){
                    outPacket = PacketConstructor().create(SP_HANDSHAKE, worker->getWorkerUID());
                    break;
                }
            }
            Log().info(__func__, "Worker: ", worker->getWorkerUID(), " :has packet scheduled ", outPacket);
        }
        ws_.text(ws_.got_text());
        ws_.write(net::buffer(std::string(outPacket.dump())));
        ws_.async_write(
            net::buffer(outPacket.dump()),
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void
    on_write(
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail(ec, "write");

        // Clear the buffer
        buffer_.consume(buffer_.size());

        // Do another read
        do_read();
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context &ioc_;
    tcp::acceptor acceptor_;

public:
    listener(
        net::io_context &ioc,
        tcp::endpoint endpoint)
        : ioc_(ioc), acceptor_(ioc)
    {
        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if (ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if (ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if (ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if (ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

private:
    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }

    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        if (ec)
        {
            fail(ec, "accept");
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(std::move(socket))->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int startSocket(std::string inputAddress, int portNumber, int totalThreads)
{
    auto const address = net::ip::make_address(inputAddress);
    auto const port = static_cast<unsigned short>(portNumber);
    auto const threads = std::max<int>(1, totalThreads);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // Create and launch a listening port
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    return EXIT_SUCCESS;
}
