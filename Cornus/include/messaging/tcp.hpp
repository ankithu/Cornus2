/****
 * Base socket code adapted from: https://github.com/eecs482/bgreeves-socket-example
 * Provides TCPServer and TCPClient classes that allow asynchronous communication between nodes.
 *
 * Create TCPServer as follows:
 * TCPServer server(port, queue_size);
 *
 * Provides "endpoint" abstraction to allow different message types to be handled by different callbacks
 *
 * A callback function should take in a TCPRequest& and return a TCPResponse.
 *
 * Write a callback like this:
 * TCPResponse handler(const TCPRequest& req){
 *     TCPResponse res;
 *     res.response = "some response";
 *     return res;
 * }
 *
 * Register the callback with the server like this:
 * server.registerCallback("endpoint", handler);
 *
 * Run the server like this:
 * server.runServer();
 *
 * Now from a client you can send request to the server.
 *
 * Make a TCPClient like this:
 * TCPClient client(hostname, port);
 *
 * client objects are not copyable and should not be shared across threads unless synchronized to not make simulatenous requests.
 *
 * However you can make multiple client objects to make simultaneous requests. (And multiple clients can connect to the same server)
 *
 * Send a request like this:
 * TCPRequest req;
 * req.endpoint = "endpoint";
 * req.request = "some request";
 * std::optional<TCPResponse> res = client.sendRequest(req);
 *
 * res will be nullopt if the request failed for some reason
 * otherwise you can access TCPResposnse fields with either the -> operator or by dereferencing the optional.
 *
 * The call to sendRequest is blocking and will wait until a response is received.
 * For asynchronous requests use sendRequestAsync.
 *
 * Example:
 * TCPRequest req;
 * req.endpoint = "endpoint";
 * req.request = "some request";
 * std::future<std::optional<TCPResponse>> future = client.sendRequestAsync(req);
 * //do other stuff
 * //get response
 * auto res = future.get();
 *
 * Additionally, static functions for both sync and async calls are provided, these static functions
 * construct (and destruct) a new TCPClient object for each call so the connection will be torn down upon completion.
 *
 * The policy is set so the sendRequestAsync will immediately launch a thread to send the request.
 *
 *****/

#ifndef CORNUS_TCP_HPP
#define CORNUS_TCP_HPP

#include <string>
#include <unordered_map>
#include <thread>

#include <arpa/inet.h>  // htons(), ntohs()
#include <netdb.h>      // gethostbyname(), struct hostent
#include <netinet/in.h> // struct sockaddr_in
#include <stdio.h>      // perror(), fprintf()
#include <string.h>     // memcpy()
#include <sys/socket.h> // getsockname()
#include <unistd.h>     // stderr

#include <iostream>
#include <future>
#include <sstream>
#include <functional>
#include <assert.h>

static const size_t MAX_MESSAGE_SIZE = 512;
static const char HEADER_DELIM = '\t';
static const char MESSAGE_END = '\n';

using ParamsT = std::unordered_map<std::string, std::string>;

class TCPRequest
{
public:
    std::string endpoint;
    std::string request;

private:
    std::unordered_map<std::string, std::string> params;

public:
    TCPRequest(std::string &&endpoint, std::string &&request) : endpoint(endpoint), request(request)
    {
        std::stringstream ss(request);
        std::string pair;
        while (std::getline(ss, pair, ';'))
        {
            if (pair.find(':') == std::string::npos)
            {
                break;
            }
            std::stringstream pss(pair);
            std::string key, val;
            std::getline(pss, key, ':');
            std::getline(pss, val, ':');
            params[key] = val;
        }
    }

    TCPRequest(const std::string &endpoint, const std::string &request) : endpoint(endpoint), request(request)
    {
        std::stringstream ss(request);
        std::string pair;
        while (std::getline(ss, pair, ';'))
        {
            if (pair.find(':') == std::string::npos)
            {
                break;
            }
            std::stringstream pss(pair);
            std::string key, val;
            std::getline(pss, key, ':');
            std::getline(pss, val, ':');
            params[key] = val;
        }
    }

    TCPRequest(const std::string &endpoint, const ParamsT &params) : endpoint(endpoint), params(params)
    {
        writeParams();
    }

    TCPRequest(std::string &&endpoint, ParamsT &&params) : endpoint(std::move(endpoint)), params(std::move(params))
    {
        writeParams();
    }

    void setParam(const std::string &param, const std::string &value)
    {
        params[param] = value;
        writeParams();
    }

    std::string getParam(const std::string &param) const
    {
        auto itr = params.find(param);
        if (itr != params.end())
        {
            return itr->second;
        }
        std::cout << "PARAM IN QUESTION " << param << std::endl;
        assert(false);
        return "";
    }

