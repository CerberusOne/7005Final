tcpdump -i eno1 -x 'tcp & src port '"$3"' and ip dst '"$2"'and ip src '"$1"'' -w server.pcap
