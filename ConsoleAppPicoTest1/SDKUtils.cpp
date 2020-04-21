#include "SDKUtils.h"


uint32_t	timebase = 8;
BOOL			scaleVoltages = TRUE;
int16_t			g_overflow = 0;

int16_t			g_autoStopped;
int16_t   	g_ready = FALSE;
uint64_t 		g_times[PS5000A_MAX_CHANNELS];
int16_t     g_timeUnit;
int32_t     g_sampleCount;
uint32_t		g_startIndex;
int16_t			g_trig = 0;
uint32_t		g_trigAt = 0;
int32_t cycles = 0;
uint16_t inputRanges[PS5000A_MAX_RANGES] = {
												10,
												20,
												50,
												100,
												200,
												500,
												1000,
												2000,
												5000,
												10000,
												20000,
												50000 };
void PREF4 callBackStreaming(int16_t handle,
	int32_t noOfSamples,
	uint32_t startIndex,
	int16_t overflow,
	uint32_t triggerAt,
	int16_t triggered,
	int16_t autoStop,
	void* pParameter)
{
	int32_t channel;
	BUFFER_INFO* bufferInfo = NULL;

	if (pParameter != NULL)
	{
		bufferInfo = (BUFFER_INFO*)pParameter;
	}

	// used for streaming
	g_sampleCount = noOfSamples;
	g_startIndex = startIndex;
	g_autoStopped = autoStop;

	// flag to say done reading data
	g_ready = TRUE;

	// flags to show if & where a trigger has occurred
	g_trig = triggered;
	g_trigAt = triggerAt;

	g_overflow = overflow;
	if (bufferInfo != NULL && noOfSamples)
	{
		for (channel = 0; channel < bufferInfo->unit->channelCount; channel++)
		{
			if (bufferInfo->unit->channelSettings[channel].enabled)
			{
				if (bufferInfo->appBuffers && bufferInfo->driverBuffers)
				{
					// Max buffers
					if (bufferInfo->appBuffers[channel * 2] && bufferInfo->driverBuffers[channel * 2])
					{
						memcpy_s(&bufferInfo->appBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t),
							&bufferInfo->driverBuffers[channel * 2][startIndex], noOfSamples * sizeof(int16_t));
					}

					// Min buffers
					if (bufferInfo->appBuffers[channel * 2 + 1] && bufferInfo->driverBuffers[channel * 2 + 1])
					{
						memcpy_s(&bufferInfo->appBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t),
							&bufferInfo->driverBuffers[channel * 2 + 1][startIndex], noOfSamples * sizeof(int16_t));
					}
				}
			}
		}
	}
}
int32_t SDKUtils::adc_to_mv(int32_t raw, int32_t rangeIndex, UNIT* unit)
{
	return (raw * inputRanges[rangeIndex]) / unit->maxADCValue;
}

PICO_STATUS SDKUtils::changePowerSource(int16_t handle, PICO_STATUS status, UNIT* unit)
{
	int8_t ch;

	switch (status)
	{
	case PICO_POWER_SUPPLY_NOT_CONNECTED:		// User must acknowledge they want to power via USB
		do
		{
			printf("\n5 V power supply not connected.");
			printf("\nDo you want to run using USB only Y/N?\n");
			ch = toupper(_getch());

			if (ch == 'Y')
			{
				printf("\nPowering the unit via USB\n");
				status = ps5000aChangePowerSource(handle, PICO_POWER_SUPPLY_NOT_CONNECTED);		// Tell the driver that's ok

				if (status == PICO_OK && unit->channelCount == QUAD_SCOPE)
				{
					unit->channelSettings[PS5000A_CHANNEL_C].enabled = FALSE;
					unit->channelSettings[PS5000A_CHANNEL_D].enabled = FALSE;
				}
				else if (status == PICO_POWER_SUPPLY_UNDERVOLTAGE)
				{
					status = changePowerSource(handle, status, unit);
				}
				else
				{
					// Do nothing
				}

			}
		} while (ch != 'Y' && ch != 'N');
		printf(ch == 'N' ? "Please use the +5V power supply to power this unit\n" : "");
		break;

	case PICO_POWER_SUPPLY_CONNECTED:
		printf("\nUsing +5 V power supply voltage.\n");
		status = ps5000aChangePowerSource(handle, PICO_POWER_SUPPLY_CONNECTED);		// Tell the driver we are powered from +5V supply
		break;

	case PICO_USB3_0_DEVICE_NON_USB3_0_PORT:
		do
		{
			printf("\nUSB 3.0 device on non-USB 3.0 port.");
			printf("\nDo you wish to continue Y/N?\n");
			ch = toupper(_getch());

			if (ch == 'Y')
			{
				printf("\nSwitching to use USB power from non-USB 3.0 port.\n");
				status = ps5000aChangePowerSource(handle, PICO_USB3_0_DEVICE_NON_USB3_0_PORT);		// Tell the driver that's ok

				if (status == PICO_POWER_SUPPLY_UNDERVOLTAGE)
				{
					status = changePowerSource(handle, status, unit);
				}
				else
				{
					// Do nothing
				}

			}
		} while (ch != 'Y' && ch != 'N');
		printf(ch == 'N' ? "Please use a USB 3.0 port or press 'Y'.\n" : "");
		break;

	case PICO_POWER_SUPPLY_UNDERVOLTAGE:
		do
		{
			printf("\nUSB not supplying required voltage");
			printf("\nPlease plug in the +5 V power supply\n");
			printf("\nHit any key to continue, or Esc to exit...\n");
			ch = _getch();

			if (ch == 0x1B)	// ESC key
			{
				exit(0);
			}
			else
			{
				status = ps5000aChangePowerSource(handle, PICO_POWER_SUPPLY_CONNECTED);		// Tell the driver that's ok
			}
		} while (status == PICO_POWER_SUPPLY_REQUEST_INVALID);
		break;
	}

	printf("\n");
	return status;
}

