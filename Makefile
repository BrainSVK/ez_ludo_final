all: k_s_server k_s_client

k_s_server: k_s_server.o
	gcc k_s_definitions.c clovece.c -pthread k_s_server.c -o k_s_server.o

k_s_client: k_s_client.o
	gcc k_s_definitions.c clovece.c -pthread k_s_client.c -o k_s_client.o

clean:
	rm *.o





