#ifndef GLOBALPARAMS_H_
#define GLOBALPARAMS_H_

#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <libxml/parser.h>
#include "Url.h"

using namespace std;

#define IRRELEVANT 1

/**
 * @class GlobalParams
 * @brief Classe que conté els paràmetres globals de l'aplicació.
 * 
 * Classe estàtica que conté tots els paràmetres globals necessaris per a l'execució de l'aplicació. 
 * 
 * @author Miquel Esplà i Gomis
 */
class GlobalParams
{
private:

	/**
	 * Variable que conté la ruta on es troba el fitxer de configuració de l'aplicació.
	 */
	static string config_file;

	/**
	 * Màxima distància d'edició (en distància absoluta) permesa per a establir una possible correspondència entre fitxers HTML.
	 * Si aquest paràmetre val -1, s'assumira que no existeix una distància d'edició màxima en la comparació. 
	 */
	static double max_edit_distance_length_absolute;

	/**
	 * Màxima distància d'edició (en distància percentual) permesa per a establir una possible correspondència entre fitxers HTML.
	 * Si aquest paràmetre val -1, s'assumira que no existeix una distància d'edició màxima en la comparació. 
	 */
	static double max_edit_distance_length_percentual;

	/**
	 * Distància màxima de profunditat en l'arbre de directoris que poden tenir dos fitxers web a
	 * comparar. Només es compararan els fitxers que es troben a una distància de profunditat a
	 * l'arbre de directoris igual o major que la indicada per aquest paràmetre. Si el valor és 0
	 * només es compararan els fitxers que estroben al mateix directori.
	 */
	static int directory_depth_distance;

	/**
	 * Per al càlculs de la distància d'edició en de dos <code>WebFile</code> ens basem en dos elements,
	 * les etiquetes HTML i la logitud del text contingut entre elles. Per a poder fer la comparativa
	 * entre ambdós paràmetres, cal conèixer la relació o l'impacte que produeix en la comparació la di-
	 * ferència entre una etiqueta i la diferència del nombre de caràcters del text contingut. És a dir,
	 * necessitem saber quants caràcters de diferència entre dos textos del fitxer web es poden equiparar
	 * amb el fet què tinguen una etiqueta diferent. Aquest paràmetre no el messurarem directament com un
	 * nombre de caràcters, sinó com un percentatge sobre la mitjana del total de caracters d'ambdós textos.
	 * El seu valor serà contingut en aquesta variable.
	 */
	static double text_distance_percent_differenciator;

	/**
	 * Percentatges de diferència de tamany màxim que es permet entre dos fitxers escrits en dues llengües
	 * concretes, de tal forma que si s'excedeix aquest percentatge, es consideraran fitxers diferents.
	 */
	static map<wstring,double> file_size_diference_percents;

	/**
	 * Percentatge de diferència de tamany màxim que es permet entre dos fitxers, tal que si s'excedeix aquest
	 * percentatge, es consideraran fitxers diferents. Aquest valor només és utilitzat en els parells de fitxers
	 * escrits en llengües que no estiguen reflectides al map file_size_diference_percents.
	 */
	static double file_size_difference_percent;

	/**
	 * Ruta del fitxer de configuració de la llibreria TextCat.
	 */
	static wstring textcat_config_file;

	/**
	 * Temps màxim de descàrrega.
	 */
	static int downloaded_size;

	/**
	 * Paràmetre que indica la ruta en què s'ha de guardar les pàgines descarregades amb el mòdul de descàrrega.
	 */
	static wstring download_path;

	/**
	 * Indicador que permet forçar a bitextor a consultar l'idioma de cada fitxer o permetre que ell mateix intente esbrinar-lo.
	 */
	static bool guess_language;

	/**
	 * Directori base on es troben els fingerprints.
	 */
	static wstring fingerprints_dir;
	
	/**
	 * Màxima distància entre text (total) permesa en percentatge.
	 */
	static double max_total_text_lenght_diff;

