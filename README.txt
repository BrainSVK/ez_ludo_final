V ramci aplikacie posiela klient textove spravy na server. Server nasledne prevedie male pismena na velke a naopak a vysledok posle klientovi.

Preklad servera:
	gcc k_s_definitions.c clovece.c -pthread k_s_server.c -o k_s_server
Preklad klienta:
	gcc k_s_definitions.c clovece.c -pthread k_s_client.c -o k_s_client

Spustenie servera:
	./k_s_server 10000
Spustenie klienta:
	./k_s_client localhost 10000
	
Ukoncenie aplikacie:
	klient zada spravu ":end"