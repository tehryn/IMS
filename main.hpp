/** \file main.hpp
 * Soubor obsahujici tridy s pretizenymi metodami Behavior. Autory tohoto
 * souboru jsou Jiri Matejka a Miroslava Misova
 */
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

/// Mnozstvi cokolady davajici se do mixeru
extern unsigned VSTUPNI_MNOZSTVI_COKOLADY;

/// Pocet zmetku cekajicich na zpracovani
extern long unsigned int zmetci;
/// Pocet vyrobene cokolady za jednotku casu. Jednotka casu se nastavuje ve tride Statistika
extern long long unsigned int vyrobeno_cokolady;

/// Pocet poruch filtru za jednotku casu
extern unsigned poruch_filtr;
/// Pocet poruch stroje za jednotku casu
extern unsigned poruch_stroje;
/// Pocet poruch cepele za jednotku casu
extern unsigned poruch_cepele;
/// Pocet zatvrdnuti cokolady za jednotku casu
extern unsigned poruch_zatvrdnuti;
/// Aktualni pocet cokolady v mixeru
extern unsigned mixer_obsazeno;
/// Aktualni pocet cokolady v nadobe za mixerem
extern unsigned nadoba_obsazeno;
/// Aktualni pocet cokolady v nadobe pred strojem
extern unsigned stroj_obsazeno;
/// V jakem case se nachazime. Cas je v jednotce casu, ktera se nastavuje ve tride Statistika
extern unsigned jednotka_casu;
/// Den v tydnu, 0-4 jsou pracovni dny, 5 a 6 jsou vikendy
extern unsigned den;
/// Interval vypisovani statistiky v sekundach
extern unsigned delka_jednotky_casu;

/// Promenna urcujici zavzdusneni stroje
extern bool zavzdusneno;
/// Promenna urcujici pracovni dobu
extern bool pracovni_doba;

/// Promenna reprezentujici cerpadlo
extern Facility cerpadlo;
/// Promenna reprezentujici mixer
extern Facility mixer;
/// Promenna reprezentujici stroj
extern Facility stroj;

/**
 * Proces pracovni doby. Meni hodnoty globalni promenne pracovni_doba.
 */
class Pracovni_doba: public Process {
	void Behavior() {
		// Po spusteni bezi neustale. Urcuje pracovni dobu Techniku a Zamestnancu.
		while ( 1 ) {
			if ( den < 5) {
				// zacatek pracovni doby
				Wait( 15 * HODINA );
				info("Pracovni doba: Den skoncil, dokoncete rozdelanou praci a jdete domu.");
				pracovni_doba = false;
				// konec pracovni doby
				Wait( 9 * HODINA );
				info( "Pracovni doba: Zacatek noveho dne, prosim hlaste se na svych stanovistich." );
				pracovni_doba = true;
				// Vzhuru do dalsiho dne
				den++;
			}
			else {
				// Je vikend!
				info("Pracovni doba: Nastal vikend, pokud nenastala zadna porucha, uzijte si volno.");
				pracovni_doba = false;
				Wait( 2 * DEN );
				// Je pondeli...
				info("Pracovni doba: Nastalo pondeli, volno skoncilo.");
				den = 0;
				pracovni_doba = true;
			}
		}
	}
};

/**
 * Proces, ktery generuje csv na standartni vystup. Vystup tohoto procesu lze pouzit
 * primo na generovani grafu.
 */
class Statistika: public Process {
	void Behavior() {
		std::cout << "Cas,Poruch cepele,Poruch filtru,Poruch stroje,Zatvrdnuti cokolady,Vyrobeno cokolady" << std::endl;
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
			Wait( delka_jednotky_casu );
		}
	}
};

/**
 * Udalost zavzdusneni pouze nastavi globalni promennou a konci. Porucha zavzdusneni
 * neni nijak fatalni a je opravena samotnym Zamestnancem, pote co stroj zpracuje
 * veskerou cokoladu na lince.
 */
class Zavzdusneni: public Event {
	void Behavior() {
		info( "Porucha: Zavzdusnil se stroj" );
		zavzdusneno = true;
		poruch_stroje++;
	}
};

