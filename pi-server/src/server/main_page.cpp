#include "Server.hpp"

void Server::main_page(Client* client)
{
    Minitel machine = *(client->minitel);

    client->currentPage = MAIN_PAGE;
	machine.echo(false);
    machine.newScreen();
    machine.println00("Hazardous Éditorial: labo graphique");
    machine.moveCursorDown(2);
    machine.attributs(CARACTERE_BLANC);
    machine.attributs(INVERSION_FOND);
    machine.print("Bienvenue au laboratoire d'impression numerique.\
			Saissez votre choix sur le pavé numérique.");
    machine.cancel();
    machine.attributs(FOND_NORMAL);
    machine.rect(5, 7, 35, 18);
    machine.moveCursorXY(8, 10);
    machine.println("1. Risographie ?");
    machine.moveCursorRight(7);
    machine.println("2. Impression numérique");
    machine.moveCursorRight(7);
    machine.println("3. Connection internet");
    machine.moveCursorRight(7);
    machine.println("4. Hazardous Collective");

    machine.moveCursorXY(1, 20);

    machine.attributs(CARACTERE_BLANC);
    machine.attributs(INVERSION_FOND);
    machine.print("Explore l'impression numerique");
    machine.cancel();
    machine.attributs(FOND_NORMAL);
    machine_print_infos(client);
}

int Server::main_page_input(Client* client)
{
	static std::string input;
	char c;
	Minitel* machine;
    unsigned long key;

	machine = client->minitel;
	key = machine->getKeyCode();
    switch (key) {
    case '1':
        risographie_page(client);
        break;
    case '2':
        main_page(client);
        break;
    case '3':
		connexion_page(client);
        break;
    case '4':
        break;
	case '\r': case ENVOI:
		handle_client_command(client, input);
		break;
    case CONNEXION_FIN:
        return 0;
    default:
		c = appendCodepoint(input, key);
		if(input.length() < 10 && c > 0)
			machine->printChar(c);
    }
    return 1;
}

