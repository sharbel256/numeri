#include <iostream>
#include <string>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio;

int main()
{
    // Create an io_context to manage the socket
    io_context ioc;

    // Create an endpoint using an IP and port
    ip::tcp::endpoint endpoint(ip::tcp::v4(), 8080);

    // Create a socket and bind it to the endpoint
    ip::tcp::acceptor acceptor(ioc, endpoint);

    // Wait for an incoming connection
    ip::tcp::socket socket(ioc);
    acceptor.accept(socket);

    // Read data from the socket
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, "\r\n");

    // Convert the read data to a string
    string request_string = boost::asio::buffer_cast<const char*>(buffer.data());
    cout << "Received request: " << request_string << endl;

    // Write a response to the socket
    string response_string = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    write(socket, boost::asio::buffer(response_string));

    return 0;
}
