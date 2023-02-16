#include "service.h"
#include <iostream>


Service::Service()
{
    test = "test"; // testing UI widgets
}



// #include <iostream>
// #include <boost/asio.hpp>

// using namespace boost::asio;
// using ip::tcp;
// using std::string;
// using std::cout;
// using std::endl;

// int main() {
//     try {
//         boost::asio::io_service io_service;
//         //socket creation
//         tcp::socket socket(io_service);
//         //connection
//         socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 8080 ));
//         // request/message from client
//         const string msg = "Hello from Client!\n";
//         boost::system::error_code error;
//         boost::asio::write( socket, boost::asio::buffer(msg), error );
//         if( !error ) {
//             cout << "Client sent hello message!" << endl;
//         }
//         else {
//             cout << "send failed: " << error.message() << endl;
//         }
//         // getting response from server
//         boost::asio::streambuf receive_buffer;
//         boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
//         if( error && error != boost::asio::error::eof ) {
//             cout << "receive failed: " << error.message() << endl;
//         }
//         else {
//             const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
//             cout << data << endl;
//         }
//         return 0;
//     } catch(std::exception const& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }

// }





// #include <boost/beast/core.hpp>
// #include <boost/beast/websocket.hpp>
// #include <boost/asio/connect.hpp>
// #include <boost/asio/ip/tcp.hpp>
// #include <cstdlib>
// #include <iostream>
// #include <string>

// namespace beast = boost::beast;         // from <boost/beast.hpp>
// namespace http = beast::http;           // from <boost/beast/http.hpp>
// namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
// namespace net = boost::asio;            // from <boost/asio.hpp>
// using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// // Sends a WebSocket message and prints the response
// int main(int argc, char** argv)
// {
//     try
//     {
//         auto const host = "127.0.0.1";
//         auto const port = "8080";
//         auto const text = "Hello, world!";

//         // The io_context is required for all I/O
//         net::io_context ioc;

//         // These objects perform our I/O
//         tcp::resolver resolver{ioc};
//         websocket::stream<tcp::socket> ws{ioc};

//         // Look up the domain name
//         auto const results = resolver.resolve(host, port);
          
//         // Make the connection on the IP address we get from a lookup
//         net::connect(ws.next_layer(), results.begin(), results.end());

//         /*
//         ws.next_layer().handshake(ssl::stream_base::client);
//         */

//         // Set a decorator to change the User-Agent of the handshake
//         ws.set_option(websocket::stream_base::decorator(
//             [](websocket::request_type& req)
//             {
//                 req.set(http::field::user_agent,
//                     std::string(BOOST_BEAST_VERSION_STRING) +
//                         " websocket-client-coro");
//             }));

//         // Perform the websocket handshake
//         ws.handshake(host, "/");

//         // Send the message
//         ws.write(net::buffer(std::string(text)));

//         // This buffer will hold the incoming message
//         beast::flat_buffer buffer;

//         // Read a message into our buffer
//         ws.read(buffer);

//         // Close the WebSocket connection
//         ws.close(websocket::close_code::normal);

//         // If we get here then the connection is closed gracefully

//         // The make_printable() function helps print a ConstBufferSequence
//         std::cout << beast::make_printable(buffer.data()) << std::endl;
//     }
//     catch(std::exception const& e)
//     {
//         std::cerr << "Error: " << e.what() << std::endl;
//         return EXIT_FAILURE;
//     }
//     return EXIT_SUCCESS;
// }