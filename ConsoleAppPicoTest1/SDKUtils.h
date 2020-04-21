#include "HeaderIncludeLib.h"
class SDKUtils
{
private:
	//int8_t streamFile[20] = "stream.txt";

public:
	/****************************************************************************************
* ChangePowerSource - fonction pour gérer les commutateurs entre l'alimentation
+ 5 V et l'alimentation USB uniquement
* S'applique uniquement aux unités PicoScope 544xA / B
******************************************************************************************/
	PICO_STATUS changePowerSource(int16_t handle, PICO_STATUS status, UNIT* unit);


	/****************************************************************************
* adc_to_mv : Convertir un décompte ADC 16 bits en millivolts
****************************************************************************/
	int32_t adc_to_mv(int32_t raw, int32_t rangeIndex, UNIT* unit);

	/****************************************************************************
* ClearDataBuffers - empêche GetData d'écrire des valeurs dans la mémoire qui a été libérée
****************************************************************************/
	PICO_STATUS clearDataBuffers(UNIT* unit);

	/****************************************************************************
* streamDataHandler
* - Utilisé par les deux exemples de données de flux - non déclenché et déclenché
* Paramètre:
* - unit - l'unité d'échantillonnage
* - preTrigger -  le nombre d'échantillons dans la phase de pré-déclenchement
* (0 si aucun déclencheur n'a été défini)
***************************************************************************/
	void streamDataHandler(UNIT* unit, uint32_t preTrigger);


	/****************************************************************************
* callbackStreaming
*Utilisé par les appels de collecte de streaming de données ps5000a, à la réception des données.
* Utilisé pour définir les drapeaux globaux, etc.
****************************************************************************/
	//void PREF4 callBackStreaming(int16_t handle,
	//	int32_t noOfSamples,
	//	uint32_t startIndex,
	//	int16_t overflow,
	//	uint32_t triggerAt,
	//	int16_t triggered,
	//	int16_t autoStop,
	//	void* pParameter);

	/****************************************************************************
* collectStreamingImmediate
*  Cette fonction montre comment collecter un flux de données
* de l'unité (commencez à collecter immédiatement)
***************************************************************************/
	void collectStreamingImmediate(UNIT* unit);

	/****************************************************************************
	* SetDefaults - restaurer les paramètres par défaut
	****************************************************************************/
	void setDefaults(UNIT* unit);

	/****************************************************************************
	* openDevice
	* Parameters
	* - unit pointeur vers la structure UNIT, où la poignée sera stockée
	* - serial pointeur vers le tableau int8_t contenant le numéro de série
	*
	* Returns
	* - PICO_STATUS pour indiquer le succès, ou si une erreur s'est produite
	***************************************************************************/
	PICO_STATUS openDevice(UNIT* unit, int8_t* serial);

	/****************************************************************************
	*	Initialiser la structure de l'unité avec
	des valeurs par défaut spécifiques aux variantes
****************************************************************************/
	void set_info(UNIT* unit);



	/****************************************************************************
* handleDevice
* Parameters
* - unit pointeur vers la structure UNIT, où la poignée sera stockée
*
* Returns
* - PICO_STATUS - Pour indiquer le succès, ou si une erreur s'est produite***********************************************************/
	PICO_STATUS handleDevice(UNIT* unit);

	/****************************************************************************
	* Poour fermer l'appareil
	****************************************************************************/
	void closeDevice(UNIT* unit);
};