/**
 * Proces poruch a oprav cepele.
 */
class Porucha_cepele: public Process {
	void Behavior() {
		while(1) {
			info( "Porucha: Rozbila se cepel" );
			// Cekam do pracovni doby, porucha nemuze byt nahlasena technikovi
			// mimo pracovni dobu
			WaitUntil( pracovni_doba );
			poruch_cepele++;
			// Technik byl puvodne process a porucha event, ale neni treba to delit,
			// statistiku to nijak neovlivni a proces neni nic jineho nez posloupnost
			// udalosti.
			info( "Technik: Chci mixer, abych opravil cepel" );
			// zaberu mixer, pokud ho ma Zamestnanec, tak mu ho seberu
			Seize( mixer, 3 );
			info( "Technik: Opravuji cepel" );
			// vsechnu cokoladu v mixeru musim hodit do zmetku
			zmetci += mixer_obsazeno;
			mixer_obsazeno = 0;
			// mixer se opravuje i o vikendu, bez nej cela linka stoji, takze
			// (nastesti) neni treba resit vikendy. V noci se samozrejmne neopravuje
			// ale to je zahrnuto uz v dobe opravy.
			Wait( Uniform( DEN * 4, DEN * 7 ) );
			// Mixer mohu predat zamestnanci az behem pracovni doby
			WaitUntil( pracovni_doba );
			Release( mixer );
			info( "Technik: cepel opravena" );
			Wait( Exponential( 365 * DEN ) );
		}
	}
};

/**
 * Proces ucpani a oprav filtru
 */
class Ucpani_filtru: public Process {
	void Behavior() {
		while (1) {
			info( "Porucha: Zaneseny filtr" );
			// Porucha nemuze byt nahlasena technikovi mimo pracovni dobu
			WaitUntil( pracovni_doba );
			poruch_filtr++;
			// Technik byl puvodne process a porucha event, ale neni treba to delit,
			// statistiku to nijak neovlivni a proces neni nic jineho nez posloupnost
			// udalosti.
			info( "Technik: Chci mixer, abych opravil filtr." );
			// Pokud se mixuje, oprava filtru pocka
			Seize( mixer, 1 );
			info( "Technik: Opravuji filtr" );
			// Toto asi nastat nemuze, ale jen pro jistotu vyhodim cokoladu
			// z mixeru do zmetku.
			zmetci += mixer_obsazeno;
			mixer_obsazeno = 0;
			Wait( Uniform( HODINA * 1, HODINA * 4 ) );
			Release( mixer );
			info( "Technik: Filtr opraven." );
			Wait( Exponential( 43 * DEN ) );
		}
	}
};

/**
 * Proces cokolady. Cokolada si zabira stroj a nasledne generuje bud zmetky nebo
 * navysuje pocet vyrobene cokolady.
 */
class Cokolada: public Process {
public:
	void Behavior() {
		// Cokolada ma prednost pred poruchou, porucha neni totiz fatalni
		// a je lepsi pokracovat ve vyrobe, nez aby zacala na cele lince tvrdnout
		// cokolada
		Seize( stroj, 1 );
		Wait( SEKUNDA * 18 );
		stroj_obsazeno--;
		Release( stroj );
		// Ted se musim rozhodnout, zda jsem vyrobil zmetek. Produkce zmetku
		// je zvysena se zavzdusnenim stroje, ale o kolik, to nikdo nevi. Ale podle
		// vseho to je jiz zahrnute v 10% procentech.
		double what_now =  Random();
		if ( what_now <= 0.1 ) {
			zmetci++;
			debug( "Cokolada: Jsem zmetek. Aktualni pocet zmetku", zmetci );
		}
		else {
			vyrobeno_cokolady++;
			debug( "Cokolada: Vyrobeno cokolady", vyrobeno_cokolady );
		}
	}
};

/**
 * Udalost tvrdnuti cokolady. Po urcite dobe pozastavi Zamestnance, ten musi
 * rozehrat zatvrdlou cokoladu.
 */
