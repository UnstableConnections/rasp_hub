from hub_core import HubCore
import socketserver
import sys
import time

core = HubCore

# See:  https://docs.python.org/3/library/socketserver.html#module-socketserver

class SateliteHandler(socketserver.BaseRequestHandler):

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1024).strip()
        
        print("{} wrote:".format(self.client_address[0]))
        print(self.data)

        # send message to processing
        answers = core.handle_message(self.data, self.client_address[0])
        
        for answ in answers:
            # answer hub
            self.request.sendall(answ)
            time.sleep(1) # pause 1 sec


if __name__ == "__main__":
    HOST, PORT = "localhost", 10000

    # Create the server, binding to localhost 
    with socketserver.TCPServer((HOST, PORT), SateliteHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()