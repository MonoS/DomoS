#ifndef DomoS_H

#define DomoS_H
#include <arduino.h>

class DomoS
{
private:

  static const byte STRINGMAXLEN = 64; //Maximum length for the command string
  static const byte SUBSTRINGMAXLEN = 16; //Maximum length for a single command
  static const byte MAXADDRESSPIN = 8; //Maximum number of adressing pin
  static const byte MAXNAMELEN = 10; //Maximum length for a peripheral name
  static const byte FILEVER = 0; //The version of the file type

  /*
   The DomoS setting file is made of two parts
   1) The header of the file which contains all of the configuration parameters
   of the DomoS module and the version of the file type
   2) The body of the file which contains all the settings for the different
   peripherals [for a explanation go to the declaration of the type]
   These are stored in sequential order
   
   With version 0 the different arduino EEPROM can contain up to:
   ATmega168 and ATmega8 [512byte]:       45 peripherals
   ATmega328 [1024byte]:                  91 peripherals
   ATmega1280 and ATmega2560 [4096byte]: 371 peripherals
   */
  struct DomoSFileHeader //size 13byte
  {
    //The order here is also the order in the EEPROM
    //RESPECT THIS ORDER
    //The "EEPROM #" on the right of the variable name is the standard position in the EEPROM

    //Version of the file type, in case of change through developement
    byte fileVer; //EEPROM 2
    //Copy of the configuration parameters
    byte numPeripheral; //EEPROM 3
    byte numAddressPin; //EEPROM 4
    byte addressPin[MAXADDRESSPIN]; //EEPROM 5-12
    byte outputPin; //EEPROM 13
    byte writeToEeprom; //EEPROM 14
  };

  struct DomoSFileBody //size 11byte
  {
    char name[MAXNAMELEN]; //The name of the peripheral
    byte number; //The number of the peripheral and the addressing parameter
  };

  //Setup function
  void GetConfigurationDataFromEeprom(); //Gets the configurations data from the EEPROM
  void FirstStart(); //Starts all the magic before the first start
  boolean CheckSetupData(); //Checks if the system has been already setupped
  void ClearEeprom(); //Clears all the EEPROM
  void WriteSetupData(); //Writes to the first two cells of EEPROM the setup check values
  void AskData(DomoSFileHeader & data); //Asks to the user the configuration parameters
  void WriteConfigurationDataToEeprom (DomoSFileHeader data); //Writes the data variables into the EEPROM
  void Initialize(); //Initializes the DomoS module
  void UpdateNumPeripheral(char type); //Updates the peripheral number

  boolean ConvertDecimalToBinary(int number, boolean result[]); //Converts a decimal number to an array of boolean, return false if the number is greater than what the module can handle, else true
  void SetAddressing(boolean addressing[]); //Sets up the addressing lines
  byte SeparateCommandBySpace(); //Separates the _command string into two strings, the first is the first word before the space, the second is the original string with the first word deleted, returns the number of char written in _subCommand
  byte CompareSubCommand(); //Compares a command with the commands' dictionary
  boolean FetchCommand(); //Fetches a command from the serial port
  void DoCommand(byte numCommand); //Executes a command
  byte GetCommand(char* command); //Gets the number of a command
  void CommandToLowerCase(); //Converts the _command string to lower case
  void SubCommandShiftLeft(); //Shifts left of one position all the character in the _subCommand string
  byte SearchPeripheralByName(char peripheral[]); //Searches the peripheral by name
  void GetPeripheralName(byte numPeripheral, char name[]); //Writes in char name[] the name of numPeripheral-th peripheral
  boolean SearchDuplicatedPeripheral(DomoSFileBody & peripheral, boolean nameCustom, boolean numberCustom); //Checks if the peripheral is unique else tries to make it unique
  void GetPeripheralNumber(byte numPeripheral, byte & number); //Writes in "number" the number of numPeripheral-th peripheral
  byte ConvertBinaryStringToDecimal();
  void BlankNewPeripheral(DomoSFileBody & peripheral);
  boolean CreateParameterCheck(DomoSFileBody & peripheral);
  boolean WritePeripheral(DomoSFileBody peripheral, byte position);
  byte SearchPeripheralByNumber(byte number);
  void ComposeStringPeripheral(DomoSFileBody peripheral, byte phrase);
  
