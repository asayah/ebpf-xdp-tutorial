TARGET = ./labs/filtering/

BPF_C = ${TARGET:=xdp.c}
BPF_OBJ = ${BPF_C:.c=.o}

xdp: $(BPF_OBJ)
	bpftool net detach xdpgeneric dev eth0
	rm -f /sys/fs/bpf/tutorial_xdp
	bpftool prog load $(BPF_OBJ) /sys/fs/bpf/tutorial_xdp
	bpftool net attach xdpgeneric pinned /sys/fs/bpf/tutorial_xdp dev eth0 

$(BPF_OBJ): %.o: %.c
	clang -S \
	    -target bpf \
	    -D __BPF_TRACING__ \
	    -Ilibbpf/src\
	    -Wall \
		-g \
	    -Wno-unused-value \
	    -Wno-pointer-sign \
	    -Wno-compare-distinct-pointer-types \
	    -Werror \
	    -O2 -emit-llvm -c -o ${@:.o=.ll} $<
	llc -march=bpf -filetype=obj -o $@ ${@:.o=.ll}

clean:
	bpftool net detach xdpgeneric dev eth0
	rm -f /sys/fs/bpf/tutorial_xdp
	rm $(BPF_OBJ)
	rm ${BPF_OBJ:.o=.ll}




