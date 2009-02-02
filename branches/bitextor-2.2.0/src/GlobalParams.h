#ifndef GLOBALPARAMS_H_
#define GLOBALPARAMS_H_

#include <libtagaligner/configreader.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
#include <vector>

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
	static wstring config_file;

	/**
	 * Senyal que identifica si el limit de distància d'edició és percentual sobre els fitxers
	 * d'entrada o és absolut.
	 */
	 static bool is_percentual_edmax;

	/**
	 * Màxima distància d'edició permesa per a establir una possible correspondència entre fitxers HTML.
	 * Si aquest paràmetre val -1, s'assumira que no existeix una distància d'edició màxima en la comparació. 
	 */
	static float max_edit_distance_length;
	
	/**
	 * Mapejat d'etiquetes HTML marcades amb nombres enters i longituds de text. Les etiquetes coningudes
	 * a la llista "irrelevant_tags" es marcaran amb enters negatius. Els enters correspondan a longituds
	 * de text contingut entre les etiquetes.
	 */
	//static map< string,int > tag_map;
	
	/**
	 * Comptador descendent per al mapejat d'etiquetes HMLT. S'utilitza per a saber quin valor tindrà la
	 * pròxima etiqueta HTML inclosa al map.
	 */
	//static int tag_map_counter;
	
	/**
	 * Llistat d'etiquetes que, per la poca informació que aporten, són despreciades. Les etiquetes
	 * contingudes en aquesta llista no seran utilitzaedes per a la comparació entre fitxers web.
	 */
	//static vector< string > irrelevant_tags;
	
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
	static float text_distance_percent_differenciator;
	
	/**
	 * Percentatge de diferència de tamany màxim que es permet entre dos fitxers, tal que si s'excedeix aquest
	 * percentatge, es consideraran fitxers diferents.
	 */
	static float file_size_difference_percent;
	
	/**
	 * Ruta del fitxer de configuració de la llibreria TagAligner.
	 */
	static wstring tagaligner_config_file;
	
	/**
	 * Ruta del fitxer de configuració de la llibreria TextCat.
	 */
	static wstring textcat_config_file;
	
	/**
	 * Mode d'aliniament en TagAligner. Pot tenir tres valors numèrics (1, 2 o 3) corresponents
	 * respectivament, als modes d'aliniament:
	 * 1-Aliniament en dos pasos amb comparació de distànciade d'aliniament text.
	 * 2-Aliniament en dos pasos amb comparació de longitud de text.
	 * 3-Aliniament directe de text i etiquetes en dos pasos.
	 */
	static int tagaligner_mode;
	
	/**
	 * Temps màxim de descàrrega.
	 */
	static int downloaded_size;

	/**
	 * Paràmetre que registra les extenssions dels fitxers objectius de la descàrrega.
	 */
	static vector<wstring> accepted_extenssions;
	
	/**
	 * Paràmetre que indica la ruta en què s'ha de guardar les pàgines descarregades amb el mòdul de descàrrega.
	 */
	static wstring download_path;
	
	/**
	 * Indicador que permet forçar a bitextor a consultar l'idioma de cada fitxer o permetre que ell mateix intente esbrinar-lo.
	 */
	static bool guess_language;

