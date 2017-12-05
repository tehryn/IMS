#include "main.hpp"

Facility cerpadlo;
Facility mixer;
Facility stroj;

bool zavzdusneno       = false;
bool cokolada_zatvrdla = false;
bool rozbita_cepel     = false;
bool zaneseny_filtr    = false;
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

long unsigned int zmetci = 0;

long long unsigned int vyrobeno_cokolady = 0;

int main(int argc, char const *argv[]) {
	if ( argc ) {
		VSTUPNI_MNOZSTVI_COKOLADY = atoi( argv[1] );
	}
	info( "Zaciname!" );
	debug( "Vstupni mnozstvi cokolady", VSTUPNI_MNOZSTVI_COKOLADY );
	Init( 0, 3650 * DEN );
	Zavzdusneni * zavzdusneni = new Zavzdusneni;
	zavzdusneni->Activate( Exponential( 14 * DEN ) );
	(new Ucpani_filtru)->Activate( Exponential( 43 * DEN ) );
	(new Porucha_cepele(zavzdusneni))->Activate( Exponential( 365 * DEN ) );
	(new Zamestnanec)->Activate();
	(new Statistika)->Activate( DEN * 7 );
	(new Pracovni_doba)->Activate();
	Run();
	info( "Koncime!" );
	return 0;
}