    friend std::ostream &operator<<(std::ostream &os, const TCPRequest &req)
    {
        os << "TCP Request = { endpoint = \"" << req.endpoint << "\", request = \"" << req.request << "\", params: {";
        for (auto &[param, val] : req.params)
        {
            os << "{ param: " << param << ", val: \"" << val << "\" }, ";
        }
        os << "}}";
        return os;
    }

private:
    void writeParams()
    {
        request = "";
        for (auto &[param, val] : params)
        {
            request += param + ":" + val + ";";
        }
    }
};

struct TCPResponse
{
    std::string response;
};

static const TCPResponse TCP_OK = TCPResponse{"OK"};

std::ostream &operator<<(std::ostream &os, const TCPResponse &res)
{
    os << "TCP Response = { response = \"" << res.response << "\"}";
    return os;
}

using CallbackT = std::function<TCPResponse(TCPRequest &)>;
using sockaddr_in = struct sockaddr_in;

bool trySendData(int sockfd, char *sendbuffer, size_t message_len)
{
    // std::cout << "trying to send response" << std::endl;
    //  Send message to remote server
    //  Call send() enough times to send all the data
    ssize_t sent = 0;
    do
    {
        const ssize_t n = send(sockfd, sendbuffer + sent, message_len - sent, 0);
        if (n == -1)
        {
            std::cout << "Error sending on stream socket" << std::endl;
            perror("Error sending on stream socket");
            return false;
        }
        sent += n;
    } while (sent < message_len);
    return true;
}

class TCPServer
{
private:
    /**
     * Returns a nullopt if msg does not contain a complete request, otherwise decodes it and returns
     * it as  TCPRequest.
     * Parameters:
     *      msg: a pointer to the buffer we are trying to decode
     *      size: size of msg
     *Returns:
     *      TCPRequest on success, nullopt on failure
     */
    std::optional<TCPRequest> getRequest(char *msg, size_t size)
    {
        std::optional<size_t> header_delim = std::nullopt;
        std::optional<size_t> message_end = std::nullopt;
        // std::cout << "trying to decode: ";
        for (size_t i = 0; i < size; ++i)
        {
            // std::cout << msg[i];
            if (msg[i] == HEADER_DELIM)
            {
                // std::cout << "got header delim" << std::endl;
                header_delim.emplace(i);
            }
            if (msg[i] == MESSAGE_END)
            {
                // std::cout << "got message end" << std::endl;
                message_end.emplace(i);
            }
        }
        // std::cout << std::endl;
        if (header_delim && message_end)
        {
            std::optional<TCPRequest> o(
                {TCPRequest(std::string(msg, *header_delim),
                            std::string(msg + *header_delim + 1, *message_end - *header_delim - 1))});
            return o;
        }
        else
        {
            return std::nullopt;
        }
    }

    ssize_t encodeIntoBuffer(const TCPResponse &resp, char *buffer)
    {
        if (resp.response.size() + 1 >= MAX_MESSAGE_SIZE)
        {
            return -1;
        }
        memcpy(buffer, resp.response.c_str(), resp.response.size());
        buffer[resp.response.size()] = MESSAGE_END;
        return resp.response.size() + 1;
    }

    /**
     * Make a server sockaddr given a port.
     */
    static sockaddr_in make_server_sockaddr(int port)
    {
        sockaddr_in addr;
        // Step (1): specify socket family.
        // This is an internet socket.
        addr.sin_family = AF_INET;
        // Step (2): specify socket address (hostname).
        // The socket will be a server, so it will only be listening.
        // Let the OS map it to the correct address.
        addr.sin_addr.s_addr = INADDR_ANY;
        // Step (3): Set the port value.
        // If port is 0, the OS will choose the port for us.
        // Use htons to convert from local byte order to network byte order.
        addr.sin_port = htons(port);
        return addr;
    }

    /**
     * Return the port number assigned to a socket.
     *
     * Parameters:
     * 		sockfd:	File descriptor of a socket
     *
     * Returns:
     *		The port number of the socket, or -1 on failure.
     */
    int get_port_number(int sockfd)
    {
        struct sockaddr_in addr;
        socklen_t length = sizeof(addr);
        if (getsockname(sockfd, (struct sockaddr *)&addr, &length) == -1)
        {
            perror("Error getting port of socket");
            return -1;
        }
        // Use ntohs to convert from network byte order to host byte order.
        return ntohs(addr.sin_port);
    }

