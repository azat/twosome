
/**
 * This file is part of the twosome package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "session.h"

#include <boost/asio/write.hpp>
#include <functional>


namespace PlaceHolders = std::placeholders;
namespace Asio = boost::asio;


Session::Session(boost::asio::io_service& socket)
    : m_socket(socket)
{
}

void Session::start()
{
    m_room.join(shared_from_this());
    asyncRead();
}

void Session::deliver(const std::string& message)
{
    asyncWrite(message);
}

void Session::asyncRead()
{
    m_socket.async_read_some(Asio::buffer(m_buffer, MAX_BUFFER_LENGTH),
                             std::bind(&Session::handleRead, this,
                                       PlaceHolders::_1,
                                       PlaceHolders::_2));
}

void Session::asyncWrite(const std::string& message)
{
    Asio::async_write(m_socket,
                      Asio::buffer(message),
                      std::bind(&Session::handleWrite, this,
                                PlaceHolders::_1));
}

void Session::handleRead(const boost::system::error_code& error,
                         size_t bytesTransferred __attribute__((unused)))
{
    if (error) {
        m_room.leave(shared_from_this());
        return;
    }

    m_room.deliver(m_buffer);
    asyncWrite(m_buffer);
}

void Session::handleWrite(const boost::system::error_code& error)
{
    if (error) {
        m_room.leave(shared_from_this());
        return;
    }

    asyncRead();
}