/****************************************************************************
* ClearDataBuffers
*
* stops GetData writing values to memory that has been released
****************************************************************************/
PICO_STATUS SDKUtils::clearDataBuffers(UNIT* unit)
{
	int32_t i;
	PICO_STATUS status;

	for (i = 0; i < unit->channelCount; i++)
	{
		if (unit->channelSettings[i].enabled)
		{
			if ((status = ps5000aSetDataBuffers(unit->handle, (PS5000A_CHANNEL)i, NULL, NULL, 0, 0, PS5000A_RATIO_MODE_NONE)) != PICO_OK)
			{
				printf("clearDataBuffers:ps5000aSetDataBuffers(channel %d) ------ 0x%08lx \n", i, status);
			}
		}
	}
	return status;
}




void SDKUtils::streamDataHandler(UNIT* unit, uint32_t preTrigger)

{
	 int8_t streamFile[20] = "stream.txt";
	int32_t i, j;
	uint32_t sampleCount = 50000; /* make sure overview buffer is large enough */
	FILE* fp = NULL;
	int16_t* buffers[2 * PS5000A_MAX_CHANNELS];
	int16_t* appBuffers[2 * PS5000A_MAX_CHANNELS];
	PICO_STATUS status;
	PICO_STATUS powerStatus;
	uint32_t sampleInterval;
	int32_t index = 0;
	int32_t totalSamples;
	uint32_t postTrigger;
	int16_t autostop;
	uint32_t downsampleRatio;
	uint32_t triggeredAt = 0;
	PS5000A_TIME_UNITS timeUnits;
	PS5000A_RATIO_MODE ratioMode;
	int16_t retry = 0;
	int16_t powerChange = 0;
	uint32_t numStreamingValues = 0;

	BUFFER_INFO bufferInfo;

	powerStatus = ps5000aCurrentPowerSource(unit->handle);

	for (i = 0; i < unit->channelCount; i++)
	{
		if (i >= DUAL_SCOPE && unit->channelCount == QUAD_SCOPE && powerStatus == PICO_POWER_SUPPLY_NOT_CONNECTED)
		{
			// No need to set the channels C and D if Quad channel scope and power supply not connected.
		}
		else
		{
			if (unit->channelSettings[i].enabled)
			{
				buffers[i * 2] = (int16_t*)calloc(sampleCount, sizeof(int16_t));
				buffers[i * 2 + 1] = (int16_t*)calloc(sampleCount, sizeof(int16_t));

				status = ps5000aSetDataBuffers(unit->handle, (PS5000A_CHANNEL)i, buffers[i * 2], buffers[i * 2 + 1], sampleCount, 0, PS5000A_RATIO_MODE_NONE);

				appBuffers[i * 2] = (int16_t*)calloc(sampleCount, sizeof(int16_t));
				appBuffers[i * 2 + 1] = (int16_t*)calloc(sampleCount, sizeof(int16_t));

				printf(status ? "StreamDataHandler:ps5000aSetDataBuffers(channel %ld) ------ 0x%08lx \n" : "", i, status);
			}
		}
	}

	downsampleRatio = 1;
	timeUnits = PS5000A_US;
	sampleInterval = 1;
	ratioMode = PS5000A_RATIO_MODE_NONE;
	preTrigger = 0;
	postTrigger = 1000000;
	autostop = TRUE;

	bufferInfo.unit = unit;
	bufferInfo.driverBuffers = buffers;
	bufferInfo.appBuffers = appBuffers;

	if (autostop)
	{
		printf("\nStreaming Data for %lu samples", postTrigger / downsampleRatio);

		if (preTrigger)	// We pass 0 for preTrigger if we're not setting up a trigger
		{
			printf(" after the trigger occurs\nNote: %lu Pre Trigger samples before Trigger arms\n\n", preTrigger / downsampleRatio);
		}
		else
		{
			printf("\n\n");
		}
	}
	else
	{
		printf("\nStreaming Data continually.\n\n");
	}

	g_autoStopped = FALSE;


	do
	{
		retry = 0;

		status = ps5000aRunStreaming(unit->handle, &sampleInterval, timeUnits, preTrigger, postTrigger, autostop,
			downsampleRatio, ratioMode, sampleCount);

		if (status != PICO_OK)
		{
			// PicoScope 5X4XA/B/D devices...+5 V PSU connected or removed or
			// PicoScope 524XD devices on non-USB 3.0 port
			if (status == PICO_POWER_SUPPLY_CONNECTED || status == PICO_POWER_SUPPLY_NOT_CONNECTED ||
				status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT || status == PICO_POWER_SUPPLY_UNDERVOLTAGE)
			{
				status = changePowerSource(unit->handle, status, unit);
				retry = 1;
			}
			else
			{
				printf("streamDataHandler:ps5000aRunStreaming ------ 0x%08lx \n", status);
				return;
			}
		}
	} while (retry);

	printf("Streaming data...Press a key to stop\n");


	fopen_s(&fp, (const char*)streamFile, "w");

	if (fp != NULL)
	{
		fprintf(fp, "Streaming Data Log\n\n");
		fprintf(fp, "For each of the %d Channels, results shown are....\n", unit->channelCount);
		fprintf(fp, "Maximum Aggregated value ADC Count & mV, Minimum Aggregated value ADC Count & mV\n\n");

		for (i = 0; i < unit->channelCount; i++)
		{
			if (unit->channelSettings[i].enabled)
			{
				fprintf(fp, "   Max ADC    Max mV  Min ADC  Min mV   ");
			}
		}
		fprintf(fp, "\n");
	}


	totalSamples = 0;

	while (!_kbhit() && !g_autoStopped)
	{
		/* Poll until data is received. Until then, GetStreamingLatestValues wont call the callback */
		g_ready = FALSE;

		status = ps5000aGetStreamingLatestValues(unit->handle, callBackStreaming, &bufferInfo);

		// PicoScope 5X4XA/B/D devices...+5 V PSU connected or removed or
		// PicoScope 524XD devices on non-USB 3.0 port
		if (status == PICO_POWER_SUPPLY_CONNECTED || status == PICO_POWER_SUPPLY_NOT_CONNECTED ||
			status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT || status == PICO_POWER_SUPPLY_UNDERVOLTAGE)
		{
			if (status == PICO_POWER_SUPPLY_UNDERVOLTAGE)
			{
				changePowerSource(unit->handle, status, unit);
			}

			printf("\n\nPower Source Change");
			powerChange = 1;
		}

		index++;

		if (g_ready && g_sampleCount > 0) /* Can be ready and have no data, if autoStop has fired */
		{
			if (g_trig)
			{
				triggeredAt = totalSamples + g_trigAt;		// Calculate where the trigger occurred in the total samples collected
			}

			totalSamples += g_sampleCount;
			printf("\nCollected %3li samples, index = %5lu, Total: %6d samples ", g_sampleCount, g_startIndex, totalSamples);

			if (g_trig)
			{
				printf("Trig. at index %lu total %lu", g_trigAt, triggeredAt + 1);	// show where trigger occurred

			}

			for (i = g_startIndex; i < (int32_t)(g_startIndex + g_sampleCount); i++)
			{

				if (fp != NULL)
				{
					for (j = 0; j < unit->channelCount; j++)
					{
						if (unit->channelSettings[j].enabled)
						{
							fprintf(fp,
								"Ch%C  %5d = %+5dmV, %5d = %+5dmV   ",
								(char)('A' + j),
								appBuffers[j * 2][i],
								adc_to_mv(appBuffers[j * 2][i], unit->channelSettings[PS5000A_CHANNEL_A + j].range, unit),
								appBuffers[j * 2 + 1][i],
								adc_to_mv(appBuffers[j * 2 + 1][i], unit->channelSettings[PS5000A_CHANNEL_A + j].range, unit));
						}
					}

					fprintf(fp, "\n");
				}
				else
				{
					printf("Cannot open the file %s for writing.\n", streamFile);
				}

			}
		}
	}

	printf("\n\n");

	ps5000aStop(unit->handle);

	if (fp != NULL)
	{

		fclose(fp);
	}

	if (!g_autoStopped && !powerChange)
	{
		printf("\nData collection aborted\n");
		_getch();
	}
	else
	{
		printf("\nData collection complete.\n\n");
	}

	for (i = 0; i < unit->channelCount; i++)
	{
		if (unit->channelSettings[i].enabled)
		{
			free(buffers[i * 2]);
			free(appBuffers[i * 2]);

			free(buffers[i * 2 + 1]);
			free(appBuffers[i * 2 + 1]);
		}
	}

	clearDataBuffers(unit);
}