    /**
     * Receives a string message from the client and prints it to stdout.
     *
     * Parameters:
     * 		connectionfd: 	File descriptor for a socket connection
     * 				(e.g. the one returned by accept())
     * Returns:
     *		0 on success, -1 on failure.
     */
    int handle_connection(int connectionfd)
    {

        char msg[MAX_MESSAGE_SIZE + 1];
        memset(msg, 0, sizeof(msg));

        // Call recv() enough times to consume all the data the client sends.
        ssize_t bytes_received;
        size_t bufferPointer = 0;
        do
        {
            // Receive as many additional bytes as we can in one call to recv()
            // (while not exceeding MAX_MESSAGE_SIZE bytes in total).
            // std::cout << "receiving bytes" << std::endl;
            bytes_received = recv(connectionfd, msg + bufferPointer, MAX_MESSAGE_SIZE - bufferPointer, 0);
            // std::cout << "received " << bytes_received << " bytes" << std::endl;
            if (bytes_received == -1)
            {
                perror("Error reading stream message");
                return -1;
            }
            bufferPointer += bytes_received;
            auto req = getRequest(msg, bufferPointer);
            if (req)
            {
                auto itr = callbacks.find(req->endpoint);
                if (itr == callbacks.end())
                {
                    // std::cout << "NO CALLBACK FOUND FOR " << *req << std::endl;
                }
                else
                {
                    auto &func = itr->second;
                    TCPResponse resp = func(*req);
                    char responseBuffer[MAX_MESSAGE_SIZE];
                    ssize_t message_size = encodeIntoBuffer(resp, responseBuffer);
                    if (message_size == -1)
                    {
                        perror("Could not encode response into buffer, perhaps to big.");
                        return -1;
                    }
                    if (!trySendData(connectionfd, responseBuffer, message_size))
                    {
                        perror("Couldn't send response");
                        return -1;
                    }
                }
                // get pointers and buffer ready for next message
                // adding 2 for the deliminators
                size_t messageSize = req->endpoint.size() + req->request.size() + 2;
                size_t extraInfoLen = bufferPointer - messageSize;
                memcpy(msg, msg + messageSize, extraInfoLen);
                bufferPointer = extraInfoLen;
            }

        } while (bytes_received > 0); // recv() returns 0 when client close

        close(connectionfd);
        // std::cout << "closing connection" << std::endl;
        return 0;
    }

    /**
     * Endlessly runs a server that listens for connections and serves
     * them _synchronously_.
     *
     * Parameters:
     *		port: 		The port on which to listen for incoming connections.
     *		queue_size: 	Size of the listen() queue
     * Returns:
     *		-1 on failure, does not return on success.
     */
    int run_server(int port, int queue_size)
    {

        // (1) Create socket
        int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
        if (sockfd == -1)
        {
            perror("Error opening stream socket");
            return -1;
        }

        // (2) Set the "reuse port" socket option
        int yesval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yesval, sizeof(yesval)) == -1)
        {
            perror("Error setting socket options");
            return -1;
        }

        // (3) Create a sockaddr_in struct for the proper port and bind() to it.
        sockaddr_in addr = make_server_sockaddr(port);

        // (3b) Bind to the port.
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("Error binding stream socket");
            return -1;
        }

        // (3c) Detect which port was chosen.
        port = get_port_number(sockfd);
        printf("Server listening on port %d...\n", port);

        // (4) Begin listening for incoming connections.
        if (listen(sockfd, queue_size) == -1)
        {
            perror("Error listening");
            return -1;
        }

        // (5) Serve incoming connections one by one forever.
        while (true)
        {
            int connectionfd = accept(sockfd, NULL, NULL);
            if (connectionfd == -1)
            {
                perror("Error accepting connection");
                return -1;
            }
            std::thread t([this, connectionfd]()
                          {
                //std::cout << "got connection, trying to handle." << std::endl;
                if (this->handle_connection(connectionfd) == -1){
                    return -1;
                }
                return 0; });
            t.detach();
        }
    }

public:
    TCPServer(int port, int queue_size) : port(port), queue_size(queue_size) {}

    int runServer()
    {
        if (run_server(port, queue_size) == -1)
        {
            return 1;
        }
        return 0;
    }

    void registerCallback(std::string endpoint, CallbackT callback)
    {
        callbacks[endpoint] = callback;
    }

private:
    int port;
    int queue_size;
    std::unordered_map<std::string, CallbackT> callbacks;
};

class TCPClient
{

private:
    /**
     * Attempts to make a client sockaddr given a remote hostname and port.
     * Parameters:
     *		hostname: 	The hostname of the remote host to connect to.
     *		port: 		The port to use to connect to the remote hostname.
     * Returns:
     *		addr on success, nullopt on failure.
     */
    std::optional<sockaddr_in> make_client_sockaddr(const std::string &hostname, int port)
    {

        sockaddr_in addr;

        // Step (1): specify socket family.
        // This is an internet socket.
        addr.sin_family = AF_INET;

        // Step (2): specify socket address (hostname).
        // The socket will be a client, so call this unix helper function
        // to convert a hostname string to a useable `hostent` struct.
        struct hostent *host = gethostbyname(hostname.c_str());
        if (host == NULL)
        {
            fprintf(stderr, "%s: unknown host\n", hostname.c_str());
            return std::nullopt;
        }
        memcpy(&addr.sin_addr, host->h_addr, host->h_length);

        // Step (3): Set the port value.
        // Use htons to convert from local byte order to network byte order.
        addr.sin_port = htons(port);

        return {addr};
    }

