# 如何在linux下实现一个web_server？
    用 C 实现一个Web 服务器,本质上是用Socket 编程.
# socket 编程demo
    -> demo1：一个简单的回显案例
        需要改善的地方：
            1. 不能处理多次输入
            2. 等待连接时阻塞
            3. 错误输出没使用日志
