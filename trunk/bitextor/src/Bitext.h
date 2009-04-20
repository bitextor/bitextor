#ifndef BITEXT_H_
#define BITEXT_H_

#include "WebFile.h"
#include "Heuristics.h"
#include "GlobalParams.h"
#include <libtagaligner/Aligner.h>
#include <libtagaligner/ConfigReader.h>

/**
 * @class Bitext
 * @brief Classe que conté els elements dels bitextos.
 * 
 * Classe que conté els elements derivats i necessaris d'un bitext. Açò són, molt ressumidament,
 * els dos textos a aparellar i la capacitat per a generar l'esmentat bitext. A més, en aquest
 * cas, també s'inclouen mètodes i paràmetres necessaris per a la comparació entre els fitxers
 * d'entrada.
 * 
 * @author Miquel Esplà i Gomis
 */
class Bitext
{
private:
	/**
	 * Indicador que controla si el bitext ha estat inicialitzat correctament.
	 */
	 bool is_initialized;

	/**
	 * El primer fitxer web del bitext.
	 */
	WebFile *wf1;

	/**
	 * El segon fitxer web del bitext.
	 */
	WebFile *wf2;
	
	/**
	 * Variable que es troba a <code>true</code> si ambdós fitxers tenen la mateixa extensió i
	 * <code>false</code> en cas contrari.
	 */
	bool same_extension;
	
	/**
	 * Percentatge de diferència de mida en bytes entre ambdós fitxers.
	 */
	double byte_size_distance;
	
	/**
	 * Percentatge de distància d'edició entre els arrays d'etiquetes/text d'ambdós fitxers.
	 */
	double edit_distance;
	
	/**
	 * Ruta d'aliniament per als bitextos.
	 */
	string alignment_path;
	
	/**
	 * Diferència de l'array de nombres trobats.
	 */
	unsigned int n_diff_numbers;
	
	/**
	 * Diferència en el total de text.
	 */
	unsigned int text_difference;
public:
	/**
	 * Constructor de la classe Bitext.
	 */
	Bitext();

	/**
	 * Destructor de la classe Bitext.
	 */
	virtual ~Bitext();

	/**
	 * Mètode que retorna el valor del flag d'inicialització.
	 * @return Retorna <code>true</code> si el bitext ha estat inicialitzat correctament i <code>false</code> en altre cas.
	 */
	bool IsInitialized();
	
	/**
	 * Mètode que incialitza la classe amb els fitxers web a partir dels quals es llegirà el text
	 * del bitext. Seguidament comprova si la llengua dels fitxers és diferent i si ambdós fitxers
	 * són prou similars com per a generar un bitext a partir d'ells.
	 * @param wf1 Un dels fitxers web que forme el bitext.
	 * @param wf2 L'altre fitxer web que forma el bitext.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna <code>true</code> si ambos fitxers són suficientment similars per a generar
	 * un bitext a partir d'ells i estan escrits en llengües diferents, mentre que retorna <code>false</code>
	 * en cas contrari.
	 */
	bool Initialize(WebFile *wf1, WebFile *wf2);
	
	/**
	 * Mètode que retorna el primer fitxer web del bitext.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna el primer fitxer web del bitext.
	 */
	WebFile* GetFirstWebFile();

	/**
	 * Mètode que retorna el segon fitxer web del bitext.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna el segon fitxer web del bitext.
	 */
	WebFile* GetSecondWebFile();
	
	/**
	 * Mètode que crea el bitext a partir dels dos fiters web introduïts en la inicialització.
	 * @param path Ruta en què es crearà el fitxer amb el bitext resultant.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna <code>true</code> si s'ha pogut generar el bitext i <code>false</code>
	 * en cas contrari.
	 */
	bool GenerateBitext(FILE * main_fout, unsigned int starting_tuid=0, unsigned int *last_tuid=NULL);
	
	/**
	 * Mètode que retorna <code>true</code> si ambdós fitxers tenen la mateixa extensió
	 * i <code>false</code> en cas contrari.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte
	 * no ha estat inicialitzat correctament.
	 * @return Retorna <code>true</code> si ambdós fitxers tenen la mateixa extensió i
	 * <code>false</code> en cas contrari.
	 */
	bool GetSameExtension();
	
	/**
	 * Mètode que retorna el percentatge de diferència de mida en bytes entre ambdós fitxers.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no
	 * ha estat inicialitzat correctament.
	 * @return Retorna el percentatge de diferència de mida en bytes entre ambdós fitxers.
	 */
	double GetSizeDistance();
	
	/**
	 * Mètode que retorna el percentatge de distància d'edició entre els arrays d'etiquetes/text
	 * d'ambdós fitxers.
	 * @throw char* El mètode retorna una excepció en forma de cadena de text si l'objecte no ha
	 * estat inicialitzat correctament.
	 * @return Retorna el percentatge de distància d'edició entre els arrays d'etiquetes/text
	 * d'ambdós fitxers.
	 */
	double GetEditDistance();

	/**
	 * Mètode que compara el bitext amb un altre i indica si el segon és millor.
	 * @param bitext Bitext per comparar l'actual bitext.
	 * @param disabled Si la diferència entre ambdós bitextos està per sota d'un detemrinat llindar, s'activa el flag disabled.
	 * @return Retorna <code>true</code> si el segon bitext és millor que l'actual i <code>false</code> en cas contrari.
	 */
	bool isBetterThan(Bitext &bitext, bool *disabled=NULL);
};

#endif /*BITEXT_H_*/
