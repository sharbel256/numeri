#include <iostream>
#include <cpprest/http_listener.h>

using namespace std;
using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

int main()
{
    // Specify the URI to listen to
    http_listener listener(U("http://localhost:8080/"));

    // Register a request handler
    listener.support(methods::GET, [](http_request request) {
        // Handle the request and return a response
        http_response response(status_codes::OK);
        response.set_body("Hello, World!");
        request.reply(response);
    });

    // Start the listener
    try {
        listener
            .open()
            .then([&listener]() { cout << "Listening for requests at: " << listener.uri().to_string() << endl; })
            .wait();
    }
    catch (exception const & e) {
        cout << e.what() << endl;
    }

    // Wait for user input
    cout << "Press Enter to exit." << endl;
    cin.ignore();
    listener.close();
    return 0;
}
