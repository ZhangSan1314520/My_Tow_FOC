# ulog 使用说明

## ulog 日志记录步骤

1. 写文件头 `write_file_header()`
2. 写标志位 `write_message_flag_bits()`
3. 写数据格式信息 `write_message_format()`
4. 写数据订阅信息 `write_message_subscription()`
5. 写数据 `write_xxx`