void SDKUtils::setDefaults(UNIT* unit)
{
	PICO_STATUS status;
	PICO_STATUS powerStatus;
	int32_t i;

	status = ps5000aSetEts(unit->handle, PS5000A_ETS_OFF, 0, 0, NULL);					// Turn off hasHardwareETS
	printf(status ? "setDefaults:ps5000aSetEts------ 0x%08lx \n" : "", status);

	powerStatus = ps5000aCurrentPowerSource(unit->handle);

	for (i = 0; i < unit->channelCount; i++) // reset channels to most recent settings
	{
		if (i >= DUAL_SCOPE && powerStatus == PICO_POWER_SUPPLY_NOT_CONNECTED)
		{
			// No need to set the channels C and D if Quad channel scope and power not enabled.
		}
		else
		{
			status = ps5000aSetChannel(unit->handle, (PS5000A_CHANNEL)(PS5000A_CHANNEL_A + i),
				unit->channelSettings[PS5000A_CHANNEL_A + i].enabled,
				(PS5000A_COUPLING)unit->channelSettings[PS5000A_CHANNEL_A + i].DCcoupled,
				(PS5000A_RANGE)unit->channelSettings[PS5000A_CHANNEL_A + i].range,
				unit->channelSettings[PS5000A_CHANNEL_A + i].analogueOffset);

			printf(status ? "SetDefaults:ps5000aSetChannel------ 0x%08lx \n" : "", status);

		}
	}
}


