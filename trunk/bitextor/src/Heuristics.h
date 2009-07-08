#ifndef HEURISTICS_H_
#define HEURISTICS_H_

#include "GlobalParams.h"
#include "WebFile.h"
#include <libtagaligner/ConfigReader.h>
#include <libtagaligner/EditDistanceTools.h>

/**
 * @class Heuristics
 * @brief Classe que que encapsula les heurístiques de l'aplicació.
 * 
 * Classe estàtica que conté tots els mètodes necessaris per a aplicar les heurístiques especificades
 * per a diferenciar els fitxers entre ells. Aquestes heurístiques són bàsicament:
 * -La diferenciació per mitjan de la comparació de les etiquetes HTML, combinada amb la comparació
 * de la longitud dels bloc de text inserits entre elles.
 * -La comparació de la mida dels fitxers en bytes.
 * -La comparació de les extensions dels fitxers.
 * -La diferència en la profunditat en l'arbre de directoris (aquesta és la única heurística que no
 * es troba implementada en esta classe, sinó en la classe <code>WebSite</code>, al mètode <code>GetMatchedFiles</code>. 
 * 
 * @author Miquel Esplà i Gomis
 */
class Heuristics
{
public:
	/**
	 * Mètode que indica si dos fitxers web poden ser el mateix basant-se l'extensió.
	 * @param wf1 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre un dels fitxers web amb el qual hem de fer la comparació.
	 * @param wf2 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre l'altre fitxer web amb el qual hem de fer la comparació.
	 * @return Retorna <code>true</code> si l'extensió d'ambdós fitxers és la mateixa, o <code>false</code> en cas contrari. 
	 */
	static bool HaveTheSameExtension(WebFile *wf1, WebFile *wf2);
	
	/**
	 * Mètode que indica si dos fitxers web poden ser el mateix basant-se en la seua mida en bytes. Per a activar aquest mètode cal donar un percentatge
	 * llindar per a la diferència de mida als paràmetres globals. Si no s'activa aquest llindar, el mètode retornarà sempre <code>true</code>.
	 * @param wf1 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre un dels fitxers web amb el qual hem de fer la comparació.
	 * @param wf2 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre l'altre fitxer web amb el qual hem de fer la comparació.
	 * @param result Paràmetre que serveix per a obtenir el percentatge de diferència de mida entre els dos fitxers. Si no es defineix el paràmetre, aquest adopta el valor NULL per defecte.
	 * @return Retorna <code>true</code> si la diferència de mida dels fitxers és acceptable segons els paràmetres establerts, o <code>false</code> en cas contrari. 
	 */
	static bool HaveAcceptableSizeDifference(WebFile *wf1, WebFile *wf2=NULL, double* result=NULL);
	
	/**
	 * Mètode que calcula la distància d'edició entre dues cadenes d'etiquetes HTML/Text tal com s'estableixen a la classe WebFile.
	 * Aquest mètode es basa en dos paràmetres globals. D'una banda, utilitza el llindar de percentatge de diferència entre longituds
	 * de text. Quan es calcula la distància d'edició entre dos blocs de text, si superen aquest percentatge, es consideren diferents
	 * i, en cas contrari, iguals. En cas que el paràmetre es trobe desactivat, es consideraran tots els blocs de text iguals.
	 * El segon paràmetre global és el del llindar de màxim percentatge de distància d'edició entre dos vectors d'etiquetes HTML/Text.
	 * Si aquest llindar està activat, la distància d'edició es limita, de tal forma que, en comptes de calcular-se tota la taula,
	 * es calcula només una diagonal determinada, basant-se en què tots els valors fora d'aquesta diagonal condueixen a resultats que
	 * excedeixen el llindar establert. La amplada d'aquesta diagonal serà directament proporcional al propi llindar.
	 * @param wf1 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre un dels fitxers web amb el qual hem de fer la comparació.
	 * @param wf2 Paràmetre de tipus <code>WebFile</code> que conté la informació sobre l'altre fitxer web amb el qual hem de fer la comparació.
	 * @param result Paràmetre que serveix per a obtenir la distància d'edició entre els vectors d'etiquetes HTML/Text dels dos fitxers. Si no es defineix el paràmetre, aquest adopta el valor NULL per defecte.
	 * @return Retorna la distància d'edició calculada com a enter major o igual a zero. En cas que la distància excedisca el màxim establert, el mètode retornarà -1.
	 */
	static bool HaveAcceptableEditDistance(WebFile *wf1, WebFile *wf2, wstring* pathdistance, double* result=NULL);

	static wstring lang1;
	static wstring lang2;

	/**
	 * Method wich calculates the cost in the edit distance function HTML tag vs. HTML tag.
	 * @param op Code of the operation wich will be performed (deletion, insertion, substitution).
	 * @param ctag1 First operand.
	 * @param ctag2 Second operand.
	 * @return Cost of the operation. 
	 */
	static double Cost(const short &op, const int &ctag1, const int &ctag2);
	
	
	static double CostTextAlignment(const short &op, const int &ctag1, const int &ctag2);

	/**
	 * Mètode que calcula la distància d'edició entre els dos arrays d'enters de dos fitxers web.
	 * @param wf1 Primer fitxer web de la comparació.
	 * @param wf2 Segon fitxer web de la comparació.
	 * @param result En cas què aquest paràmetre siga diferent de NULL, quan acaben els càlculs s'hi emmagatzemarà el resultat numeèric.
	 * @return Retorna <code>true</code> en cas què la distància siga inferior al llindar establert i <code>false</code> en cas contrari.
	 */
	static bool DistanceInNumericFingerprint(WebFile &wf1, WebFile &wf2, double *result=NULL);

	/**
	 * Mètode que calcula el cost de cada operació en la distància d'edició entre els arrays d'enters de cada fitxer web.
	 * @param op Codi de l'operació [inserció-eliminació-inserció].
	 * @param c1 Primer element a comprarar.
	 * @param c2 Segon element a comparar.
	 */
	static double CostNumbers(const short &op, const int &c1, const int &c2);

	/**
	 * Mètode que calcula si dos fitxers són prou similars per la seua llargària en caràcters.
	 * @param wf1 Primer fitxer web a comparar.
	 * @param wf2 Segon fitxer web a comprar.
	 * @param value En cas que aquest punter siga diferent a NULL, s'hi emmagatzemarà la diferència entre les longituds de text d'ambdós fitxers.
	 * @return En cas què la la diferència entre les longituds de text d'ambdós fitxers siga inferior al llindar establert retorna <code>true</code> i, en cas contrari, retorna <code>false</code>.
	 */
	static bool NearTotalTextSize(WebFile &wf1, WebFile &wf2, double *value=NULL);
	
	static double GetPhraseVariance(WebFile &wf1, WebFile &wf2, const wstring &pathdistance);
	
	static double GetPhraseVarianceDesviation(WebFile &wf1, WebFile &wf2, const wstring &pathdistance, const double &prhasevariance);
};

#endif /*HEURISTICS_H_*/
