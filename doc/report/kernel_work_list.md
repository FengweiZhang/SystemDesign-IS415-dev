# 内核模块相关功能清单

## 完成功能

`prm_hook`

- [x] 找到系统调用表位置

- [x] 对系统调用进行hook

`prm_netlink`

- [x] 基础数据通信

- [x] 包含信息类型的数据通信

- [x] 进程间数据通信（使用信号量与全局变量）


`user_netlink`

- [x] 与内核态建立连接

- [x] 与内核态基础通信，包含信息类型的通信

- [x] 重新建立/断开与内核态的连接


## 下一步

`prm_hook`

- [ ] 系统调用函数参数处理，进程用户获取

- [ ] 写hook功能函数，调用`prm_netlink`进行权限查找


`prm_netlink`

- [ ] 权限查找函数参数补全

- [ ] 与用户态通信，携带权限查找函数参数

- [ ] 处理用户态可能发来的过期的权限查询结果，当前使用参数对比方式，可以使用list记录替换（再说）


`user_netlink`

- [ ] 权限查找处理函数，与内核态对接

- [ ] 权限查找处理函数，与lzz对接

- [ ] 监听函数，处理内核态消息并调用权限查找处理函数


- [ ] 初始化链接发送消息


