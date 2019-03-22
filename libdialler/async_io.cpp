#include <libdialler/async_io.h>

#include <cassert>
#include <exception>

using namespace boost::asio;
using namespace dialler;

async_io::async_io(boost::asio::io_context *service)
    : _sock(*service), next_message_size(0) {
  _messages_to_send = 0;
  _is_stoped = true;
  assert(service != nullptr);
  _service = service;
}

async_io::~async_io() noexcept {
  fullStop();
}

void async_io::start(data_handler_t onRecv, error_handler_t onErr) {
  if (!_is_stoped) {
    return;
  }
  _on_recv_hadler = onRecv;
  _on_error_handler = onErr;
  _is_stoped = false;
  _begin_stoping_flag = false;
  readNextAsync();
}

void async_io::fullStop(bool waitAllmessages) {
  _begin_stoping_flag = true;
  try {

    if (_sock.is_open()) {
      if (waitAllmessages && _messages_to_send.load() != 0) {
        auto self = this->shared_from_this();
		boost::asio::post(_service->get_executor(), [self]() { self->fullStop(); });
        //_service->post([self]() { self->fullStop(); });
      } else {
        boost::system::error_code ec;
        _sock.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        if (ec) {
          //#ifdef DOUBLE_CHECKS
          //          auto message = ec.message();
          //
          //          logger_fatal("AsyncIO::full_stop: _sock.shutdown() => code=",
          //          ec.value(),
          //                       " msg:", message);
          //#endif
        } else {
          _sock.close(ec);
          if (ec) {
            //#ifdef DOUBLE_CHECKS
            //            auto message = ec.message();
            //
            //            logger_fatal("AsyncIO::full_stop: _sock.close(ec)  => code=",
            //            ec.value(),
            //                         " msg:", message);
            //#endif
          }
        }
        _service = nullptr;
        _is_stoped = true;
      }
    }

  } catch (...) {
  }
}

void async_io::send(const message_ptr d) {
  if (_begin_stoping_flag) {
    return;
  }
  auto self = shared_from_this();

  auto ds = d->as_buffer();

  _messages_to_send.fetch_add(1);
  auto buf = boost::asio::buffer(ds.data, ds.size);

  auto on_write = [self, d](auto err, auto /*read_bytes*/) {
    if (err) {
      self->_on_error_handler(d, err);
    }
    self->_messages_to_send.fetch_sub(1);
  };

  async_write(_sock, buf, on_write);
}

void async_io::readNextAsync() {
  auto self = shared_from_this();

  auto on_read_message = [self](auto err, auto read_bytes, auto data_left,
                                message_ptr d) {
    if (err) {
      self->_on_error_handler(d, err);
    } else {
      assert(read_bytes == data_left);
      bool cancel_flag = false;
      try {
        self->_on_recv_hadler(std::move(d), cancel_flag);
      } catch (std::exception &ex) {
        throw std::logic_error(std::string("exception on async readNextAsync::on_read_message. - ") +
                               ex.what());
      }
      if (!cancel_flag) {
        self->readNextAsync();
      }
    }
  };

  auto on_read_size = [self, on_read_message](auto err, auto read_bytes) {
    if (err) {
      self->_on_error_handler(nullptr, err);
    } else {
      assert(read_bytes == message::SIZE_OF_SIZE);

      auto data_left = self->next_message_size - message::SIZE_OF_SIZE;
      message_ptr d = std::make_shared<message>(data_left);

      auto buf_ptr = (uint8_t *)(d->get_header());
      auto buf = boost::asio::buffer(buf_ptr, data_left);
      auto callback = [self, on_read_message, data_left, d](auto err, auto read_bytes) {
        on_read_message(err, read_bytes, data_left, d);
      };
      async_read(self->_sock, buf, callback);
    };
  };
  auto buf = boost::asio::buffer(static_cast<void *>(&self->next_message_size),
                                 message::SIZE_OF_SIZE);
  async_read(_sock, buf, on_read_size);
}
