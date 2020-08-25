import json
import sys

def main():
    samples = 1
    if len(sys.argv) > 1:
        samples = sys.argv[1]
        output_path = sys.argv[2]
        broker = "0.0.0.0" # hardcoding broker for now since we can't use it
    print ("Generating " + samples + " federates")

    with open(output_path+"/samples.csv", "w") as fed:
        for i in range(int(samples)):
            fed.write("EVMsgrunner.json\n")
        fed.close()
        

if __name__ == "__main__":
    main()
