import cv2
import socket

def send_command(cmd, server_ip, server_port):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.connect((server_ip, server_port))
            sock.sendall(cmd.encode())
    except Exception as e:
        print(f"Error sending command: {e}")

def main():
    rtsp_url = "STREAMURL"
    command_server_ip = "SERVER_IP"
    command_server_port = <SERVER_MESSAGE_PORT>
    
    cap = cv2.VideoCapture(rtsp_url)
    if not cap.isOpened():
        print("Failed to open RTSP stream")
        return
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("Failed to retrieve frame")
            break
        
        cv2.imshow("Stream", frame)
        key = cv2.waitKey(1) & 0xFF
        
        if key == ord('x'):
            break
        elif key in [ord(c) for c in "nrmedbca"]:
            send_command(chr(key), command_server_ip, command_server_port)
    
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
