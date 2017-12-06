/*
 * Modul: main.cpp
 * Autor: Jiri Matejka
 * Popis: Soubor s definici funkce main a inicializaci globalnich promennych.
 */
#include "main.hpp"
Facility cerpadlo;
Facility mixer;
Facility stroj;

bool zavzdusneno       = false;
bool pracovni_doba     = true;

unsigned den = 0;
unsigned jednotka_casu = 1;
unsigned poruch_filtr = 0;
unsigned poruch_stroje = 0;
unsigned poruch_cepele = 0;
unsigned poruch_zatvrdnuti = 0;
unsigned mixer_obsazeno  = 0;
unsigned nadoba_obsazeno = 0;
unsigned stroj_obsazeno  = 0;

unsigned VSTUPNI_MNOZSTVI_COKOLADY = 50;
unsigned delka_jednotky_casu = DEN * 7;

long unsigned int zmetci = 0;

long long unsigned int vyrobeno_cokolady = 0;

int main(int argc, char const *argv[]) {
	unsigned delka_simulace     = 3650 * DEN;
	double aktivace_cepele      = Exponential( 365 * DEN );
	double aktivace_filtru      = Exponential( 43  * DEN );
	double aktivace_zavzdusneni = Exponential( 14  * DEN );
#ifdef EXAMPLE
	delka_simulace      = 7 * DEN;
	delka_jednotky_casu = 1 * DEN;
#endif
#ifdef CEPEL
	delka_simulace      = 30 * DEN;
	aktivace_cepele     = Uniform( 2 * DEN, 20 * DEN );
	aktivace_filtru     = aktivace_zavzdusneni = 365 * DEN;
	delka_jednotky_casu = 1 * DEN;
#endif
#ifdef FILTR
	delka_simulace      = 1 * DEN;
	aktivace_filtru     = Uniform( 1 * HODINA, 8 * HODINA );
	aktivace_cepele     = aktivace_zavzdusneni = 365 * DEN;
	delka_jednotky_casu = 1 * HODINA;
#endif
#ifdef STROJ
	delka_simulace       = 1 * DEN;
	aktivace_zavzdusneni = Uniform( 1 * HODINA, 12 * HODINA );
	aktivace_cepele      = aktivace_filtru = 365 * DEN;
	delka_jednotky_casu  = 1 * HODINA;
#endif
	if ( argc ) {
		VSTUPNI_MNOZSTVI_COKOLADY = atoi( argv[1] );
	}
	info( "Zaciname!" );
	debug( "Vstupni mnozstvi cokolady", VSTUPNI_MNOZSTVI_COKOLADY );
	Init( 0, delka_simulace );
	(new Zamestnanec)->Activate();
	(new Zavzdusneni)->Activate( aktivace_zavzdusneni );
	(new Ucpani_filtru)->Activate( aktivace_filtru );
	(new Porucha_cepele)->Activate( aktivace_cepele );
	(new Statistika)->Activate( delka_jednotky_casu );
	(new Pracovni_doba)->Activate();
	Run();
	//mixer.Output();
	info( "Koncime!" );
	return 0;
}
