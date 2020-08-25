import json
import sys

def main():
    samples = 1
    if len(sys.argv) > 1:
        samples = sys.argv[1]
        output_path = sys.argv[2]
        # broker = sys.argv[3]
        broker = "0.0.0.0" # hardcoding broker for now since we can't use it
    print ("Generating " + samples + " federates")
    samples = int(samples)/2
    h_cli_sc = []

    # broker = open(output_path+"/broker.json", "w")
    # broker_json = json.dumps(
    #     { "federates": [{"directory": ".", 
    #                      "exec": "helics_app broker --federates " + str(int(samples) *2),
    #                      "host": "localhost",
    #                      "name": "broker_of_"+str(int(samples)*2)}],
    #       "name" : "broker"},
    #     indent=4, sort_keys=True)
    # broker.write(broker_json)
    # broker.close()
    # h_cli_sc.append(output_path+"/broker.json")

    with open("federates.csv", "w") as fed:
        for i in range(int(samples)):
            send_file_name = output_path+"/pisender"+str(i)+".json"
            recv_file_name = output_path+"/pirecv"+str(i)+".json"
            sender = open(send_file_name, "w")
            recv = open(recv_file_name, "w")
            h_cli_sc.append(send_file_name)
            h_cli_sc.append(recv_file_name)
            
            send_name = "pisender"+str(i)
            fed.write("pisender.py " + str(i) + " \n")
            s_json = json.dumps(
                { "federates": [{"directory": ".", 
                                 "exec": "python3 -u pisender.py " + str(i) + " " + broker,
                                 "host": broker, # The host arg doesn't do anything FYI
                                 "name": send_name}],
                  "name" : send_name},
                indent=4, sort_keys=True)

            recv_name = "pireceiver"+str(i)
            fed.write("pireceiver.py " + str(i) + " \n")
            r_json = json.dumps(
                { "federates": [{"directory": ".", 
                                 "exec": "python3 -u pireceiver.py " + str(i)  + " " + broker,
                                 "host": broker,
                                 "name": recv_name}],
                  "name" : recv_name},
                indent=4, sort_keys=True)
            
            sender.write(s_json)
            recv.write(r_json)
            sender.close()
            recv.close()
        

    with open("samples.csv", "w") as f:
        f.write("\n".join(h_cli_sc))

if __name__ == "__main__":
    main()
