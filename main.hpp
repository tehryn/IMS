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

#define VSTUPNI_MNOZSTVI_COKOLADY 50


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
			if ( poruch_cepele ) {
				std::cout << poruch_cepele << ",";
			}
			else {
				std::cout << ",";
			}
			if ( poruch_filtr ) {
				std::cout << poruch_filtr << ",";
			}
			else {
				std::cout << ",";
			}
			if ( poruch_stroje ) {
				std::cout << poruch_stroje << ",";
			}
			else {
				std::cout << ",";
			}
			if ( poruch_zatvrdnuti ) {
				std::cout << poruch_zatvrdnuti << ",";
			}
			else {
				std::cout << ",";
			}
			std::cout << vyrobeno_cokolady/1000.0 << std::endl;
			jednotka_casu++;
			poruch_stroje     = 0;
			poruch_cepele     = 0;
			poruch_filtr      = 0;
			poruch_zatvrdnuti = 0;
			vyrobeno_cokolady = 0;
			Wait( DEN );
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
		info( "Technik: Opravuji cepel" );
		Seize( mixer, 3 );
		Wait( Uniform( MINUTA * 40, MINUTA * 50 ) );
		rozbita_cepel = false;
		Release( mixer );
		info( "Technik: cepel opravena" );
	}
};

class Porucha_cepele: public Event {
	void Behavior() {
		info( "Porucha: Rozbila se cepel" );
		rozbita_cepel = true;
		poruch_cepele++;
		( new Oprava_cepele )->Activate();
		Activate( Time + Exponential( 365 * DEN ) );
	}
};

class Oprava_filtru: public Process {
	void Behavior() {
		info( "Technik: Opravuji filtr" );
		Seize( mixer, 3 );
		Wait( Uniform( HODINA * 1, HODINA * 4 ) );
		zaneseny_filtr = false;
		Release( mixer );
		info( "Technik: Filtr opraven." );
	}
};

class Ucpani_filtru: public Event {
	void Behavior() {
		info( "Porucha: Zaneseny filtr" );
		zaneseny_filtr = true;
		poruch_filtr++;
		( new Oprava_filtru )->Activate();
		Activate( Time + Exponential( 43 * DEN ) );
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
		info( "Pozastavuji zamestnance." );
		poruch_zatvrdnuti++;
		ptr->Wait( Uniform( MINUTA * 30, MINUTA * 40 ) );
	}
};

class Zamestnanec: public Process {
private:
	void odvzdusni_stroj() {
		info( "Zamestnanec: Je zavzdusneno, cekam na stroj" );
		Seize( stroj, 0 );
		info( "Zamestnanec: Je zavzdusneno, mam stroj" );
		Wait( Exponential( MINUTA * 15 ) );
		zavzdusneno = false;
		info( "Zamestnanec: Je Odvzdusneno" );
		Release( stroj );
	}

	void rozehrej_cokoladu() {
		Wait( Uniform( MINUTA * 30, MINUTA * 40 ) );
		info( "Zamestnanec: Cokoladu jsem rozehral" );
		cokolada_zatvrdla = false;
	}

	void obsluhuj_mixer() {
		// chci mixer
		Seize(mixer, 1);
		info( "Zamestnanec: Mam mixer" );
		debug( "Zamestnanec: Aktualni pocet zmentku", zmetci );
		// do mixeru hodim vsechny zmetky
		if ( zmetci < VSTUPNI_MNOZSTVI_COKOLADY ) {
			zmetci = 0;
		}
		else {
			zmetci -= VSTUPNI_MNOZSTVI_COKOLADY;
		}
		debug( "Zamestnanec: Novy pocet zmetku", zmetci );
		info( "Zamestnanec: Cekam az bude v kadi dostatek mista" );
		WaitUntil( KAPACITA_MIXER >= ( mixer_obsazeno + VSTUPNI_MNOZSTVI_COKOLADY ) );
		mixer_obsazeno += VSTUPNI_MNOZSTVI_COKOLADY;
		// Zamestnanec ma naplneny mixer cokoladou
		info( "Zamestnanec: mixuji" );
		// Zamestnanec spousti mixer
		Wait( Uniform( 14 * MINUTA , 17 * MINUTA ) );
		info( "Zamestnanec: Domixovano" );
		WaitUntil( KAPACITA_NADOBA >= ( nadoba_obsazeno + VSTUPNI_MNOZSTVI_COKOLADY ) );
		nadoba_obsazeno += VSTUPNI_MNOZSTVI_COKOLADY;
		// Za mixerem je dostatek mista v kadi na cokoladu
		for( int i = 0; i < VSTUPNI_MNOZSTVI_COKOLADY; i++ ) {
			mixer_obsazeno--;
		}
		Release(mixer);
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

public:
	void Behavior() {
		while ( 1 ) {
			info ( "Hura do prace." );
			WaitUntil( pracovni_doba );
			info ( "Jsem v praci." );
			if ( zavzdusneno ) {
				odvzdusni_stroj();
				continue;
			}
			info( "Zamestnanec: Chci mixer" );
			obsluhuj_mixer();
			Tvrdnuti_cokolady * tvrdnuti = new Tvrdnuti_cokolady( this );
			while ( nadoba_obsazeno > 0 ) {
				info( "Zamestnanec: Jsem u cerpadla" );
				WaitUntil( stroj_obsazeno < KAPACITA_STROJ );
				stroj_obsazeno++;
				if ( cokolada_zatvrdla ) {
					info("Zamestnanec: Zatvrdla mi cokolada");
					rozehrej_cokoladu();
					continue;
				}
				pln_cerpadlo();
				debug( "Zamestnanec: Presunul jsem kilo cokolady, zbyva", nadoba_obsazeno );
			}
			delete tvrdnuti;
		}
	}
};