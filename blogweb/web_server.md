# 如何在linux下实现一个web_server？
    用 C 实现一个Web 服务器,本质上是用Socket 编程.
# socket 编程demo
    '''
    #include <stdio.h>              // 标准输入输出函数，如 printf、perror
    #include <stdlib.h>             // 标准库函数，如 exit
    #include <string.h>             // 字符串处理，如 memset、strcpy
    #include <unistd.h>             // UNIX 标准函数，如 close、read、write
    #include <sys/socket.h>         // 套接字接口，提供 socket/bind/listen/accept 等函数
    #include <netinet/in.h>         // sockaddr_in 结构体定义及相关宏（IPV4）
    #include <arpa/inet.h>          // inet_ntoa 和 htons 等函数声明

    #define PORT 8080               // 服务器监听端口
    #define BUFFER_SIZE 1024        // 缓冲区大小
    #define BACKLOG 5               // 监听队列最大连接数（客户端同时连接的等待上限）

    int main() {
        int server_fd, client_fd;                      // 文件描述符：服务器端 socket、客户端 socket
        struct sockaddr_in server_addr, client_addr;   // 用于存储地址信息的结构体（IP + 端口）
        socklen_t client_addr_len = sizeof(client_addr);
        char buffer[BUFFER_SIZE] = {0};                // 数据接收缓冲区（初始化为全 0）

        // 1. 创建 socket，参数分别是：
        // AF_INET：IPv4
        // SOCK_STREAM：TCP（面向连接）
        // 0：协议（默认即可）
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket creation failed");  // 输出错误信息
            exit(EXIT_FAILURE);
            }

        // 设置 socket 选项 SO_REUSEADDR，允许端口重用
        // 否则服务重启时可能会提示“Address already in use”
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // 2. 配置服务器地址结构体
        memset(&server_addr, 0, sizeof(server_addr));       // 清空结构体
        server_addr.sin_family = AF_INET;                   // 使用 IPv4
        server_addr.sin_addr.s_addr = INADDR_ANY;           // 监听所有可用 IP（0.0.0.0）
        server_addr.sin_port = htons(PORT);                 // 将主机字节序端口号转换为网络字节序

        // 3. 将 socket 绑定到服务器地址
        if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("bind failed");                          // 比如端口已占用
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // 4. 开始监听，进入被动等待连接状态
        if (listen(server_fd, BACKLOG) < 0) {
            perror("listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        printf("Server listening on port %d...\n", PORT);

        // 5. 接受客户端连接（阻塞直到有连接）
        // 返回新的 socket 文件描述符（client_fd）用于与客户端通信
        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // 输出客户端的 IP 地址和端口号
        printf("Accepted connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),             // 将 IP 地址转换为点分十进制字符串
               ntohs(client_addr.sin_port));                // 网络字节序转换为主机字节序

        // 6. 接收客户端发送的数据
        // recv 函数与 read 类似，但用于 socket
        ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0) {
            perror("recv failed");
        } else {
            buffer[bytes_read] = '\0';  // 确保接收的字符串以 NULL 结尾
            printf("Received: %s\n", buffer);

            // 7. 构造响应并发送给客户端
            char response[BUFFER_SIZE];
            snprintf(response, sizeof(response), "Server received: %s", buffer);
            send(client_fd, response, strlen(response), 0);  // 发送响应
        }

        // 8. 关闭客户端 socket 和服务器 socket
        close(client_fd);
        close(server_fd);

        return 0;
    }
    '''

