#include <iostream>

#include <mysql.h>
#include "Connect.h"
//#include "ps5000aApi.h"
#include "SDKUtils.h"
using namespace std;
int qstate;
    
int main()

{
  ////  Connect(char* host, char* rootName, char* password, char* dbname, int port);
  // Connect con((char*)"127.0.0.1", (char*)"root", (char*)"d", (char*)"testDBCplusplus", 3306);
  // // Connect con((char*)"127.0.0.1");
  //  MYSQL* conn;
  //  MYSQL_ROW row;
  //  MYSQL_RES* res;
  //  conn = mysql_init(0);
  // 
  //  conn = mysql_real_connect(conn, con.getHost(), "root", "", "testDBCplusplus", 3306, NULL, 0);

  //  if (conn) {
  //      cout << "Connexion réussi" << endl;
  //      const char* query = "SELECT * FROM test";
  //      //const char* q = query.c_str;
  //      qstate = mysql_query(conn, query);
  //      if (!qstate)
  //      {
  //          cout << "Connection reussi";
  //          res = mysql_store_result(conn);

  //          while ((row = mysql_fetch_row(res)) != NULL)
  //          {
  //              cout << row[0] << row[1];

  //          }
  //      }

  //  }

    PICO_STATUS status = PICO_OK;
    int16_t handle = 0;
    printf(" PicoScope 5000 Series (ps5000a) Driver MSO Block Capture Example Program");

    // Établir une connexion avec l'appareil
    // ------------------------------

    // Ouvre la connexion à l'appareil
   //status = ps5000aOpenUnit(&handle, NULL, PS5000A_DR_8BIT);
   //cout <<" STATUTS  "<< status<<endl;
   //cout << " PICO_USB3_0_DEVICE_NON_USB3_0_PORT " << PICO_USB3_0_DEVICE_NON_USB3_0_PORT<<endl;
   //cout << " PICO_POWER_SUPPLY_NOT_CONNECTED " << PICO_POWER_SUPPLY_NOT_CONNECTED;


   //if (status != PICO_OK)
   //{
   //    if (status == PICO_POWER_SUPPLY_NOT_CONNECTED || PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
   //    {
   //        status = ps5000aChangePowerSource(handle, status);
   //    }
   //    else
   //    {
   //        fprintf(stderr, "No device found - status code %d\n", status);
   //        return -1;
   //    }


   //    int16_t requiredSize = 0;
   //    int8_t	line[80];
   //    int32_t channelCount = 0;
   //    int32_t digitalPortCount = 0;
   //    int32_t i = 0;

   //    int8_t description[11][25] = { "Driver Version",
   //                                    "USB Version",
   //                                    "Hardware Version",
   //                                    "Variant Info",
   //                                    "Serial",
   //                                    "Cal Date",
   //                                    "Kernel Version",
   //                                    "Digital HW Version",
   //                                    "Analogue HW Version",
   //                                    "Firmware 1",
   //                                    "Firmware 2" };

   //    printf("Device information:-\n\n");

   //    for (i = 0; i < 11; i++)
   //    {
   //        status = ps5000aGetUnitInfo(handle, line, 80, &requiredSize, i);

   //        // info = 3 - PICO_VARIANT_INFO
   //        if (i == PICO_VARIANT_INFO)
   //        {
   //            channelCount = (int16_t)line[1];
   //            channelCount = channelCount - 48; // Subtract ASCII 0 (48)

   //            // Determine if the device is an MSO
   //         /*   if (strstr(line, "MSO") != NULL)
   //            {
   //                digitalPortCount = 2;
   //            }
   //            else
   //            {
   //                digitalPortCount = 0;
   //                fprintf(stderr, "This example is for PicoScope 5000 Series Mixed Signal Oscilloscopes.\n");
   //                printf("Exiting application...\n");
   //                Sleep(5000);
   //                return -1;
   //            }*/

   //        }

   //        printf("%s: %s\n", description[i], line);
   //    }
   //}
   // return 0;
    SDKUtils sdk;


    int8_t ch;
    uint16_t devCount = 0, listIter = 0, openIter = 0;
    //device indexer -  64 chars - 64 is maximum number of picoscope devices handled by driver
    int8_t devChars[] =
        "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#";
  
    UNIT allUnits[MAX_PICO_DEVICES];

    printf("PicoScope 5000 Series (ps5000a) Driver Example Program\n");
    printf("\nEnumerating Units...\n");

    do
    {
        status = sdk.openDevice(&(allUnits[devCount]), NULL);

        if (status == PICO_OK || status == PICO_POWER_SUPPLY_NOT_CONNECTED
            || status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
        {
            allUnits[devCount++].openStatus = (int16_t)status;
        }

    } while (status != PICO_NOT_FOUND);

    if (devCount == 0)
    {
        printf("Picoscope devices not found\n");
        return 1;
    }

    // if there is only one device, open and handle it here
    if (devCount == 1)
    {
        printf("Found one device, opening...\n\n");
        status = allUnits[0].openStatus;

        if (status == PICO_OK || status == PICO_POWER_SUPPLY_NOT_CONNECTED
            || status == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
        {
            if (allUnits[0].openStatus == PICO_POWER_SUPPLY_NOT_CONNECTED || allUnits[0].openStatus == PICO_USB3_0_DEVICE_NON_USB3_0_PORT)
            {
                allUnits[0].openStatus = (int16_t)sdk.changePowerSource(allUnits[0].handle, allUnits[0].openStatus, &allUnits[0]);
            }

            sdk.set_info(&allUnits[0]);
            status = sdk.handleDevice(&allUnits[0]);
        }

        if (status != PICO_OK)
        {
            printf("Picoscope devices open failed, error code 0x%x\n", (uint32_t)status);
            return 1;
        }

        sdk.collectStreamingImmediate(&allUnits[0]);

        printf("Exit...\n");
        return 0;
    }

}