class Tvrdnuti_cokolady: public Event {
private:
	/// Ukazatel na zamestnance
	Process *ptr;

	void Behavior() {
		info( "Porucha: Zatvrdla cokolada, zacinam proces rozehrivani." );
		poruch_zatvrdnuti++;
		ptr->Passivate();
		ptr->Activate( Time + Uniform( MINUTA * 30, MINUTA * 40 ) );
	}
public:
	/**
	 * Konstruktor objektu tridy.
	 * @param p Ukazatel na zamestnance
	 */
	Tvrdnuti_cokolady(Process *p): ptr(p) {
		Activate( Time + Uniform( MINUTA * 15, MINUTA * 20 ) );
	}
};

/**
 * Proces reprezentujici zamestnance
 */
class Zamestnanec: public Process {
private:
	/**
	 * Odvzdusneni stroje
	 */
	void odvzdusni_stroj() {
		// Odvzdusnovat se zacne, az stroj prestane pracovat
		Seize( stroj, 0 );
		info( "Zamestnanec: Odvzdusnuji stroj." );
		Wait( Exponential( MINUTA * 15 ) );
		zavzdusneno = false;
		Release( stroj );
		// Aktivace nove poruchy
		( new Zavzdusneni )->Activate( Time + Exponential( 14 * DEN ) );
	}

	/**
	 * Obsluha Mixeru - Vlozeni cokolady do mixeru a nasledne samotne mixovani.
	 */
	void obsluhuj_mixer() {
		// Pokud je u cerpadla nejaka cokolada, nebudu mixovat!
		if ( nadoba_obsazeno > 0 ) {
			info( "Zamestnanec: V nadobe na cokoladu neco je, musim to prvne zpracovat." );
			return;
		}
		Seize(mixer, 1); // Zamestnanec chce mixer
		debug( "Zamestnanec: Zacinam plnit mixer, momentalne v nem je cokolady", mixer_obsazeno );
		// Zamestnanec ma mixer a zacina ho plnit
		while ( mixer_obsazeno < VSTUPNI_MNOZSTVI_COKOLADY && mixer_obsazeno < KAPACITA_MIXER ) {
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

	/**
	 * Precerpa 1 kilo cokolady
	 */
	void pln_cerpadlo() {
		// Chci cerpadlo
		Seize( cerpadlo, 1 );
		// Zpracuji kilo cokolady
		Wait( 12 * SEKUNDA );
		nadoba_obsazeno--;
		// Dale se cokolada o sebe musi postarat sama
		(  new Cokolada )->Activate();
		Release( cerpadlo );
	}

	void Behavior() {
		while ( 1 ) {
			// Pokud neni pracovni doba, nezacinam novy cyklus obsluhy linky
			WaitUntil( pracovni_doba );
			info( "Zamestnanec: Kontroluji zda nenastala zadna porucha." );
			if ( zavzdusneno ) {
				info( "Zamestnanec: Zavzdusnil se mi stroj." );
				odvzdusni_stroj();
				info( "Zamestnanec: Zavzdusneny stroj je znovu funkcni." );
				continue;
			}
			// Obsluha mixeru
			obsluhuj_mixer();
			// Po obsluze mixeru nam cokolada zacina tvrdnout
			Tvrdnuti_cokolady * tvrdnuti = new Tvrdnuti_cokolady( this );
			// Dokud se vse neprecerpa, musim michat s cokoladou a hrnout ji do
			// cerpadla.
			while ( nadoba_obsazeno > 0 ) {
				// Pokud neni misto, kam bych precerpal cokoladu, nezbyva mi nic
				// jineho nez vypnout cerpadlo a doufat, ze mi to mezitim neztvrdne.
				WaitUntil( stroj_obsazeno < KAPACITA_STROJ );
				stroj_obsazeno++;
				pln_cerpadlo();
				debug( "Zamestnanec: Precerpal jsem kilo cokolady, zbyva", nadoba_obsazeno );
			}
			// Po precerpani uz nehrozi zatvrdnuti cokolady, ve stroji uz je zase teplo.
			delete tvrdnuti;
		}
	}
};