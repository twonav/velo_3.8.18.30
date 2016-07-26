cmd_net/netfilter/nfnetlink_queue.o := arm-linux-gnueabihf-ld -EL    -r -o net/netfilter/nfnetlink_queue.o net/netfilter/nfnetlink_queue_core.o ; scripts/mod/modpost net/netfilter/nfnetlink_queue.o