	/**
	 * Màxima distància permesa entre els arrays d'enters, en valor absolut.
	 */
	static int max_nfingerprint_distance;

	/**
	 * Flag que indica si s'han de guradrar tots els bitextos en un mateix fitxer.
	 */
	static bool all_bitexts_in_one;

	/**
	 * Flag que indica si cal crear un quadern de bitàcora amb totes les operacions realitzades.
	 */
	static wofstream log_file;

	/**
	 * Mida mínia de l'array d'etiquetes-blocs de text per a que la comparació siga realitzada.
	 */
	static int min_array_size;

	/**
	 * Flag que indica si s'han de mostrar per pantalla les operacions realitzades.
	 */
	static bool verbose;

	/**
	 * Flag que indica si s'han de crear tots els possibles candidats a bitext o només el millor aparellament per a cada fitxer.
	 */
	static bool create_all_candidates;

	/**
	 * Mètode que genera el fitxer de configuració de LibTextCat.
	 */
	static void GenerateTextCatConfigFile();

	/**
	 * Flag que indica si s'han de crear o no aquelles parelles per a les quals s'hi troben candidats molt semblants (candidats ambigus).
	 */
	static double generate_ambiguous_bitexts;

	/**
	 *
	 */
	static bool generate_tmx;

	/**
	 * Flag que indica si cal crear un un fitxer en què escriure les parelles generades.
	 */
	static wofstream results_file;
	
	static map<UrlLangRule, pair<unsigned int, unsigned int> > url_lang_rules;
	
	

public:
	/**
	 * Llista de fingerprints per a la detecció d'idiomes.
	 */
	static map<wstring,wstring> fingerprints;

	/**
	 * Mètode que allibera tota la memòria reservada al fitxer de configuració.
	 */
	static void Clear();
	
	/**
	 * Mètode que permet obtenir la màxima distància d'edició en percentatge entre fitxers web establerta.
	 * @return Llindar que es establert com a màxima distància d'edició.
	 */
	static double GetMaxEditDistancePercentual();

	/**
	 * Mètode que permet obtenir la màxima distància d'edició en valor absolut entre fitxers web establerta.
	 * @return Llindar que es establert com a màxima distància d'edició.
	 */
	static double GetMaxEditDistanceAbsolute();
	
	/**
	 * Mètode que permet obtenir el precentatge de màxima distància entre textos per a
	 * considerar-los iguals.
	 * @return Percentatge de distància màxima de textos.
	 */
	static double GetTextDistancePercentDifferenciator();

	/**
	 * Mètode que permet establir el límit de profunditat en l'arbre de directoris per a la comparació
	 * de fitxers web.
	 * @param value Valor del límit que es vol establir. Si el valor que es passa és igual o menor a
	 * zero, només es comprovaran els fit xers que es troben en el mateix nivell de profunditat.
	 */
	static void SetDirectoryDepthDistance(const int &value);

	/**
	 * Mètode que permet establir el valor de la variable ja descrita <code>text_distance_percent_differenciator</code>.
	 * @param value Valor que es vol establir.
	 */
	static void SetTextDistancePercentDifferenciator(const double &value);
	
	/**
	 * Mètode que permet obtenir el límit de profunditat en l'arbre de directoris per a la comparació
	 * de fitxers web establert.
	 * @return Retorna el valor del límit de profunditat en l'arbre de directoris per a la comparació
	 * de fitxers web establert.
	 */
	static int GetDirectoryDepthDistance();
	
	/**
	 * Mètode que permet carregar els paràmetres globals des d'un fitxer XML.
	 * @param path Ruta del fitxer XML que conté la configuració dels paràmetres globals.
	 * @throw char* El mètode llança una excepció si el fitxer de configuració de TextCat especificat
	 * no existeix.
	 * @throw char* El mètode llança una excepció si el fitxer de configuració de TagAligner especificat
	 * no existeix.
	 * @return bool Retorna <code>true</code> si la càrrega s'ha realitzat de forma satisfactòria i
	 * <code>false</code> en cas contrari.
	 */
	static bool LoadGlobalParams(const string &path);
	
