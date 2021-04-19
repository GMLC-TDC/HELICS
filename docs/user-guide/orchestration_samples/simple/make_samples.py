import json
import sys


def main():
    samples = 1
    if len(sys.argv) > 1:
        samples = sys.argv[1]
        output_path = sys.argv[2]
    print("Generating " + samples + " samples")

    h_cli_sc = []

    broker = open(output_path + "/broker.json", "w")
    broker_json = json.dumps(
        {
            "federates": [
                {
                    "directory": ".",
                    "exec": "helics_app broker --federates " + str(int(samples) * 2),
                    "host": "localhost",
                    "name": "broker_of_" + str(int(samples) * 2),
                }
            ],
            "name": "broker",
        },
        indent=4,
        sort_keys=True,
    )
    broker.write(broker_json)
    broker.close()
    h_cli_sc.append(output_path + "/broker.json")

    for i in range(int(samples)):
        send_file_name = output_path + "/pysender" + str(i) + ".json"
        recv_file_name = output_path + "/pyrecv" + str(i) + ".json"
        sender = open(send_file_name, "w")
        recv = open(recv_file_name, "w")
        h_cli_sc.append(send_file_name)
        h_cli_sc.append(recv_file_name)

        send_name = "pisender" + str(i)
        s_json = json.dumps(
            {
                "federates": [
                    {
                        "directory": ".",
                        "exec": "python3 -u pisender.py " + str(i),
                        "host": "localhost",
                        "name": send_name,
                    }
                ],
                "name": send_name,
            },
            indent=4,
            sort_keys=True,
        )

        recv_name = "pireceiver" + str(i)
        r_json = json.dumps(
            {
                "federates": [
                    {
                        "directory": ".",
                        "exec": "python3 -u pireceiver.py " + str(i),
                        "host": "localhost",
                        "name": recv_name,
                    }
                ],
                "name": recv_name,
            },
            indent=4,
            sort_keys=True,
        )

        sender.write(s_json)
        recv.write(r_json)
        sender.close()
        recv.close()

    with open("samples.csv", "w") as f:
        f.write("\n".join(h_cli_sc))


if __name__ == "__main__":
    main()