void SDKUtils::collectStreamingImmediate(UNIT* unit)
{
	PICO_STATUS status = PICO_OK;

	this->setDefaults(unit);

	printf("Collect streaming...\n");
	printf("Data is written to disk file (stream.txt)\n");
	printf("Press a key to start\n");
	_getch();

	/* Trigger disabled	*/
	status = ps5000aSetSimpleTrigger(unit->handle, 0, PS5000A_CHANNEL_A, 0, PS5000A_RISING, 0, 0);

	this->streamDataHandler(unit, 0);
}



PICO_STATUS SDKUtils::openDevice(UNIT* unit, int8_t* serial)
{
	PICO_STATUS status;
	unit->resolution = PS5000A_DR_8BIT;

	if (serial == NULL)
	{
		status = ps5000aOpenUnit(&unit->handle, NULL, unit->resolution);
	}
	else
	{
		status = ps5000aOpenUnit(&unit->handle, serial, unit->resolution);
	}

	unit->openStatus = (int16_t)status;
	unit->complete = 1;

	return status;
}


/****************************************************************************
* Initialise unit' structure with Variant specific defaults
****************************************************************************/
void SDKUtils::set_info(UNIT* unit)
{
	int8_t description[11][25] = { "Driver Version",
		"USB Version",
		"Hardware Version",
		"Variant Info",
		"Serial",
		"Cal Date",
		"Kernel Version",
		"Digital HW Version",
		"Analogue HW Version",
		"Firmware 1",
		"Firmware 2" };

	int16_t i = 0;
	int16_t requiredSize = 0;
	int8_t line[80];
	int32_t variant;
	PICO_STATUS status = PICO_OK;

	// Variables used for arbitrary waveform parameters
	int16_t			minArbitraryWaveformValue = 0;
	int16_t			maxArbitraryWaveformValue = 0;
	uint32_t		minArbitraryWaveformSize = 0;
	uint32_t		maxArbitraryWaveformSize = 0;

	//Initialise default unit properties and change when required
	unit->sigGen = SIGGEN_FUNCTGEN;
	unit->firstRange = PS5000A_10MV;
	unit->lastRange = PS5000A_20V;
	unit->channelCount = DUAL_SCOPE;
	unit->awgBufferSize = MIN_SIG_GEN_BUFFER_SIZE;
	unit->digitalPortCount = 0;

	if (unit->handle)
	{
		printf("Device information:-\n\n");

		for (i = 0; i < 11; i++)
		{
			status = ps5000aGetUnitInfo(unit->handle, line, sizeof(line), &requiredSize, i);

			// info = 3 - PICO_VARIANT_INFO
			if (i == PICO_VARIANT_INFO)
			{
				variant = atoi((char*)line);
				memcpy(&(unit->modelString), line, sizeof(unit->modelString) == 5 ? 5 : sizeof(unit->modelString));

				unit->channelCount = (int16_t)line[1];
				unit->channelCount = unit->channelCount - 48; // Subtract ASCII 0 (48)

				// Determine if the device is an MSO
				if (strstr((char*)line, "MSO") != NULL)
				{
					unit->digitalPortCount = 2;
				}
				else
				{
					unit->digitalPortCount = 0;
				}

			}
			else if (i == PICO_BATCH_AND_SERIAL)	// info = 4 - PICO_BATCH_AND_SERIAL
			{
				memcpy(&(unit->serial), line, requiredSize);
			}

			printf("%s: %s\n", description[i], line);
		}

		printf("\n");

		// Set sig gen parameters
		// If device has Arbitrary Waveform Generator, find the maximum AWG buffer size
		status = ps5000aSigGenArbitraryMinMaxValues(unit->handle, &minArbitraryWaveformValue, &maxArbitraryWaveformValue, &minArbitraryWaveformSize, &maxArbitraryWaveformSize);
		unit->awgBufferSize = maxArbitraryWaveformSize;

		if (unit->awgBufferSize > 0)
		{
			unit->sigGen = SIGGEN_AWG;
		}
		else
		{
			unit->sigGen = SIGGEN_FUNCTGEN;
		}
	}
}


