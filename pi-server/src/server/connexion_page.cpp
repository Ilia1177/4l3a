#include "Server.hpp"

void Server::connexion_page(Client* client)
{
	Minitel* machine;

	client->currentPage = CONNINFO;
	machine = client->minitel;
	machine->newScreen();
	machine->newXY(1, 2);
	machine->attributs(FOND_VERT);
	machine->println("Nom: HazardousÉditorial");
	machine->print("Mot de passe:");
	machine->attributs(FOND_VERT);
	machine->println(" RisoPrint");
	machine->attributs(FOND_BLANC);
	machine->attributs(CARACTERE_NOIR);
	machine->displayPng("qrcode.png", 128);
	machine->attributs(INVERSION_FOND);
    machine_print_infos(client);
}

int Server::connexion_input(Client* client)
{
	Minitel* machine;
	unsigned long key;
	char c;
	static std::string input_command;

	machine = client->minitel;
	key = machine->getKeyCode();
	switch(key) {
		case ANNULATION:
			main_page(client);
			break;
		case '\r': case ENVOI:
			handle_client_command(client, input_command);
			break;
		default:
			c = appendCodepoint(input_command, key);
			if(input_command.length() < 10 && c > 0)
				machine->printChar(c);
			break;
	}
	return 0;
}
