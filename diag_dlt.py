#!/usr/bin/env python3
"""
DLT Viewer TCP Server - 深度诊断脚本
用法: python diag_dlt.py

配合 Viewer 调试日志使用:
1. 重新编译 Viewer (已添加 qDebug 日志)
2. 从命令行启动 Viewer (或在 Qt Creator Debug 模式)
3. 在 Viewer 中: File → Clear (扫帚图标)
4. 配置 ECU 为 TCP Server, 端口 3490
5. **取消勾选 Auto Reconnect** (重要!)
6. 点击 Connect
7. 运行本脚本
8. 对比脚本输出和 Viewer 控制台日志
"""

import socket
import struct
import time
import sys

HOST = "127.0.0.1"
PORT = 3490

# DLT constants
DLT_HTYP_UEH  = 0x01
DLT_HTYP_MSBF = 0x02
DLT_HTYP_WEID = 0x04
DLT_HTYP_WSID = 0x08
DLT_HTYP_WTMS = 0x10
DLT_HTYP_VERS = 0x20

DLT_TYPE_LOG  = 1
DLT_LOG_INFO  = 6

DLT_TYPE_INFO_STRG = 0x0200
DLT_TYPE_INFO_BOOL = 0x0400
DLT_TYPE_INFO_SINT = 0x0800
DLT_TYPE_INFO_UINT = 0x1000

SERIAL_HEADER = b"DLS\x01"

_msg_counter = 0


def build_dlt_log_msg(ecuid, apid, ctid, text):
    """构建一条 DLT 非详细日志消息"""
    global _msg_counter
    _msg_counter += 1

    payload = struct.pack(">I", DLT_TYPE_INFO_STRG)
    text_bytes = text.encode("utf-8")
    payload += struct.pack(">H", len(text_bytes) + 1)
    payload += text_bytes + b"\x00"

    noar = 1
    msin = (DLT_TYPE_LOG << 1) | DLT_LOG_INFO

    ecu = ecuid.encode("ascii").ljust(4, b"\x00")[:4]
    apid_b = apid.encode("ascii").ljust(4, b"\x00")[:4]
    ctid_b = ctid.encode("ascii").ljust(4, b"\x00")[:4]
    sid = struct.pack(">I", _msg_counter)
    tmsp = struct.pack(">I", _msg_counter * 1000)

    htyp = DLT_HTYP_VERS | DLT_HTYP_UEH | DLT_HTYP_MSBF | DLT_HTYP_WEID | DLT_HTYP_WSID | DLT_HTYP_WTMS
    mcnt = _msg_counter & 0xFF
    # len = standard_header(4) + extra(12) + extended_header(10) + payload
    total_len = 4 + 12 + 10 + len(payload)

    msg = SERIAL_HEADER
    msg += struct.pack("B", htyp)
    msg += struct.pack("B", mcnt)
    msg += struct.pack(">H", total_len)
    msg += ecu
    msg += sid
    msg += tmsp
    msg += struct.pack("B", msin)
    msg += struct.pack("B", noar)
    msg += apid_b
    msg += ctid_b
    msg += payload

    return msg


def hex_dump(data, prefix=""):
    lines = []
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_part = " ".join(f"{b:02x}" for b in chunk)
        ascii_part = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append(f"{prefix}  {i:04x}: {hex_part:<48s} {ascii_part}")
    return "\n".join(lines)


def main():
    print("=" * 60)
    print("DLT Viewer TCP Server 深度诊断")
    print("=" * 60)
    print()

    # Build test message
    msg = build_dlt_log_msg("ECU1", "TEST", "INFO", "Hello DLT Viewer!")
    print(f"DLT 消息 ({len(msg)} bytes):")
    print(hex_dump(msg))
    print()

    # Connect
    print(f"连接 {HOST}:{PORT} ...")
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(10)
        sock.connect((HOST, PORT))
        print("[OK] 连接成功!")
    except Exception as e:
        print(f"[FAIL] 连接失败: {e}")
        sys.exit(1)

    # Send multiple messages with 500ms interval
    num_msgs = 20
    interval = 0.5
    sent = 0
    errors = 0

    print(f"\n发送 {num_msgs} 条消息, 间隔 {interval}s ...")
    print("-" * 40)

    for i in range(num_msgs):
        t = time.time()
        try:
            sock.send(msg)
            sent += 1
            elapsed = time.time() - t
            print(f"  [{i+1:2d}/{num_msgs}] 发送成功 ({len(msg)} bytes, {elapsed*1000:.1f}ms)")
        except Exception as e:
            errors += 1
            print(f"  [{i+1:2d}/{num_msgs}] 发送失败: {e}")
            # Try reconnect
            try:
                sock.close()
            except:
                pass
            time.sleep(1)
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(10)
                sock.connect((HOST, PORT))
                print(f"         -> 重连成功!")
                sock.send(msg)
                sent += 1
                errors -= 1  # undo error count since resend succeeded
            except Exception as e2:
                print(f"         -> 重连失败: {e2}")
                break

        time.sleep(interval)

    print("-" * 40)
    print(f"发送完成: 成功={sent}, 失败={errors}")

    # Check final connection state
    try:
        sock.settimeout(1)
        data = sock.recv(4096)
        if data:
            print(f"\n收到 Viewer 数据 ({len(data)} bytes):")
            print(hex_dump(data, "  "))
        else:
            print("\n连接已关闭 (recv 返回空)")
    except socket.timeout:
        print("\n连接仍然存活 (recv 超时=无数据)")
    except Exception as e:
        print(f"\n连接异常: {e}")

    try:
        sock.close()
    except:
        pass

    print()
    print("=" * 60)
    print("下一步:")
    print("1. 查看 DLT Viewer 控制台输出 (Qt Creator 'Application Output')")
    print("   关键日志:")
    print("   - 'readyRead() called' → socket 信号正常触发")
    print("   - 'FAIL locking indexer' → indexer 锁冲突!")
    print("   - 'condition FAIL' → socket/connected 匹配失败!")
    print("   - 'read() TCP/TCP_SERVER' → 数据读取正常")
    print("   - 'autoReconnect timeout' → 超时断开 (取消勾选 Auto Reconnect)")
    print()
    print("2. 检查 Viewer 状态栏:")
    print("   - 'Recv:' 值是否 > 0 ?")
    print("   - 表格中是否有消息 ?")
    print("=" * 60)


if __name__ == "__main__":
    main()
