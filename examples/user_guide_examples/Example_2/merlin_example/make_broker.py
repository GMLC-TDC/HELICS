import json
import sys
import socket

def main():
    if len(sys.argv) > 1:
        samples = sys.argv[1]
        output_path = sys.argv[2]

    ## getting the hostname by socket.gethostname() method
    hostname = socket.gethostname()
    ## getting the IP address using socket.gethostbyname() method
    ip_address = socket.gethostbyname(hostname)

    h_cli_sc = []

    broker = open(output_path+"/broker.json", "w")
    broker_json = json.dumps(
        { "federates": [{"directory": ".", 
                         "exec": "helics_app broker --external --federates " + str(int(samples) *2),
                         "host": "localhost",
                         "name": "broker_" + ip_address + "_of_"+str(int(samples)*2)}],
          "name" : "broker"},
        indent=4, sort_keys=True)
    broker.write(broker_json)
    broker.close()
    h_cli_sc.append(output_path+"/broker.json")

    with open(output_path+"/broker.ip", "w") as f:
        f.write(ip_address)

    with open("broker.csv", "w") as f:
        f.write("\n".join(h_cli_sc))

if __name__ == "__main__":
    main()
