# Modbus 协议详细测试报告

## 总览

- 总用例数: 38
- 通过: 37
- 失败: 0
- 错误: 0
- 跳过: 1

## 用例明细

| 用例 | 预期结果类型 | 覆盖内容 | 状态 | 耗时(s) | 错误/断言输出 |
|---|---|---|---|---:|---|
| test_tcp_protocol_header | 正常响应 | MBAP 头字段(transaction/protocol/length/unit-id)校验；FC03 响应结构合法性 | PASS | 0.002 | - |
| test_tcp_read_and_write_coils | 正常响应 | FC01 读线圈；FC05 写单线圈；写后读一致性验证 | PASS | 0.002 | - |
| test_tcp_read_discrete_inputs | 正常响应 | FC02 读离散输入；位打包/解包一致性 | PASS | 0.001 | - |
| test_tcp_read_and_write_registers | 正常响应 | FC03 读保持寄存器；FC06 写单寄存器；寄存器写后读回一致性 | PASS | 0.001 | - |
| test_tcp_read_input_registers | 正常响应 | FC04 读输入寄存器；寄存器字节序解析校验 | PASS | 0.001 | - |
| test_tcp_batch_operations | 正常响应 | FC0F 批量写线圈；FC10 批量写保持寄存器；批量写后读回一致性 | PASS | 0.002 | - |
| test_tcp_exception_cases | 异常响应（预期异常） | 非法功能码异常(0x01)；非法地址异常(0x02)；非法数据值异常(0x03) | PASS | 0.002 | - |
| test_tcp_transaction_id_echo_sequence | 正常响应 | MBAP Transaction ID 回显一致性；多请求顺序一致性 | PASS | 0.001 | - |
| test_tcp_unsupported_standard_function_codes[7] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.000 | - |
| test_tcp_unsupported_standard_function_codes[8] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.000 | - |
| test_tcp_unsupported_standard_function_codes[11] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.000 | - |
| test_tcp_unsupported_standard_function_codes[12] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.000 | - |
| test_tcp_unsupported_standard_function_codes[17] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.001 | - |
| test_tcp_unsupported_standard_function_codes[22] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.001 | - |
| test_tcp_unsupported_standard_function_codes[23] | 异常响应（预期异常） | 未实现标准功能码返回非法功能异常(0x01) | PASS | 0.000 | - |
| test_tcp_read_quantity_limits[1-2000-0-2001] | 边界混合（正常+异常） | FC01/02/03/04 读数量上下限校验；非法数量返回异常(0x03) | PASS | 0.003 | - |
| test_tcp_read_quantity_limits[2-2000-0-2001] | 边界混合（正常+异常） | FC01/02/03/04 读数量上下限校验；非法数量返回异常(0x03) | PASS | 0.002 | - |
| test_tcp_read_quantity_limits[3-125-0-126] | 边界混合（正常+异常） | FC01/02/03/04 读数量上下限校验；非法数量返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_read_quantity_limits[4-125-0-126] | 边界混合（正常+异常） | FC01/02/03/04 读数量上下限校验；非法数量返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_read_address_boundaries[1] | 边界混合（正常+异常） | FC01/02/03/04 地址边界校验；越界地址返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_read_address_boundaries[2] | 边界混合（正常+异常） | FC01/02/03/04 地址边界校验；越界地址返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_read_address_boundaries[3] | 边界混合（正常+异常） | FC01/02/03/04 地址边界校验；越界地址返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_read_address_boundaries[4] | 边界混合（正常+异常） | FC01/02/03/04 地址边界校验；越界地址返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_write_single_address_boundaries[5-'\x0f\xff\x00-'\x10\xff\x00] | 边界混合（正常+异常） | FC05/06 写单点地址边界校验；写地址越界返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_write_single_address_boundaries[6-'\x0f\x13W-'\x10\x13W] | 边界混合（正常+异常） | FC05/06 写单点地址边界校验；写地址越界返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_write_multi_address_boundaries[15-9998-9999-2-\x03] | 边界混合（正常+异常） | FC0F/10 批量写地址边界校验；批量写越界返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_write_multi_address_boundaries[16-9998-9999-2-\xaaUU\xaa] | 边界混合（正常+异常） | FC0F/10 批量写地址边界校验；批量写越界返回异常(0x02) | PASS | 0.001 | - |
| test_tcp_write_multi_limits_and_byte_count[15-1969-1968-\x00-2] | 边界混合（正常+异常） | FC0F/10 数量限制与字节计数一致性；非法数量/字节数返回异常(0x03) | PASS | 0.141 | - |
| test_tcp_write_multi_limits_and_byte_count[16-124-123-\x124-1] | 边界混合（正常+异常） | FC0F/10 数量限制与字节计数一致性；非法数量/字节数返回异常(0x03) | PASS | 0.003 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[1-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[2-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[3-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[4-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[5-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[6-\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[15-\x00\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_tcp_short_request_pdu_returns_illegal_data_value[16-\x00\x00\x00\x00] | 异常响应（预期异常） | 请求 PDU 长度不足处理；长度错误返回异常(0x03) | PASS | 0.001 | - |
| test_rtu_single_register_roundtrip | 正常响应 | RTU FC06/FC03 往返验证；RTU CRC16 校验；RTU 从站地址一致性 | SKIP | 0.000 | RTU port not provided |
