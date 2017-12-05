#include <simlib.h>
#include <iostream>
#ifdef DEBUG
#define debug( klic, hodnota ) do { std::cerr << static_cast<int>(Time) << ": " << (klic) << ": " << (hodnota) << std::endl; } while(0)
#define info( retezec ) do { std::cerr << static_cast<int>(Time) << ": " << (retezec) << std::endl; } while(0)
#else
#define debug( klic, hodnota ) ;
#define info( retezec ) ;
#endif

#define SEKUNDA 1
#define MINUTA  60
#define HODINA  3600
#define DEN     86400

#define KAPACITA_MIXER  100
#define KAPACITA_STROJ  40
#define KAPACITA_NADOBA 100

extern unsigned VSTUPNI_MNOZSTVI_COKOLADY;

extern long unsigned int zmetci;
extern long long unsigned int vyrobeno_cokolady;

extern unsigned poruch_filtr;
extern unsigned poruch_stroje;
extern unsigned poruch_cepele;
extern unsigned poruch_zatvrdnuti;
extern unsigned mixer_obsazeno;
extern unsigned nadoba_obsazeno;
extern unsigned stroj_obsazeno;
extern unsigned jednotka_casu;
extern unsigned den;

extern bool zavzdusneno;
extern bool cokolada_zatvrdla;
extern bool rozbita_cepel;
extern bool zaneseny_filtr;
extern bool pracovni_doba;

extern Facility cerpadlo;
extern Facility mixer;
extern Facility stroj;

class Pracovni_doba: public Process {
	void Behavior() {
		while ( 1 ) {
			if ( den < 5) {
				Wait( 15 * HODINA );
				pracovni_doba = false;
				Wait( 9 * HODINA );
				pracovni_doba = true;
				den++;
			}
			else {
				pracovni_doba = false;
				Wait( 2 * DEN );
				den = 0;
				pracovni_doba = true;
			}
		}
	}
};

class Statistika: public Process {
	void Behavior() {
		std::cout << "Cas,Poruch cepel,Poruch filtru,Poruch stroje,Zatvrdnuti cokolady,Vyrobeno cokolady" << std::endl;
		while (1) {
			std::cout << jednotka_casu << ",";
			std::cout << poruch_cepele << ",";
			std::cout << poruch_filtr << ",";
			std::cout << poruch_stroje << ",";
			std::cout << poruch_zatvrdnuti << ",";
			std::cout << vyrobeno_cokolady/1000.0 << std::endl;
			jednotka_casu++;
			poruch_stroje     = 0;
			poruch_cepele     = 0;
			poruch_filtr      = 0;
			poruch_zatvrdnuti = 0;
			vyrobeno_cokolady = 0;
			Wait( DEN * 7 );
		}
	}
};

class Zavzdusneni: public Event {
	void Behavior() {
		info( "Porucha: Zavzdusnil se stroj" );
		zavzdusneno = true;
		poruch_stroje++;
		Activate( Time + Exponential( 14 * DEN ) );
	}
};

class Oprava_cepele: public Process {
	void Behavior() {
		info( "Technik: Chci mixer, abych opravil cepel" );
		Seize( mixer, 3 );
		info( "Technik: Opravuji cepel" );
		zmetci += mixer_obsazeno;
		mixer_obsazeno = 0;
		Wait( Exponential( 5 * DEN ) );
		rozbita_cepel = false;
		Release( mixer );
		info( "Technik: cepel opravena" );
	}
};

class Ucpani_filtru: public Event {
	void Behavior() {
		info( "Porucha: Zaneseny filtr" );
		zaneseny_filtr = true;
		poruch_filtr++;
		Activate( Time + Exponential( 43 * DEN ) );
	}
};

class Porucha_cepele: public Event {
	Event * ptr;
public:
	Porucha_cepele( Event * e ) : ptr(e) {}
	void Behavior() {
		info( "Porucha: Rozbila se cepel" );
		delete ptr;
		rozbita_cepel = true;
		poruch_cepele++;
		( new Oprava_cepele )->Activate();
		Activate( Time + Exponential( 365 * DEN ) );
		ptr = new Ucpani_filtru();
		ptr->Activate( Time + Exponential( 43 * DEN ) );
	}
};