	/**
	 * Mètode que permet obtenir el percentatge màxim de diferència de mida de dos fitxers per a ser
	 * considerats el mateix fitxer.
	 * @return Retorna el valor del percentatge màxim de diferència de mida de dos fitxers per a ser
	 * considerats el mateix fitxer.
	 */
	static double GetFileSizeDifferencePercent();
	
	/**
	 * Mètode que permet establir el valor de la variable ja descrita <code>file_size_difference_percent</code>.
	 * @param value Valor que es vol establir.
	 */
	static void SetFileSizeDifferencePercent(const double &value);
	
	/**
	 * Mètode que permet establir la ruta on es troba el fitxer de configuració de la llibreria TextCat.
	 * @param path Ruta on es troba el fitxer.
	 */
	static void SetTextCatConfigFile(const wstring &path);
	
	/**
	 * Mètode que permet obtenir la ruta on es troba el fitxer de configuració de la llibreria TextCat.
	 * @return Retorna la ruta on es troba el fitxer.
	 */
	static wstring GetTextCatConfigFile();

	/**
	 * Mètode que processa recursivament els nodes del fitxer XML de configuració.
	 * @param node Node que es processarà.
	 * @param tagname Nom de la darrera etiqueta llegida a l'arbre d'etiquetes.
	 */
	static void ProcessNode(xmlNode* node, wstring tagname);

	/**
	 * Mètode que permet obtenir el nombre màxim de bytes que es descarregaran quan s'engegue el mòdul de descàrrega web.
	 * @return Retorna el nombre màxim de bytes que es descarregaran quan s'engegue el mòdul de descàrrega web.
	 */
	static int GetMaxDownloadedSize();

	/**
	 * Mètode que permet obtenir la ruta en què s'ha de guardar les pàgines descarregades amb el mòdul de descàrrega.
	 * @return Retorna la ruta en què s'ha de guardar les pàgines descarregades amb el mòdul de descàrrega.
	 */
	static wstring GetDownloadPath();
	
	/**
	 * Mètode que permet indicar a bitextor si ha d'esbrinar l'idioma dels fitxers o ha d'intentar consultar-lo a l'usuari.
	 * @param value Valor que es preten donar a aquest paràmetre.
	 */
	static void SetGuessLanguage(const bool &value);
	
	/**
	 * Mètode que permet saber si bitextor ha d'esbrinar l'idioma dels fitxers o ha d'intentar consultar-lo a l'usuari.
	 * @return Retorna true en cas que bitextor siga qui esbrina l'idioma i false si l'ha de consultar a l'usuari.
	 */
	static bool GetGuessLanguage();
	
	/**
	 * Mètode que permet afegir un percentatge a la llista de percentatges de distància de longitud entre blocs de text.
	 * El mètode pren com a clau les dues llengües indicades (els seus codis, que han de ser iguals que els del fitxer
	 * de configuració de LibTextCat) i els hi assigna un percentatge. Les entrades són duplicades de forma que l'ordre
	 * de les llengües no tinga importància. Això vol dir que per a una entrada dels idiomes català (ca) i esukera (eu)
	 * a la llista s'introduiria dues vegades el percentatge, amb clau "ca_eu" i "eu_ca".
	 * @param lang1 Codi que representa una de les llengües.
	 * @param lang2 Codi que representa l'altra llengua.
	 * @param percent Màxim percentatge permés com a diferència entre blocs de text de les dues llengües.
	 */
	static void AddFileSizeDiferencePercent(const wstring &lang1, const wstring &lang2, const double &percent);
	
