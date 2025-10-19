import socket
import serial
import threading
import time
import select

class TCPComBridge:
    def __init__(self, com_port, baudrate, tcp_port, host='0.0.0.0'):
        self.com_port = com_port
        self.baudrate = baudrate
        self.tcp_port = tcp_port
        self.host = host
        self.running = False
        self.tcp_socket = None
        self.serial_conn = None

    def start(self):
        """Starts TCP-COM bridge"""
        try:
            # Initialize serial connection
            self.serial_conn = serial.Serial(
                port=self.com_port,
                baudrate=self.baudrate,
                timeout=1
            )
            print(f"Connected to COM port: {self.com_port}")

            # Initialize TCP socket
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.tcp_socket.bind((self.host, self.tcp_port))
            self.tcp_socket.listen(1)
            print(f"TCP server listening on {self.host}:{self.tcp_port}")

            self.running = True

            # Main loop accepting connections
            while self.running:
                print("Waiting for TCP connection...")
                client_socket, client_address = self.tcp_socket.accept()
                print(f"Connected to {client_address}")

                # Start connection handler
                self.handle_connection(client_socket)

        except Exception as e:
            print(f"Error: {e}")
        finally:
            self.stop()

    def handle_connection(self, client_socket):
        """Handles single TCP connection"""
        try:
            # Set socket timeout to check running flag
            client_socket.settimeout(1.0)

            while self.running:
                # Check data from TCP
                try:
                    data = client_socket.recv(1024)
                    if data:
                        # Send data from TCP to COM
                        self.serial_conn.write(data)
                        print(f"TCP → COM: {len(data)} bytes")
                except socket.timeout:
                    pass
                except socket.error:
                    break

                # Check data from COM
                if self.serial_conn.in_waiting > 0:
                    com_data = self.serial_conn.read(self.serial_conn.in_waiting)
                    if com_data:
                        # Send data from COM to TCP
                        client_socket.send(com_data)
                        print(f"COM → TCP: {len(com_data)} bytes")

                time.sleep(0.01)  # Small delay to not overload CPU

        except Exception as e:
            print(f"Connection error: {e}")
        finally:
            client_socket.close()
            print("TCP connection closed")

    def stop(self):
        """Stops the bridge"""
        self.running = False
        if self.tcp_socket:
            self.tcp_socket.close()
        if self.serial_conn:
            self.serial_conn.close()
    print("Bridge stopped")

# Przykład użycia
if __name__ == "__main__":
    bridge = TCPComBridge(
        com_port='COM5',      # Change to correct COM port
        baudrate=115200,        # Match your device
        tcp_port=8888,        # TCP port to listen
        host='0.0.0.0'        # Listen on all interfaces
    )

    try:
        bridge.start()
    except KeyboardInterrupt:
        print("\nStopping...")
        bridge.stop()