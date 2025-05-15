#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define BACKLOG 5

int main() {
  int server_fd, client_fd;
  struct sockaddr_in server_addr, client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  char buffer[BUFFER_SIZE] = {0};

  // 创建tcp连接
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // 设置
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 设置套接字端口 地址 协议
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // 绑定套接字
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  // 监听套接字
  if (listen(server_fd, BACKLOG) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Server listening on port %d...\n", PORT);

  // 接受客户端连接
  if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                          &client_addr_len)) < 0) {
    perror("accept failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
         ntohs(client_addr.sin_port));

  // 接收数据
  ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_read < 0) {
    perror("recv failed");
  } else {
    buffer[bytes_read] = '\0';
    printf("Received: %s\n", buffer);

    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "Server received: %s", buffer);
    send(client_fd, response, strlen(response), 0);
  }

  // 关闭连接
  close(client_fd);
  close(server_fd);

  return 0;
}
