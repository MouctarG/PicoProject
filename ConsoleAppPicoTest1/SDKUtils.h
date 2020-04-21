#include "HeaderIncludeLib.h"
class SDKUtils
{
private:
	//int8_t streamFile[20] = "stream.txt";

public:
	/****************************************************************************************
* ChangePowerSource - fonction pour g�rer les commutateurs entre l'alimentation
+ 5 V et l'alimentation USB uniquement
* S'applique uniquement aux unit�s PicoScope 544xA / B
******************************************************************************************/
	PICO_STATUS changePowerSource(int16_t handle, PICO_STATUS status, UNIT* unit);


	/****************************************************************************
* adc_to_mv : Convertir un d�compte ADC 16 bits en millivolts
****************************************************************************/
	int32_t adc_to_mv(int32_t raw, int32_t rangeIndex, UNIT* unit);

	/****************************************************************************
* ClearDataBuffers - emp�che GetData d'�crire des valeurs dans la m�moire qui a �t� lib�r�e
****************************************************************************/
	PICO_STATUS clearDataBuffers(UNIT* unit);

	/****************************************************************************
* streamDataHandler
* - Utilis� par les deux exemples de donn�es de flux - non d�clench� et d�clench�
* Param�tre:
* - unit - l'unit� d'�chantillonnage
* - preTrigger -  le nombre d'�chantillons dans la phase de pr�-d�clenchement
* (0 si aucun d�clencheur n'a �t� d�fini)
***************************************************************************/
	void streamDataHandler(UNIT* unit, uint32_t preTrigger);


	/****************************************************************************
* callbackStreaming
*Utilis� par les appels de collecte de streaming de donn�es ps5000a, � la r�ception des donn�es.
* Utilis� pour d�finir les drapeaux globaux, etc.
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
*  Cette fonction montre comment collecter un flux de donn�es
* de l'unit� (commencez � collecter imm�diatement)
***************************************************************************/
	void collectStreamingImmediate(UNIT* unit);

	/****************************************************************************
	* SetDefaults - restaurer les param�tres par d�faut
	****************************************************************************/
	void setDefaults(UNIT* unit);

	/****************************************************************************
	* openDevice
	* Parameters
	* - unit pointeur vers la structure UNIT, o� la poign�e sera stock�e
	* - serial pointeur vers le tableau int8_t contenant le num�ro de s�rie
	*
	* Returns
	* - PICO_STATUS pour indiquer le succ�s, ou si une erreur s'est produite
	***************************************************************************/
	PICO_STATUS openDevice(UNIT* unit, int8_t* serial);

	/****************************************************************************
	*	Initialiser la structure de l'unit� avec
	des valeurs par d�faut sp�cifiques aux variantes
****************************************************************************/
	void set_info(UNIT* unit);



	/****************************************************************************
* handleDevice
* Parameters
* - unit pointeur vers la structure UNIT, o� la poign�e sera stock�e
*
* Returns
* - PICO_STATUS - Pour indiquer le succ�s, ou si une erreur s'est produite***********************************************************/
	PICO_STATUS handleDevice(UNIT* unit);

	/****************************************************************************
	* Poour fermer l'appareil
	****************************************************************************/
	void closeDevice(UNIT* unit);
};