    ssize_t encodeIntoBuffer(const TCPRequest &req, char *buffer)
    {
        if (req.endpoint.size() + req.request.size() + 2 >= MAX_MESSAGE_SIZE)
        {
            return -1;
        }
        memcpy(buffer, req.endpoint.c_str(), req.endpoint.size());
        buffer[req.endpoint.size()] = HEADER_DELIM;
        memcpy(buffer + req.endpoint.size() + 1, req.request.c_str(), req.request.size());
        buffer[req.endpoint.size() + req.request.size() + 1] = MESSAGE_END;
        return req.endpoint.size() + req.request.size() + 2;
    }

    std::optional<TCPResponse> tryDecodeResponse(char *buffer, size_t len)
    {
        std::optional<TCPResponse> out = std::nullopt;
        // std::cout << "attempting to decode: ";
        for (size_t i = 0; i < len; ++i)
        {
            // std::cout << buffer[i];
            if (buffer[i] == MESSAGE_END)
            {
                // std::cout << "got message end" << std::endl;
                TCPResponse r;
                r.response = std::string(buffer, i);
                out.emplace(r);
                break;
            }
        }
        return out;
    }

public:
    TCPClient(const std::string &hostname, const int port)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

        // (2) Create a sockaddr_in to specify remote host and port
        std::optional<sockaddr_in> addrOpt = make_client_sockaddr(hostname, port);
        if (!addrOpt)
        {
            valid = false;
        }
        auto &addr = *addrOpt;

        // (3) Connect to remote server
        if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            perror("Error connecting stream socket");
            valid = false;
        }
    }

    TCPClient(const std::string &hostnamecolonport)
    {
        size_t delim = hostnamecolonport.find(":");
        std::string host = hostnamecolonport.substr(0, delim);
        int port = std::stoi(hostnamecolonport.substr(delim + 1));
        TCPClient(host, port);
    }

    ~TCPClient()
    {
        // (5) Close connection
        close(sockfd);
    }

    // do not copy TCP client objects
    TCPClient &operator=(const TCPClient &) = delete;
    TCPClient(const TCPClient &) = delete;

    std::optional<TCPResponse> sendRequest(TCPRequest &req)
    {
        std::cout << "Sending request: " << req.endpoint << " " << req.request << std::endl;
        if (!valid)
        {
            perror("Socket not initialized correctly! \n");
            return std::nullopt;
        }

        char buffer[MAX_MESSAGE_SIZE];
        ssize_t message_len = encodeIntoBuffer(req, buffer);
        if (message_len == -1)
        {
            perror("Error: Message exceeds maximum length\n");
            return std::nullopt;
        }
        std::cout << "Ready to send data" << std::endl;
        if (!trySendData(sockfd, buffer, message_len))
        {
            return std::nullopt;
        }

        // get response
        std::cout << "waiting on response" << std::endl;
        ssize_t bytes_received = 0;
        memset(buffer, 0, MAX_MESSAGE_SIZE);
        size_t bufferPointer = 0;
        while (true)
        {
            std::cout << "receiving bytes" << std::endl;
            bytes_received = recv(sockfd, buffer + bufferPointer, MAX_MESSAGE_SIZE - bufferPointer, 0);
            bufferPointer += bytes_received;
            // connection closed.
            if (bytes_received == 0)
            {
                return std::nullopt;
            }
            std::cout << "got " << bytes_received << " bytes" << std::endl;
            auto resp = tryDecodeResponse(buffer, bufferPointer);
            if (resp)
            {
                return *resp;
            }
        }
    }

    // sends request asynchronously, do not destroy client object until future is resolved
    std::future<std::optional<TCPResponse>> sendRequestAsync(TCPRequest &req)
    {
        return std::async(std::launch::async, [this, &req]()
                          { return this->sendRequest(req); });
    }

    // static functions allow for requests when we don't want to preserve the connection
    static std::optional<TCPResponse> sendRequest(const std::string &hostname, int port, TCPRequest &req)
    {
        TCPClient cli(hostname, port);
        return cli.sendRequest(req);
    }

    static std::future<std::optional<TCPResponse>> sendRequestAsync(const std::string &hostname, int port, TCPRequest &req)
    {
        return std::async(std::launch::async, [hostname, port, &req]()
                          { return TCPClient::sendRequest(hostname, port, req); });
    }

private:
    int sockfd;
    bool valid = true;
};

#endif // CORNUS_TCP_HPP
