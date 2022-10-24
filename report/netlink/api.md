# Netlink Module

最长的 payload 为 1024



## 用户态

```c
int u2k_socket_init();
// 初始化 socket 及地址，需要在内核模块启动后进行
// If this function succeeds, return PRM_SUCCESS, else return PRM_ERROR

int u2k_send(char *buf, size_t len);
// 向内核发送 buf 中的 len 长度字节
// 成功返回 PRM_SUCCESS, 失败返回 PRM_ERROR

ssize_t u2k_recv(char *buf, size_t buflen);
// 阻塞接收信息，存在 buf 中，buf 长度为 buflen
// 返回实际接收的数据长度, 失败返回 PRM_ERROR

int u2k_socket_release();
// 释放 socket 及内存空间
// 返回 PRM_SUCCESS
```



## 核心态