public:
	/**
	 * Mètode que permet establir la màxima distància d'eidició entre dos fiters web per a considerar
	 * que són el mateix.
	 * @param value Llindar que es preten establir com a màxima distància d'edició.
	 * @throw char* El mètode llança una excepció si el valor a assignar no és major que 0.
	 */
	static void SetMaxEditDistance(const float &value);
	
	/**
	 * Mètode que indica si el valor de màxima distància d'edició és percentual o absolut.
	 * @return Retorna <code>true</code> en cas que el valor de màxima distància d'edició siga percentual i <code>false</code> en cas que siga absolut.
	 */
	static bool IsPercentMaxEditDistance();
	
	/**
	 * Mètode que permet obtenir la màxima distància d'edició entre fitxers web establerta.
	 * @return Llindar que es establert com a màxima distància d'edició.
	 */
	static float GetMaxEditDistance();
	
	/**
	 * Mètode que permet obtenir el precentatge de màxima distància entre textos per a
	 * considerar-los iguals.
	 * @return Percentatge de distància màxima de textos.
	 */
	static float GetTextDistancePercentDifferenciator();
	
	/**
	 * Mètode que permet buscar l'enter que es correpon amb un etiqueta HMTL en el mapa d'etiquetes HMTL
	 * "tag_map". Si l'etiqueta no es troba, s'introdueix al map fent servir el comptador "tag_map_counter".
	 * @param tag Etiqueta de què es desitja saber el seu identificador enter.
	 * @return Retorna l'identificador enter de l'etiqueta passada per paràmetre. Si l'identificador és
	 * major que 0, significa que aquesta etiqueta no és rellevant.
	 */
	static int GetHTMLTagValue(const wstring tag);
	
	/**
	 * Mètode que afegix una nova etiqueta a la llista d'etiquetes irrelevants.
	 * @param tag Etiqueta que es dessitja afegir a la llista.
	 */
	static void AddIrrelevantTag(const wstring &tag);
	
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
	static void SetTextDistancePercentDifferenciator(const float &value);
	
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
	static bool LoadGlobalParams(const wstring &path);
	
	/**
	 * Mètode que permet obtenir el percentatge màxim de diferència de mida de dos fitxers per a ser
	 * considerats el mateix fitxer.
	 * @return Retorna el valor del percentatge màxim de diferència de mida de dos fitxers per a ser
	 * considerats el mateix fitxer.
	 */
	static float GetFileSizeDifferencePercent();
	
	/**
	 * Mètode que permet establir el valor de la variable ja descrita <code>file_size_difference_percent</code>.
	 * @param value Valor que es vol establir.
	 */
	static void SetFileSizeDifferencePercent(const float &value);
	
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
	 * Mètode que permet establir la ruta on es troba el fitxer de configuració de la llibreria Tagaligner.
	 * @param path Ruta on es troba el fitxer.
	 */
	static void SetTagAlignerConfigFile(const wstring &path);
	
	/**
	 * Mètode que permet obtenir la ruta on es troba el fitxer de configuració de la llibreria Tagaligner.
	 * @return Retorna la ruta on es troba el fitxer.
	 */
	static wstring GetTagAlignerConfigFile();
	
	/**
	 * Mètode que permet establir el mode d'aliniament de TagAligner.
	 * @param mode Mode d'aliniament de TagAligner. Valors possibles:
	 * 1-Aliniament en dos pasos amb comparació de distànciade d'aliniament text.
	 * 2-Aliniament en dos pasos amb comparació de longitud de text.
	 * 3-Aliniament directe de text i etiquetes en dos pasos.
	 * @throw char* El mètode llança una excepció si el mode especificat per a TagAligner no és valid.
	 */
	static void SetTagAlignerMode(const int &mode);
	
	/**
	 * Mètode que permet obtenir el mode d'aliniament de TagAligner.
	 * @return Retorna el mode d'aliniament de TagAligner.
	 */
	static int GetTagAlignerMode();
	
	/**
	 * Mètode que processa recursivament els nodes del fitxer XML de configuració.
	 * @param node Node que es processarà.
	 * @param indention
	 */
	static void ProcessNode(xmlNode* node, wstring tagname);
	
	/**
	 * Mètode que permet obtenir la llista d'extensions acceptades per als fitxer a descarregar pel mòdul de descàrrega.
	 * @return Retorna la llista d'extensions acceptades per als fitxer a descarregar pel mòdul de descàrrega.
	 */
	static vector<wstring> GetAcceptedExtenssions();

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
	 * @param Valor que es preten donar a aquest paràmetre.
	 */
	static void SetGuessLanguage(const bool &value);
	
	/**
	 * Mètode que permet saber si bitextor ha d'esbrinar l'idioma dels fitxers o ha d'intentar consultar-lo a l'usuari.
	 * @return Retorna true en cas que bitextor siga qui esbrina l'idioma i false si l'ha de consultar a l'usuari.
	 */
	static bool GetGuessLanguage();
};

#endif /*GLOBALPARAMS_H_*/