PICO_STATUS SDKUtils::handleDevice(UNIT* unit)
{
	int16_t value = 0;
	int32_t i;
	struct tPwq pulseWidth;
	PICO_STATUS status;

	printf("Handle: %d\n", unit->handle);

	if (unit->openStatus != PICO_OK)
	{
		printf("Unable to open device\n");
		printf("Error code : 0x%08x\n", (uint32_t)unit->openStatus);
		while (!_kbhit());
		exit(99); // exit program
	}

	printf("Device opened successfully, cycle %d\n\n", ++cycles);

	// Setup device info - unless it's set already
	if (unit->model == MODEL_NONE)
	{
		this->set_info(unit);
	}

	// Turn off any digital ports (MSO models only)
	if (unit->digitalPortCount > 0)
	{
		printf("Turning off digital ports.");

		for (i = 0; i < unit->digitalPortCount; i++)
		{
			status = ps5000aSetDigitalPort(unit->handle, (PS5000A_CHANNEL)(i + PS5000A_DIGITAL_PORT0), 0, 0);
		}
	}

	timebase = 1;

	ps5000aMaximumValue(unit->handle, &value);
	unit->maxADCValue = value;

	status = ps5000aCurrentPowerSource(unit->handle);

	for (i = 0; i < unit->channelCount; i++)
	{
		// Do not enable channels C and D if power supply not connected for PicoScope 544XA/B devices
		if (unit->channelCount == QUAD_SCOPE && status == PICO_POWER_SUPPLY_NOT_CONNECTED && i >= DUAL_SCOPE)
		{
			unit->channelSettings[i].enabled = FALSE;
		}
		else
		{
			unit->channelSettings[i].enabled = TRUE;
		}

		unit->channelSettings[i].DCcoupled = TRUE;
		unit->channelSettings[i].range = PS5000A_5V;
		unit->channelSettings[i].analogueOffset = 0.0f;
	}

	memset(&pulseWidth, 0, sizeof(struct tPwq));

	this->setDefaults(unit);

	/* Trigger disabled	*/
	status = ps5000aSetSimpleTrigger(unit->handle, 0, PS5000A_CHANNEL_A, 0, PS5000A_RISING, 0, 0);

	return unit->openStatus;
}

void SDKUtils::closeDevice(UNIT* unit)
{
	ps5000aCloseUnit(unit->handle);
}




