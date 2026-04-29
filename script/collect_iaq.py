"""
collect_iaq.py

Thu thập dữ liệu IAQ từ ZMOD4410 qua UART và ghi vào file CSV để train AI.

Format nhận từ UART (IAQ_PACKET_ASCII mode):
    "1.0,0.031,400.0,0.016\r\n"

Format CSV output:
    timestamp,iaq,tvoc,eco2,etoh,label

Cách dùng:
    pip install pyserial
    python collect_iaq.py --port COM3 --label indoor --output data.csv
"""

import serial
import csv
import argparse
import os
from datetime import datetime

def parse_args():
    parser = argparse.ArgumentParser(description="Thu thập dữ liệu IAQ từ ZMOD4410")
    parser.add_argument("--port",   required=True,          help="COM port (vd: COM3)")
    parser.add_argument("--baud",   type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--label",  default="indoor",       help="Label cho data (default: indoor)")
    parser.add_argument("--output", default="iaq_data.csv", help="Tên file CSV output")
    return parser.parse_args()

def main():
    args = parse_args()

    file_exists = os.path.isfile(args.output)

    print(f"Port  : {args.port} @ {args.baud} baud")
    print(f"Label : {args.label}")
    print(f"Output: {args.output}")
    print("Đang thu thập... Nhấn Ctrl+C để dừng.\n")

    sample = 0

    with serial.Serial(args.port, args.baud, timeout=2) as ser, \
         open(args.output, "a", newline="") as csvfile:

        writer = csv.writer(csvfile)

        # Ghi header nếu file mới
        if not file_exists:
            writer.writerow(["timestamp", "sample", "iaq", "tvoc", "eco2", "etoh", "label"])
            csvfile.flush()

        while True:
            try:
                line = ser.readline().decode("ascii", errors="ignore").strip()
            except serial.SerialException as e:
                print(f"[ERROR] {e}")
                break

            if not line:
                continue

            # Parse dòng "[ZMOD] IAQ=1.0 TVOC=0.015 eCO2=400.0 EtOH=0.008"
            if not line.startswith("[ZMOD] IAQ="):
                continue

            try:
                # Tách các field: IAQ=x.x TVOC=x.xxx eCO2=x.x EtOH=x.xxx
                fields = {}
                for token in line[len("[ZMOD] "):].split():
                    key, val = token.split("=")
                    fields[key] = float(val)
                iaq  = fields["IAQ"]
                tvoc = fields["TVOC"]
                eco2 = fields["eCO2"]
                etoh = fields["EtOH"]
            except (ValueError, KeyError):
                continue

            sample += 1
            ts = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            writer.writerow([ts, sample, iaq, tvoc, eco2, etoh, args.label])
            csvfile.flush()

            print(f"[{sample:5d}] {ts}  IAQ={iaq:.1f}  TVOC={tvoc:.3f}  eCO2={eco2:.1f}  EtOH={etoh:.3f}")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nDừng thu thập.")
