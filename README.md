# dialler

boost::asio wrapper

## build
1. add conan repo
```sh
$ conan remote add comunity https://api.bintray.com/conan/conan-community/conan 
```
2. build
```sh
$ mkdir build
$ cd build
$ conan install ..
$ cmake ..
```
# Example

```C++

struct Listener : public dialler::abstract_listener_consumer {
  bool on_new_connection(dialler::listener_client_ptr) override {
    connections.fetch_add(1);
    return true;
  }

  void on_network_error(dialler::listener_client_ptr i,
                        const dialler::message_ptr & /*d*/,
                        const boost::system::error_code & /*err*/) override {}

  void on_new_message(dialler::listener_client_ptr i, dialler::message_ptr && /*d*/,
                      bool & /*cancel*/) override {}

  void on_disconnect(const dialler::listener_client_ptr & /*i*/) override {
    connections.fetch_sub(1);
  }

  std::atomic_int16_t connections = 0;
};

struct Connection : public dialler::abstract_dial {
  void on_connect() override { mock_is_connected = true; };
  void on_new_message(dialler::message_ptr &&, bool &) override {}
  void on_network_error(const dialler::message_ptr &,
                        const boost::system::error_code &err) override {
    bool isError = err == boost::asio::error::operation_aborted ||
                   err == boost::asio::error::connection_reset ||
                   err == boost::asio::error::eof;
    if (isError && !is_stoped()) {
      auto msg = err.message();
      EXPECT_FALSE(true);
    }
  }

  bool mock_is_connected = false;
  bool connection_error = false;
};

std::shared_ptr<dialler::listener> server = nullptr;
std::shared_ptr<Listener> lstnr = nullptr;


void start_server(){
  dialler::listener::params_t p;
  p.port = 4040;
  server = std::make_shared<dialler::listener>(service, p);
  lstnr = std::make_shared<Listener>();
  server->add_consumer(lstnr.get());
  server->start();
}

void connect(){
    dialler::dial::params_t p("localhost", 4040);
    auto client= std::make_shared<dialler::dial>(service, p);
    auto con = std::make_shared<Connection>();
    client->add_consumer(con.get());
    client->start_async_connection();
}

```