  byte GetError();

  boolean WritePeripheralToEeprom(DomoSFileBody peripheral, byte position); //Writes the peripheral to the EEPROM
  //boolean WritePeripheralToSd(DomoSFileBody peripheral, byte position); //Function not yet developed
  
  void Create(); //Create a peripheral
  void Turn(); //Activate a peripheral
  void Delete(); //Delete a peripheral, probably this wont be developed
  void Exit(); //Turn off DomoS module
  void Reset(); //Resets the DomoS module
  void List(); //Give a list of all the installed peripheral

  int parseInt();

  void ThrownError();

  //Declaration of configuration variables
  byte _numAddressPin; //Number of pins used for addressing peripherals
  byte _addressPin[MAXADDRESSPIN]; //Pins used for addressing
  byte _outputPin; //The pin used for the output, pin 6 will automatically be selected
  byte _writeToEeprom; //-1 if DomoS must store the peripheral settings in the EEPROM, else the CSPin where the SD card is connected for storing the settings in DomoS.dat file
  byte _numPeripheral; //Number of peripheral created by user
  byte _fileVer; //The version of the file type

  boolean _on; //Tell if DOMOS system is on
  byte _lastError; //Tell the last error thrown by DomoS

  /*
   Declaration of strings constant
   Inizialization in DomoS.cpp
   */
  static const byte NCOMMAND = 8; //number of commands allowed
  static const char* COMMAND[NCOMMAND]; //Array of commands, for explanation go to inizialization

  static const int NPHRASE = 12; //Number of phrases, for eventually translation
  static const char* PHRASE[NPHRASE];

  static const int NERROR = 24;
  static const char* ERROR[NERROR];

  static const int START = 2;
  static const byte SETUP[START]; //These two values are stored in the first two cell of EEPROM, if already present the system have been already setup, if not launch the first start procedure
  
  static const int RCLOAD = 300; //Number of millisecond necessary for charging of RC circuit

  //String for fetching and checking commands
  char _command[STRINGMAXLEN];
  char _subCommand[SUBSTRINGMAXLEN];

public:
  //Constructor
  DomoS();

  void Work();
  boolean IsOn(); //Control if the module is ready to handle new request

  //Errors constant
  static const byte OK = 0;
  static const byte COMMANDSTRINGTOOLONG = 1;
  static const byte SUBCOMMANDSTRINGTOOLONG = 2;
  static const byte COMMANDNOTRECOGNIZED = 3;
  static const byte NOCOMMANDPARAMETERS = 4;
  static const byte SUBCOMMANDNOTRECOGNIZED = 5;
  static const byte NAMENOTDEFINED = 6;
  static const byte NAMETOOLONG = 7;
  static const byte ASNOTDEFINED = 8;
  static const byte BINARYNUMBERTOOLONG = 9;
  static const byte EEPROMISFULL = 10;
  static const byte PERIPHERALNOTFOUND = 11;
  static const byte STRANGETURNPARAMETER = 12;
  static const byte BADTHINGSHAPPEN = 13;
  static const byte PERIPHERALZERONOTALLOWED = 14;
  static const byte PERIPHERALMAXIMUMNUMBERREACH = 15;
  static const byte PERIPHERALNAMENOTUNIQUE = 16;
  static const byte PERIPHERALNUMBERNOTUNIQUE = 17;
  static const byte CANTFINDFREENAME = 18;
  static const byte CANTFINDFREENUMBER = 19;
  static const byte VOLTAGETOOLOW = 20;
  static const byte VOLTAGETOOHIGH = 21;
  static const byte DECIMALNUMBERTOOBIG = 22;
  static const byte THEREAREZEROPERIPHERAL = 23;
};
#endif