	/**
	 * Mètode que permet prendre, a partir dels codis de dues llengües, el percentatge de màxima diferència establert per
	 * al parell d'idiomes. Si el parell no té assignat un màxim de diferència específic, se li assigna el percentatge per
	 * defecte.
	 * @return Retorna el percentatge de màxima diferència de longitud de blocs de text corresponent al parell d'idiomes.
	 */
	static double GetFileSizeDiferencePercent(const wstring &lang1, const wstring &lang2);

	/**
	 * Mètode que retorna la màxima distància permesa entre el text total dels fitxers.
	 * @return Retorna la màxima distància permesa entre el text total dels fitxers.
	 */
	static double GetMaxTotalTextLengthDiff();

	/**
	 * Mètode que retorna el número màxim de diferències permeses entre els arrays d'enters de dos WebFile.
	 * @return Retorna el número màxim de diferències permeses entre els arrays d'enters de dos WebFile.
	 */
	static int GetMaxNumericFingerprintDistance();

	/**
	 * Mètode que retorna el flag que indica si tots els bitextos s'han de guardar en el mateix fitxer.
	 * @return Retorna el flag que indica si tots els bitextos s'han de guardar en el mateix fitxer.
	 */
	static bool AllBitextInAFile();

	/**
	 * Mètode que retorna la mínima mida requerida per a l'array d'etiquetes-blocs de text per a considerar el fitxer candidat.
	 * @return Retorna la mínima mida requerida per a l'array d'etiquetes-blocs de text per a considerar el fitxer candidat.
	 */
	static int GetMinArraySize();

	/**
	 * Mètode que escriu el text indicat en el fitxer de log. En cas de que no s'haja activat este fitxer, no s'hi fa res.
	 * @param log_text Text que es vol escriure al fitxer de quadern de bitàcora.
	 */
	static void WriteLog(const wstring &log_text);

	/**
	 * Mètode que obre el fitxer de quadern de bitàcora.
	 * @param log_path Adreça del fitxer de log.
	 * @return Retorna <code>true</code> si s'ha creat correctament i <code>false</code> en cas contrari.
	 */
	static bool OpenLog(const string &log_path);

	/**
	 * Mètode que tanca el fitxer de quadern de bitàcora.
	 */
	static void CloseLog();

	/**
	 * Mètode que retorna el flag que indica si s'han de generar els bitextos amb tots els candidats dins dels paràmetres establerts per a cada fitxer web o només el millor candidat.
	 * @return Retorna <code>true</code> si s'han de generar els bitextos amb tots els candidats dins dels paràmetres establerts per a cada fitxer web o <code>false</code> si només s'ha de generar el millor candidat.
	 */
	static bool GetCreateAllCandidates();

	/**
	 * Mètode que retorna el flag que indica si cal executar l'aplicació en mode verbose o no.
	 * @return Retorna <code>true</code> si cal executar l'aplicació en mode verbose i <code>false</code> en cas contrari.
	 */
	static bool IsVerbose();

	/**
	 * Mètode que estableix l'execució a mode verbose.
	 */
	static void SetVerbose();

	/**
	 * Mètode que retorna el llindar de proximitat en número de caràcters per establir si dos fitxers són tan pareguts a un tercer que provoquen una generació ambígua.
	 * @return Retorna el llindar de proximitat en número de caràcters per establir si dos fitxers són tan pareguts a un tercer que provoquen una generació ambígua.
	 */
	static double GetGenerateAmbiguousBitexts();

	/**
	 * 
	 */
	static void GenerateTMX(bool generate);

	/**
	 * 
	 */
	static bool GetGenerateTMX();

	static void WriteResults(const wstring &result_text);

	static bool OpenResults(const string &results_path);

	/**
	 * Mètode que tanca el fitxer de resultats.
	 */
	static void CloseResults();

	static unsigned int AddUrlLangRule(UrlLangRule *rule);
};

#endif /*GLOBALPARAMS_H_*/