class Oprava_filtru: public Process {
	void Behavior() {
		info( "Technik: Chci mixer, abych opravil filtr." );
		Seize( mixer, 0 );
		info( "Technik: Opravuji filtr" );
		Wait( Uniform( HODINA * 1, HODINA * 4 ) );
		zaneseny_filtr = false;
		Release( mixer );
		info( "Technik: Filtr opraven." );
	}
};

class Cokolada: public Process {
public:
	void Behavior() {
		Seize( stroj, 1 );
		Wait( SEKUNDA * 18 );
		stroj_obsazeno--;
		Release( stroj );
		double what_now =  Random();
		if ( what_now <= 0.1 ) {
			info( "Cokolada: Jsem zmetek." );
			zmetci++;
		}
		else {
			vyrobeno_cokolady++;
			debug( "Cokolada: Vyrobeno cokolady", vyrobeno_cokolady );
		}
	}
};

class Tvrdnuti_cokolady: public Event {
private:
	Process *ptr;
public:
	Tvrdnuti_cokolady(Process *p): ptr(p) {
		Activate( Time + Uniform( MINUTA * 15, MINUTA * 20 ) );
	}
	void Behavior() {
		info( "Porucha: Zatvrdla cokolada, zacinam proces rozehrivani." );
		poruch_zatvrdnuti++;
		ptr->Wait( Uniform( MINUTA * 30, MINUTA * 40 ) );
	}
};

class Zamestnanec: public Process {
private:
	void odvzdusni_stroj() {
		Seize( stroj, 0 );
		info( "Zamestnanec: Odvzdusnuji stroj." );
		Wait( Exponential( MINUTA * 15 ) );
		zavzdusneno = false;
		Release( stroj );
	}

	void obsluhuj_mixer() {
		// Pokud je u cerpadla nejaka cokolada, nebudu mixovat!
		if ( nadoba_obsazeno > 0 ) {
			info( "Zamestnanec: V nadobe na cokoladu neco je, musim to prvne zpracovat." );
			return;
		}
		Seize(mixer, 1); // Zamestnanec chce mixer
		debug( "Zamestnanec: Zacinam plnit mixer, momentalne v nem je cokolady", mixer_obsazeno );
		// Zamestnanec ma mixer a zacina ho plnit
		for( unsigned i = mixer_obsazeno; i < VSTUPNI_MNOZSTVI_COKOLADY && i < KAPACITA_MIXER; i++ ) {
			if ( zmetci > 0 ) {
				zmetci--;
			}
			Wait( Uniform( 3 * SEKUNDA, 5 * SEKUNDA ) );
			mixer_obsazeno++;
			debug( "Zamestnanec: Dal jsem kilo cokolady do mixeru, momentalne v nem je cokolady", mixer_obsazeno );
		}
		debug( "Zamestnanec: Mixer je naplnen, jdu mixovat, je v nem cokolady", mixer_obsazeno );
		// Mixer je naplnen a zaciname mixovat
		Wait( Uniform( 14 * MINUTA , 17 * MINUTA ) );
		nadoba_obsazeno += mixer_obsazeno;
		mixer_obsazeno = 0;
		Release(mixer);
		info( "Zamestnanec: Odchazim od mixeru." );
	}

	void pln_cerpadlo() {
		// Chci cerpadlo
		Seize( cerpadlo, 1 );
		// Zpracuji kilo cokolady
		Wait( 12 * SEKUNDA );
		nadoba_obsazeno--;
		(  new Cokolada )->Activate();
		Release( cerpadlo );
	}

	void Behavior() {
		while ( 1 ) {
			WaitUntil( pracovni_doba );
			info( "Zamestnanec: Prisel jsem do prace" );
			if ( zavzdusneno ) {
				info( "Zamestnanec: Zavzdusnil se mi stroj" );
				odvzdusni_stroj();
				info( "Zamestnanec: Zavzdusneny stroj je znovu funkcni" );
				continue;
			}
			obsluhuj_mixer();
			Tvrdnuti_cokolady * tvrdnuti = new Tvrdnuti_cokolady( this );
			while ( nadoba_obsazeno > 0 ) {
				WaitUntil( stroj_obsazeno < KAPACITA_STROJ );
				stroj_obsazeno++;
				pln_cerpadlo();
				debug( "Zamestnanec: Precerpal jsem kilo cokolady, zbyva", nadoba_obsazeno );
			}
			delete tvrdnuti;
		}
	}
};