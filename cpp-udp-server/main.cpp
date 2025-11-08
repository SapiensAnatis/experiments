#include <coro/coro.hpp>
#include <cstdint>
#include <expected>
#include <iostream>
#include <memory>
#include <print>
#include <sys/socket.h>

enum class Error { FailedToCreateSocket, FailedToCreatEpoll };

struct __attribute__((packed)) MessageHeader {
  uint32_t document_id;
  uint32_t chunk_id;
};

static_assert(sizeof(MessageHeader) == 4 * 2);

std::expected<int, Error> create_socket() {
  const int socket_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
  if (socket_fd < 0) {
    perror("socket");
    return std::unexpected(Error::FailedToCreateSocket);
  }

  sockaddr_in serverAddress{};
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(3000);

  if (bind(socket_fd, reinterpret_cast<sockaddr *>(&serverAddress),
           sizeof(serverAddress)) < 0) {
    perror("bind");
    return std::unexpected(Error::FailedToCreateSocket);
  }

  return socket_fd;
}

std::expected<int, Error> create_epoll(int socket_fd) {
  int epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    perror("epoll_create1");
    return std::unexpected(Error::FailedToCreatEpoll);
  }

  epoll_event ev{};
  ev.events = EPOLLIN;
  ev.data.fd = socket_fd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev) < 0) {
    perror("epoll_ctl");
    return std::unexpected(Error::FailedToCreatEpoll);
  }

  return epoll_fd;
}

using XmlChannel = coro::queue<std::string>;

coro::task<> receive_xml_document(coro::queue<std::string> &chunk_buffer) {
  std::string finalBuffer;

  // TODO: use chunk_id here to be able to handle receiving packets out of order

  do {
    auto chunk_result = co_await chunk_buffer.pop();

    if (!chunk_result.has_value()) {
      std::println(std::cerr, "Failed to read from ring buffer");
      co_return;
    }

    finalBuffer += chunk_result.value();
  } while (finalBuffer.find("</catalog>") == std::string::npos);

  co_await chunk_buffer.shutdown();

  std::println("Read full XML document: {}", finalBuffer);
}

int main() {
  constexpr int MAX_EVENTS = 10;

  auto pool = coro::thread_pool::make_shared(coro::thread_pool::options{
      .thread_count = 4,
  });

  const int socket_fd = create_socket().value();
  const int epoll_fd = create_epoll(socket_fd).value();

  std::println("Server started on 0.0.0.0:3000/udp");

  std::array<epoll_event, MAX_EVENTS> events{};
  std::array<uint8_t, 8> header_buffer{};
  std::array<char, 256> data_buffer{};

  std::unordered_map<uint32_t, XmlChannel> buffers_by_document_id{};

  while (true) {
    // TODO: epoll edge triggering?
    const int num_fds = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
    if (num_fds == -1) {
      perror("epoll_wait");
      exit(EXIT_FAILURE);
    }

    assert(num_fds <= MAX_EVENTS);
    for (int i = 0; i < num_fds; i++) {
      const ssize_t header_byte_count =
          recv(socket_fd, header_buffer.data(), header_buffer.size(), MSG_PEEK);
      if (header_byte_count < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
      }

      assert(header_byte_count >= 8);
      const auto header = std::bit_cast<MessageHeader>(header_buffer);

      // TODO: could introduce a checksum on the header (doesn't UDP handle this
      // already)?

      std::println("Received header. Document ID: {}, chunk ID: {}",
                   header.document_id, header.chunk_id);

      auto [map_it, inserted] =
          buffers_by_document_id.try_emplace(header.document_id);
      auto &channel = map_it->second;

      if (inserted) {
        pool->spawn(receive_xml_document(channel));
      }

      const ssize_t data_byte_count =
          recv(socket_fd, data_buffer.data(), data_buffer.size(), 0);
      if (data_byte_count < 0) {
        perror("recv");
        exit(EXIT_FAILURE);
      }

      auto data_span = std::span(data_buffer.data() + sizeof(MessageHeader),
                                 data_buffer.data() + data_byte_count);
      auto end_it = std::ranges::find(data_span, '\0');
      if (end_it == data_span.end()) {
        std::println(std::cerr, "Malformed packet: no divider found");
        exit(EXIT_FAILURE);
      }

      coro::sync_wait(channel.emplace(
          data_span.begin(), end_it)); // TODO: could we avoid sync_wait here?
    }

    // TODO: delete finished queues from the unordered_map (how to check if it's
    // closed?)
  }

  return EXIT_SUCCESS;
}
