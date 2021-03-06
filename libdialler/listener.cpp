#include <boost/asio.hpp>
#include <functional>
#include <libdialler/listener.h>
#include <libdialler/listener_client.h>
#include <string>

using namespace boost::asio;
using namespace boost::asio::ip;

using namespace dialler;

abstract_listener_consumer ::~abstract_listener_consumer() {
  _lstnr->erase_consumer();
}

void abstract_listener_consumer::set_listener(const std::shared_ptr<listener> &lstnr) {
  _lstnr = lstnr;
}

void abstract_listener_consumer::send_to(uint64_t id, message_ptr d) {
  if (!_lstnr->is_stopping_started()) {
    _lstnr->send_to(id, d);
  }
}

void abstract_listener_consumer::stop() {
  _lstnr->stop();
}

listener::listener(boost::asio::io_context *context, listener::params_t p)
    : _context(context)
    , _consumer()
    , _params(p) {
  _next_id.store(0);
}

listener::~listener() {
  stop();
}

void listener::start() {
  initialisation_begin();
  tcp::endpoint ep(tcp::v4(), _params.port);
  _aio = std::make_shared<async_io>(_context);
  _acc = std::make_shared<boost::asio::ip::tcp::acceptor>(*_context, ep);

  if (_consumer != nullptr) {
    _consumer->initialisation_begin();
  }
  start_async_accept();
}

void listener::start_async_accept() {

  _acc->async_accept(_aio->socket(), [this](auto ec) { this->on_accept_handler(ec); });
  if (is_stopping_started()) {
    _aio = nullptr;
    return;
  }
  initialisation_complete();
  if (_consumer != nullptr) {
    _consumer->initialisation_complete();
  }
}

void listener::on_accept_handler(const boost::system::error_code &err) {
  if (is_stopping_started()) {
    return;
  }
  if (err) {
    if (err == boost::asio::error::operation_aborted
        || err == boost::asio::error::connection_reset
        || err == boost::asio::error::eof) {
      return;
    } else {
      throw std::logic_error("listener: error on accept - " + err.message());
    }
  } else {
    assert(!is_stoped());

    std::shared_ptr<listener_client> new_client = nullptr;
    {
      std::lock_guard lg(_locker_connections);
      auto a = _aio;
      _aio = nullptr;
      new_client = std::make_shared<listener_client>(_next_id.fetch_add(1), a, this);
    }
    bool connectionAccepted = false;
    if (_consumer != nullptr) {
      connectionAccepted = _consumer->on_new_connection(new_client);
    }
    if (true == connectionAccepted) {
      std::lock_guard lg(_locker_connections);
      new_client->start();
      _connections.push_back(new_client);
    } else {
      _aio->full_stop();
      _aio = nullptr;
    }
  }

  boost::asio::ip::tcp::socket new_sock(*_context);
  _aio = std::make_shared<async_io>(_context);
  if (is_stopping_started()) {
    _aio = nullptr;
    return;
  }
  start_async_accept();
}

void listener::stop() {
  if (!stoping.is_started()) {
    stopping_started();

    if (_consumer != nullptr) {
      _consumer->stopping_started();
    }

    auto local_copy = [this]() {
      std::lock_guard lg(_locker_connections);
      return std::vector<std::shared_ptr<listener_client>>(_connections.begin(),
                                                           _connections.end());
    }();

    for (auto con : local_copy) {
      con->close();
    }

    if (_consumer != nullptr) {
      _consumer->stopping_completed();
    }
    if (_aio != nullptr) {
      _aio->socket().close();
      //_aio->fullStop();
      _aio = nullptr;
    }

    _acc->close();
    _acc = nullptr;
    _consumer = nullptr;
    _connections.clear();
    stopping_completed();
  }
}

void listener::erase_client_description(const listener_client_ptr client) {
  bool locked_localy = _locker_connections.try_lock();
  auto it = std::find_if(_connections.cbegin(), _connections.cend(), [client](auto c) {
    return c->get_id() == client->get_id();
  });
  if (it == _connections.cend()) {
    throw std::logic_error("delete error");
  }
  _connections.erase(it);
  if (locked_localy) {
    _locker_connections.unlock();
  }
  if (_consumer != nullptr) {
    _consumer->on_disconnect(client->shared_from_this());
  }
  client->stopping_completed();
}

void listener::send_to(listener_client_ptr i, message_ptr d) {
  i->send_data(d);
}

void listener::send_to(uint64_t id, message_ptr d) {
  std::lock_guard lg(this->_locker_connections);
  for (const auto &c : _connections) {
    if (c->get_id() == id) {
      send_to(c, d);
      return;
    }
  }
  throw std::logic_error("listener: unknow client #" + std::to_string(id));
}

void listener::add_consumer(const abstract_listener_consumer_ptr &c) {
  _consumer = c;
  c->set_listener(shared_from_this());
}

void listener::erase_consumer() {
  _consumer = nullptr;
}

void listener::on_network_error(listener_client_ptr i,
                                const boost::system::error_code &err) {
  if (_consumer != nullptr) {
    _consumer->on_network_error(i, err);
  }
}

void listener::on_new_message(listener_client_ptr i,
                              std::vector<message_ptr> &d,
                              bool &cancel) {
  if (_consumer != nullptr) {
    _consumer->on_new_message(i, d, cancel);
  }
}
