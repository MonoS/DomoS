#include "DomoS.h"
#include <Serial.h>
#include <arduino.h>
#include <EEPROM.h>

const byte DomoS::SETUP[DomoS::START] = {
  168, 63};

const char* DomoS::COMMAND[DomoS::NCOMMAND] = {
  "create",  //Create a new peripheral
  //syntax: create [name test] [as 42/b00101010]

  "turn",    //Sent a signal to a peripheral
  //syntax: turn name high/low/%10/v2.3

  "delete",  //Delete a peripheral
  //syntax: delete name

  "exit",    //Turn off the DomoS Module
  //syntax: exit

  "name", 	//Define the name of the new peripheral
  //Max 9 character
  //Can be blank

  "as",	  	//Define the custom number/addressment of the new peripheral
  //Can be blank
  //Accept both decimal number and binary number

  "reset",      //Reset the DomoS module deletting the first two cells of EEPROM

  "list"        //Give a list of all the installed peripheral
};

const char* DomoS::PHRASE[DomoS::NPHRASE] = {
  "Write the number of address pins (max 8): ", //0
  "Write the address pin: ", //1
  "Do you want to store peripherals data into EEPROM? (-1 for yes or the CSpin): ", //2
  "Error! Reinsert the asked data: ", //3
  "Pin already used! Select another: ", //4
  "Digital pin 6 will automatically used for output, don't select them!", //5
  "Peripheral N whit number M (addressing B) created succesfully!", //6
  "Welcome to DomoS", //7
  "Resetting complete, now reset your arduino", //8
  "Buy buy from me and my creator ;)", //9
  "Peripheral N whit number M (addressing B)", //10
  "Peripheral deletted succesfully" //11
};

const char* DomoS::ERROR[DomoS::NERROR] = {
  "YESH, no error :)",
  "The command string you entered exeded 64 character,.",
  "One of your sub command is too long.",
  "Your main command wasn't recognized.",
  "There are no command in your string.",
  "One of the sub command wasn't recognized.",
  "The name of the peripheral wasn't defined.",
  "The name you entered is too long.",
  "The address wasn't defined.",
  "The binary number you entered is too long.",
  "The EEPROM is full.",
  "The peripheral you entered wasn't found.",
  "Turn parameters wasn't recognized.",
  "Random errors sometimes occurs.",
  "The peripheral with number zero is not allowed.",
  "The maximum number of allowed peripheral was reach.",
  "The name you entered is not unique.",
  "The number you entered is not unique.",
  "Can't find a free standard name.",
  "Can't find free address",
  "The voltage you entered is too low.",
  "he voltage you entered is too high.",
  "The number you entered is too big.",
  "There are no peripheral to be showed"
};

DomoS::DomoS()
/*	Standard costructor for the DomoS Module
 	At first, check if DomoS was already setup using the CheckSetupData
 	If is the first start call the FirstStart function then initialize the module whit Initialize,
 	else go directly to Initialize
 	
 	Debugged: Don't need to eb debugged
 */
{
  Serial.begin(9600);
  if (!CheckSetupData())
    FirstStart();

  Initialize();

  Serial.println(PHRASE[7]);
  return;
}

boolean DomoS::CheckSetupData()
/*	Check if the two setup value are present in the first two EEPROM cells
 
 	Debugged: OK
 */
{
  boolean ok;

  if ((EEPROM.read(0) == SETUP[0]) && (EEPROM.read(1) == SETUP[1]))
    ok = true;
  else
    ok = false;

  return ok;
}

void DomoS::Initialize()
/*	Initialize all the DomoS' components
 	At first read all the configuration parameters from the EEPROM using
 	GetConfigurationDataFromEeprom, then initialize all the DomoS variables at a known state
 	
 	Debugged: Don't need to be debugged
 */
{
  GetConfigurationDataFromEeprom();
  _lastError = OK; 		//Set the last error at no error (OK)
  _command[0] = '\0'; 	//Set the command string at empty string
  _subCommand[0] = '\0'; 	//Set the subcommand string at empty string
  _on = true;				//Set the on parameter at true

    for(byte i = 0; i<_numAddressPin; i++)
    pinMode(_addressPin[i], OUTPUT);

  pinMode(_outputPin, OUTPUT);

  /*if(_writeToEeprom != -1)
   pinMode(_writeToEeprom, OUTPUT);*/

  return;
}

void DomoS::GetConfigurationDataFromEeprom()
/*	Get all the configuration parameters from the internal arduino EEPROM
 	Starting from START address start reading all the parameters following a specific sequence
 	if someone modify this sequence go to modify also WriteConfigurationDataToEeprom()
 	
 	Debugged: OK
 */
{
  byte i; 

  i = START;

  //Start reading all data
  //After each reading increment i
  _fileVer = EEPROM.read(i); 
  i++;
  _numPeripheral = EEPROM.read(i); 
  i++;
  _numAddressPin = EEPROM.read(i); 
  i++;

  //Cycle for avoid a sequence of 8 single reading
  //After the cycle increment i
  //Here are been used two index, j for the _adressPin and i for reading
  for(byte j = 0; j<MAXADDRESSPIN; j++, i++)
    _addressPin[j] = EEPROM.read(i);

  _outputPin = EEPROM.read(i); 
  i++;
  _writeToEeprom = EEPROM.read(i);

  return;
}

void DomoS::FirstStart()
/*	Act the first start of DomoS
 	Clear the EEPROM, write the two inizial value into the EEPROM, ask all the configuration Data
 	and write them to the EEPROM
 	
 	Debugged: Don't need to be debugged
 */
{
  DomoSFileHeader data;

  ClearEeprom();

  AskData(data);

  WriteConfigurationDataToEeprom(data);

  WriteSetupData();

  return;
}

void DomoS::WriteSetupData()
/*	Write the two setup value into the first two EEPROM cells
 	
 	Debugged: OK
 */
{ 
  EEPROM.write(0, SETUP[0]);
  EEPROM.write(1, SETUP[1]);

  return;
}

void DomoS::ClearEeprom()
/*	Clear all the EEPROM setting at 0 all the EEPROM cells
 	E2END constant contain the number of the last EEPROM cell
 	
 	Debugged: OK
 */
{
  int i;

  //Cycle for all the EEPROM cells and set to 0
  for(i = 0; i <= E2END; i++)
    if(EEPROM.read(i) != 0)
      EEPROM.write(i, 0);

  return;
}

void DomoS::AskData(DomoSFileHeader & data)
/*	Ask all the configuration data to the user usign the serial interface
 	Also check for correctness of inserted values
 	
 	Debugged: OK
 	Solved a problem into the part that ask the address pin
 	The first pin wont be checked if was equal to output pin
 */
{
  byte val;	//The value read by parseInt
  byte i, j;
  boolean ok;	//Tell if the values readed are correctly

  Serial.begin(9600); //Start the serial communication

  //Allarm the user about pin 6
  Serial.println(PHRASE[5]);
  data.outputPin = 6;

  //Read variable value for numAddressPin
  Serial.println(PHRASE[0]); //Maybe i should create a custom function for writing phrases

  val = (byte)parseInt(); //Read the value

  //Check if the inserted value is greater than maximum
  //Cycle until a correct value is read
  while(val > MAXADDRESSPIN)
  {
    Serial.println(PHRASE[3]);

    val = (byte)parseInt();
  }

  data.numAddressPin = val;
  //End reading of numAddressPin

  //Read variable values for array addressPin

  //Cycle until read all the addressPin was read
  i = 0;
  while(i < data.numAddressPin)
  {
    Serial.println(PHRASE[1]);

    do //Read the value and search for duplicates
    {
      ok = true; //Assume that a correct value is read

      val = (byte)parseInt(); //Read the value

      //Check if this value is different from the previous
      //Cycle until checked all the previous values or found an identical value
      j = 0;
      if(val != data.outputPin)
      {
        while ((j < i) && (ok))
        {
          if ((val == data.addressPin[j]))
            ok = false;
          else
            j++;
        }
      }
      else
        ok = false;

      if (!ok)
        Serial.println(PHRASE[3]);


    }
    while (!ok);

    data.addressPin[i] = val; //Write the value and go ahead
    i++;
  }

  //Now that we read all the requested value, fill whit -1 all the other values
  for(i = i; i < MAXADDRESSPIN; i++)
    data.addressPin[i] = -1;

  //End reading of array addressPin

  //Read variable value for writeToEeprom
  Serial.println(PHRASE[2]);

  val = (byte)parseInt();

  data.writeToEeprom = val;
  //End reading for writeToEeprom

  data.fileVer = FILEVER; //Set fileVer to the internal file version
  data.numPeripheral = 0; //Set the number of peripheral to 0

    return;
}

int DomoS::parseInt()
/*	Parse an int from the first arduino serial port
 	Don't substitute the originale Serial.parseInt function
 	Use some trick for being sure that data was read
 	
 	Debugged: OK
 */
{
  boolean serialFetch; 	//Tell if data was read from the serial port
  int val;				//The value returned from Serial.parseInt

  serialFetch = false; //No dafa was read from the serial port
  while(!serialFetch) //Cycle until data was read
    if(Serial.available())
    {
      delay(4); //Wait 4 millisecons for allowing the serial port to ger data
      val = (int)Serial.parseInt(); //Parse the integer value from the serial port
      serialFetch = true;
    }
  Serial.flush(); //Free the serial buffer

  return val;
}

void DomoS::WriteConfigurationDataToEeprom (DomoSFileHeader data)
/*	Write the configuration parameters contain in data
 	Starting from START address start writing all the parameters following a specific sequence
 	If someone modify this sequence go to modify also GetConfigurationDataFromEeprom()
 	
 	Debugged: OK
 */
{
  byte i;

  i = START;

  //Start writing all data
  //After each writing increment i
  EEPROM.write(i, data.fileVer); 
  i++;
  EEPROM.write(i, data.numPeripheral); 
  i++;
  EEPROM.write(i, data.numAddressPin); 
  i++;

  //Cycle for avoid a sequence of 8 single writing
  //After the cycle increment i
  //Here are been used two index, j for the _addressPin and i for writing
  for(byte j = 0; j < MAXADDRESSPIN; i++, j++)
    EEPROM.write(i, data.addressPin[j]);

  EEPROM.write(i, data.outputPin); 
  i++;
  EEPROM.write(i, data.writeToEeprom);

  return;
}

boolean DomoS::ConvertDecimalToBinary(int number, boolean result[])
/*	Convert a decimal number into an array of boolean value, false for 0, true for 1
 	Return false if the number is too big for the conversion, true if the conversion goes right
 	
 	Debugged: OK
 	Corrected a typo on the first if [<= -> >=]
 	Solved a problem on the first if, div should be divided by 2
 */
{
  int part;
  int div;
  int i;
  boolean ok;

  ok = true; //Assume the conversion goes right
  div = 1 << _numAddressPin; //Equal to 2 ^ _numAddressPin
  if ((div - 1) >= number) //Check if the number is too big
  {
    div = div/2;
    i = 0;
    part = number;

    while (i < _numAddressPin)
    {
      if (part / div == 1)
      {
        result[i] = true;
        part = part % div;
      }
      else
        result[i] = false;

      i++;
      div = div / 2;
    }
  }
  else
    ok = false;

  return ok;
}

void DomoS::SetAddressing(boolean addressing[])
/*	Set the addressing pin to the values contained in addressing array
 	
 	Debugged: OK
 */
{
  byte i;

  //Cycle for all the addressing pin
  for (i = 0; i < _numAddressPin; i++)
    //Set the addressing pin to HIGH or LOW depending on addressing values
    digitalWrite(_addressPin[i], (addressing[i] ? HIGH : LOW));

  return;
}

byte DomoS::SeparateCommandBySpace()
/*	Separate the _command string into two string:
 	The _subCommand string which contain the first word of _command before a space and the _command
 	string which contain the original _command string whitouth the word in _subCommand
 	
 	Debugged: OK
 	Added in the first while the control for the string terminator
 	Added a control for eliminate the space after a word
 	Solved an error in the do...while
 */
{
  byte i, j;
  byte readChar;

  readChar = 0; //Assume we don't read any character
  i = 0; 
  j = 0;

  //Cycle until hw found a space or _subCommand is too long
  while ((_command[i] != ' ') && (_command[i] != '\0') && (j < SUBSTRINGMAXLEN))
  {
    _subCommand[j] = _command[i]; //Copy the first word
    i++; 
    j++;
  }

  if (j < SUBSTRINGMAXLEN) //Check if _subCommand is too long
  {
    _subCommand[j] = '\0'; //Terminate the string
    readChar = j; //Set the number of character readed

    if (_command[i] == ' ') //_command[i] can be ' ' or '\0', go jump of one character if is a
      //space, not if is the string terminator
      i++;

    //Start deletting the first word into _command string
    j = 0;
    //Cycle until we found the string terminator
    do
    {
      _command[j] = _command[i];
      j++; 
      i++;
    }
    while(_command[i - 1] != '\0');

    _command[j]= '\0'; //Terminate the string
  }
  else
  {
    _lastError = SUBCOMMANDSTRINGTOOLONG; //Set the error
    //Deletting _command and _subCommand
    _command[0] = '\0';
    _subCommand[0] = '\0';

    readChar = -1; //Setting readChar at an error state
  }

  return readChar;
}

byte DomoS::CompareSubCommand()
/*	Compare the _subCommand string whit the commands dictnionary
 	For avoid useless comparisons we set and start and end parameters
 	If don't found the command return the successive index
 	
 	Debugged: OK
 */
{
  boolean find;
  byte i;

  find = false; 
  i = 0;
  //Cycle until find or don't find the command
  while((!find) && (i < NCOMMAND))
  {
    if(strcmp(_subCommand, COMMAND[i]) == 0) //Compare _subCommand whit a command
      find = true;
    else
      i++;
  }

  return i;
}

void DomoS::CommandToLowerCase()
/*	Convert the command string into lower case
 	
 	Debugged: OK
 */
{
  byte i;

  i = 0;
  //Cycle until string terminator
  while (_command[i] != '\0')
  {
    //Check if there's an upper case character
    if ((_command[i] >= 'A') && (_command[i] <= 'Z'))
      _command[i] += 32; //Convert to lower case (es: A == 65 -> a == 97)

    i++;
  }

  return;
}

void DomoS::Work()
/*	This function allow the DomoS to work
 	
 	Debugged: Don't need to be debugged
 */
{
  if (GetError() == OK) //Control if there's an error pending
  {
    if(Serial.available()) //Controll if there's something to be read from the serial port
    {
      if(FetchCommand()) //Fetch the command from the serial port
      {
        CommandToLowerCase(); //Convert the _command string to lower case
        if(SeparateCommandBySpace() > 0) //Separate _command string and check if separation
          //end correclty
        {
          DoCommand(CompareSubCommand()); //Compare the command whit the dictionary and
          //execute the relative command
        }
      }
    }
  }
  else
    ThrownError();

  return;
}

byte DomoS::GetCommand(char* command)
/*	Get the index of a command from the COMMAND array
 */
{
  boolean find;
  byte i;

  find = false; 
  i = 0;
  //Cycle until find or don't find the command
  while((!find) && (i < NCOMMAND))
  {
    if(strcmp(command, COMMAND[i]) == 0) //Compare _command whit a command
      find = true;
    else
      i++;
  }

  return i;
}

boolean DomoS::FetchCommand()
/*	Fetch a command from the serial port
 	This function can be changed to whatever change the _command for input of commands
 	
 	Debugged: In part
 	Need more tests
 	Logically is right, but in case the user input too much character the serial buffer wont be free
 */
{
  boolean ok;
  byte i;

  i = 0; 
  ok = true; //Assume to read a correct string
  //Cycle until a character is avaible from the serial port and too many character are read
  while ((Serial.available() > 0) && (i < STRINGMAXLEN)) 
  {
    _command[i] = Serial.read(); //Put the readed character into the _command string
    i++;
    delay(2); //Wait 2ms for allowing the serial buffer to fill whit the next character
  }

  //Check if too many character was read
  if (i < STRINGMAXLEN)
    _command[i]='\0'; //Terminate the string
  else
  {
    //Too much character was read
    Serial.flush(); //Free the serial buffer
    _lastError = COMMANDSTRINGTOOLONG; //Set the error
    _command[0] = '\0'; //Empty the string
    ok = false;
  }

  return ok;
}

void DomoS::DoCommand(byte numCommand)
/*	Execute one of the basic comman
 	
 	Debugged: Don't need to be debugged
 */
{
  switch (numCommand)
  {
  case 0:
    Create();
    break;

  case 1:
    Turn();
    break;

  case 2:
    Delete();
    break;

  case 3:
    Exit();
    break;

  case 6:
    Reset();
    break;

  case 7:
    List();
    break;

  default:
    //If the command aren't recognized, return an error
    _lastError = COMMANDNOTRECOGNIZED;
  }

  return;
}

void DomoS::Create()
/*	This fuction act the creation of a new peripheral
 	Check the _command string for all the configuration parameters
 	
 	If everithing went right, write this new peripheral into the EEPROM/file
 */
{
  byte readChar; //The number opf character separed from the separator
  boolean error;
  DomoSFileBody newPeripheral; //The new peripheral to be write
  byte val, i;

  BlankNewPeripheral(newPeripheral); //Initialize the newPeripheral to known value

  error = false; //Assume there's no error
  if (_numPeripheral < ((1 << _numAddressPin) - 1))
  {
    if(SeparateCommandBySpace() > 0)
    {
      //Cycle when we have more parameters and there're no error
      do //Start cycle for checking parameters
      {
        switch(CompareSubCommand()) //Start extern switch
        {
        case 4: //Define the name of the new peripheral
          readChar = SeparateCommandBySpace();
          if (readChar > 0) //Check if separation goes right
          {
            if(readChar < MAXNAMELEN) //Check if the name wasn't too long
            {
              for (i = 0; _subCommand[i] != '\0'; i++) //Copy the name into the
                //new peripheral
                newPeripheral.name[i] = _subCommand[i];

              newPeripheral.name[i] = '\0';
            }
            else //Else set an error
            {
              _lastError = NAMETOOLONG;
              error = true;
            }
          }
          else
          {
            _lastError = NAMENOTDEFINED;
            error = true;
          }
          break;

          //Define the custom address name for the new peripheral
        case 5:
          readChar = SeparateCommandBySpace();

          if (readChar > 0) //Check if separation goes right
          {
            if (_subCommand[0] == 'b') //Check if the user set a binary address
            {
              if(readChar > (_numAddressPin + 1)) //Check if there're too many bit
              {
                error = true;
                _lastError = BINARYNUMBERTOOLONG;
              }
              else
              {
                SubCommandShiftLeft();
                //Convert the binary string into a decimal number
                newPeripheral.number = ConvertBinaryStringToDecimal();
              }
            }
            else
            {
              //Convert the decimal string into a decimal number
              val = (byte)atoi(_subCommand);
              if (val < (1 << _numAddressPin))
                newPeripheral.number = val;
              else
              {
                error = true;
                _lastError = DECIMALNUMBERTOOBIG;
              }
            }
          }
          else //Set an error state
          {
            error = true;
            _lastError = ASNOTDEFINED;
          }
          break;

        default: //Set an error if no command was recognized
          _lastError = SUBCOMMANDNOTRECOGNIZED;
          error = true;
          break;
        } //End extern switch
        readChar = SeparateCommandBySpace();
      }
      while((readChar > 0) && (!error)); //End cycle for checking parameters
    }

    //If there're no error try creating the new peripheral
    if(!error)
    {
      //Check if we have all the data needed
      if (CreateParameterCheck(newPeripheral))
      {
        //Write the peripheral
        if (WritePeripheral(newPeripheral, _numPeripheral))
        {
          UpdateNumPeripheral('+');
          ComposeStringPeripheral(newPeripheral, 6);
        }
      }
    }
  }
  else
    _lastError = PERIPHERALMAXIMUMNUMBERREACH;


  return;
}

void DomoS::BlankNewPeripheral (DomoSFileBody & peripheral)
/*	Initialize a peripheral to known values
 	
 	Debugged: Don't need to be debugged
 */
{
  peripheral.name[0] = '\0';
  peripheral.number = -1;

  return;
}

boolean DomoS::CreateParameterCheck(DomoSFileBody & peripheral)
/*	Controll if all the data needed for the creation of a peripheral are present
 	Also fill the empty values if they aren't required
 */
{
  boolean ok;
  boolean nameCustom, numberCustom; //True if the user defined a custom name/number

  nameCustom = true; 
  numberCustom = true; //Assume the user defined a custom name/number

  if (peripheral.number != 0) //Check if the number wasn't defined
  {
    if (peripheral.number == (byte)-1) //Check if the peripheral is 0
    {
      numberCustom = false;
      peripheral.number = _numPeripheral + 1; //If not set the number at the current number of
      //peripheral
    }

    if (peripheral.name[0] == '\0') //Check if the name wasn't defined
    {
      nameCustom = false;
      itoa((int)peripheral.number, peripheral.name, 10); //If not set the number as the
      //name
    }

    //Search if the currect peripheral have duplicated parameters and try to correct it
    ok = SearchDuplicatedPeripheral(peripheral, nameCustom, numberCustom);
  }
  else
  {
    _lastError = PERIPHERALZERONOTALLOWED;
    ok = false;
  }

  return ok;
}

boolean DomoS::WritePeripheral(DomoSFileBody peripheral, byte position)
/*	Write the peripheral on the EEPROM or on the SD if was selected
 	
 	Debugged: Don't need to be debugged 
 */
{
  boolean ok;

  ok = true; //Assume the writing goes right

  if(_writeToEeprom == (byte)-1) //Check if a CSPin was set
    ok = WritePeripheralToEeprom(peripheral, position); //If not write to EEPROM
  /*else
   		ok = WritePeripheralToSd(peripheral, position);*/  //Because i don't own a sd card to test, for the
  //instance i wont develope this function

    return ok;
}

boolean DomoS::WritePeripheralToEeprom(DomoSFileBody peripheral, byte position)
/*	Write the peripheral to the EEPROM
 	
 	Debugged: OK
 	Solved a erroneus increment of i
 */
{
  DomoSFileHeader configuration; //Only used for sizeof function
  int start; //The starting address
  byte i, j;
  boolean ok;

  ok = true; //Assume the writing goes right

  //Start to write the new peripheral from the address described by this formula
  //2 = the first two cells are occupied by the setup values
  //sizeof(configuration) = the next cells are occupied by the configuration parameters
  //In the EEPROM there're _numPeripheral peripheral big sizeof(peripheral), so go ahead
  //for all this peripheral
  start = 2 + sizeof(configuration) + (sizeof(peripheral) * position);

  if ((start + sizeof(peripheral)) < E2END) //Check if the EEPROM can contain the new peripheral
  {
    //Start writing peripheral
    for(j = 0; j < MAXNAMELEN; j++, start++)
      EEPROM.write(start, peripheral.name[j]);

    EEPROM.write(start, peripheral.number);
  }
  else	//ERROR, the EEPROM is full
  {
    ok = false;
    _lastError = EEPROMISFULL;
  }

  return ok;
}

void DomoS::UpdateNumPeripheral(char type)
/*	Update the number of peripheral
 	
 	Debugged: Don't need to be debugged
 */
{
  if (type == '+')
    _numPeripheral++;
  else
    _numPeripheral--;
  EEPROM.write(3, _numPeripheral);
  //TODO Update also the number contained on the SDCard

  return;
}

void DomoS::Turn()
/*	Act the Turn command
 */
{
  byte peripheral;
  int val;
  boolean addressing[_numAddressPin];

  SeparateCommandBySpace();

  peripheral = SearchPeripheralByName(_subCommand); //Find the peripheral
  if(peripheral != (byte)-1) 
  {
    SeparateCommandBySpace();

    switch(_subCommand[0]) //Controll the first _subCommand character
    {
    case '%': //If there's an % the user set a percentage
      SubCommandShiftLeft(); //delete the % simbol
      val = map(atoi(_subCommand), 0, 100, 0, 255); //Convert from percentage to val
      break;

    case 'v': //If there's an v the user set a voltage
      SubCommandShiftLeft(); //Delete the v simobl
      val = atof(_subCommand);

      //Check if the voltage is too high or low
      if ((val >= 0) && (val <= 5))
        val = map(atof(_subCommand), 0, 5, 0, 255); //Convert from volt to val
      else if (val < 0)
        _lastError = VOLTAGETOOLOW;
      else
        _lastError = VOLTAGETOOHIGH;
      break;

    case 'l': //If there's an l the user set a logic low signal
      val = 0; //Set val to 0
      break;

    case 'h': //If there's an h the user set a logic high signal
      val = 255; //set val to 255
      break;

    default: //If something else was set, set an error
      val = -1;
      _lastError = STRANGETURNPARAMETER;
      break;
    }

    if (val > -1)
    {
      byte number;
      GetPeripheralNumber(peripheral, number);
      //Convert the peripheral number into binary for the addressing
      if (ConvertDecimalToBinary(number, addressing))
      {
        analogWrite(_outputPin, val); //Set the outputPin at val
        delay(RCLOAD); //Wait for charging of rc circuit

        SetAddressing(addressing); //Set the addressing line

        delay(1000); //Wait for spread of signals

        //Clean everything
        analogWrite(_outputPin, 0);
        ConvertDecimalToBinary(0, addressing);
        SetAddressing(addressing);
      }
      else //If we can't convert into binary... IMPOSSIBURU
      _lastError = BADTHINGSHAPPEN;
    }
  }
  else //If the peripheral isn't present set an error
  _lastError = PERIPHERALNOTFOUND;
}

void DomoS::Exit()
/*	Act the exit from the system
 	
 	Debugged: Don't need to be debugged
 */
{
  _on = false;
  Serial.println(PHRASE[9]);
  Serial.end();

  return;
}

void DomoS::SubCommandShiftLeft()
/*	Shift of one character to left the _subCommand string
 	
 	Debugged: OK
 	Corrected some typo
 */
{
  byte i;

  i = 0;
  do
  {
    _subCommand[i] = _subCommand[i+1];
    i++;
  } 
  while (_subCommand[i] != '\0');

  return;
}

byte DomoS::SearchPeripheralByName(char peripheral[])
/*	Search the peripheral by name
 	The function return the index of the peripheral or -1 if not found
 	
 	Debugged: OK
 */
{
  boolean find;
  byte i;
  char name[MAXNAMELEN];

  find = false; //Assume we don't find the peripheral
  //Cycle until we find the peripheral or the peripherals are finished
  i = 0;
  while ((i < _numPeripheral) && (!find))
  {
    GetPeripheralName(i, name); //Get the name of the i-th peripheral

    if (strcmp(peripheral, name) == 0) //Check if the name are equal
      find = true; //We find the peripheral
    else
      i++; //Go ahead
  }

  if (!find) //if the peripheral wasn't find set an error
    i = -1;

  return i;
}

void DomoS::GetPeripheralName(byte numPeripheral, char name[])
/*	Put into char name[] the name of the numPeripheral-th peripheral
 	
 	Debugged: Ok
 	Modify the second sizeof for calculate the size of the correct structure
 */
{
  int peripheral;
  DomoSFileBody body;
  DomoSFileHeader configuration;
  byte i;

  //The numPeripheral-th peripheral is stored from the address described by this formula
  //2 = the first two cells are occupied by the setup values
  //sizeof(configuration) = the next cells are occupied by the configuration parameters
  //In the EEPROM there're numPeripheral peripheral big sizeof(peripheral), so go ahead
  //for all this peripheral
  peripheral = 2 + sizeof(configuration) + (sizeof(body) * numPeripheral);

  for (i = 0; i < MAXNAMELEN; i++)
    name[i] = EEPROM.read(peripheral + i);

  return;
}

boolean DomoS::SearchDuplicatedPeripheral(DomoSFileBody & peripheral, boolean nameCustom, boolean numberCustom)
/*	Search for duplicated peripheral, both by name and number
 	If no custom name and number was set, modify that for making the new peripheral unique
 */
{
  boolean error;
  byte i;

  error = false; //Assume that there aren't errors
  if (nameCustom) //Check if the user set the name
  {
    if (SearchPeripheralByName(peripheral.name) != (byte)-1) //Check if the user's name is already present
    {
      _lastError = PERIPHERALNAMENOTUNIQUE;
      error = true;
    }
  }

  if ((numberCustom) && (!error)) //Check if the user set a number and there aren't errors
  {
    if (SearchPeripheralByNumber(peripheral.number) != (byte)-1) //Check if the user's number is already present
    {
      _lastError = PERIPHERALNUMBERNOTUNIQUE;
      error = true;
    }
  }

  if ((!nameCustom) && (!error)) //Check if the user doesn't set a name and there aren't errors
  {
    i = 0;
    //Cycle until the allowed name are finished (a bit strange, but i think can happen) or 
    //a free name was found
    while ((i < 255) && (SearchPeripheralByName(peripheral.name) != (byte)-1))
    {
      itoa((atoi(peripheral.name) + 1), peripheral.name, 10); //Same as peripheral.name++;
      i++;
    }

    if (i == 255) //Check if no available name was found
    {
      //If yes set an error
      error = true;
      _lastError = CANTFINDFREENAME; 
    }
  }

  if ((!numberCustom) && (!error))
  {
    i = 0;
    //Cycle until the allowed number are finished (a bit strange, but i think can happen) or 
    //a free number was found
    while ((i < ((1 << _numAddressPin) - 1)) && (SearchPeripheralByNumber(peripheral.number) != (byte)-1))
    {
      peripheral.number = ((peripheral.number + 1) % (1 << _numAddressPin));
      if(peripheral.number == 0)
        peripheral.number++;
      i++;
    }

    if (i == ((1 << _numAddressPin) - 1)) //Check if no available number was found
    {
      error = true;
      _lastError = CANTFINDFREENUMBER; 
    }
  }

  return (!error);
}

byte DomoS::SearchPeripheralByNumber(byte peripheral)
/*	Search the peripheral by number
 	The function return the index of the peripheral or -1 if not found
 	
 	Debugged: OK
 */
{
  boolean find;
  byte i;
  byte number;

  find = false; //Assume we don't find the peripheral
  //Cycle until we find the peripheral or the peripherals are finished
  i = 0;
  while ((i < _numPeripheral) && (!find))
  {
    GetPeripheralNumber(i, number); //Get the name of the i-th peripheral

    if (peripheral == number) //Check if the name are equal
      find = true; //We find the peripheral
    else
      i++; //Go ahead
  }

  if (!find) //if the peripheral wasn't find set an error
    i = -1;

  return i;
}

void DomoS::GetPeripheralNumber(byte numPeripheral, byte & number)
/*	Put into char name[] the name of the numPeripheral-th peripheral
 	
 	Debugged: Ok
 	Modify the second sizeof for calculate the size of the correct structure
 */
{
  int peripheral;
  DomoSFileBody body;
  DomoSFileHeader configuration;
  byte i;

  //The numPeripheral-th peripheral is stored from the address described by this formula
  //2 = the first two cells are occupied by the setup values
  //sizeof(configuration) = the next cells are occupied by the configuration parameters
  //In the EEPROM there're numPeripheral peripheral big sizeof(peripheral), so go ahead
  //for all this peripheral
  //Before the number thare're the name of the peripheral, so go ahead of MAXNAMELEN cells
  peripheral = 2 + sizeof(configuration) + (sizeof(body) * numPeripheral) + MAXNAMELEN;

  number = EEPROM.read(peripheral);

  return;
}

void DomoS::ThrownError()
/*	Give a description of the last error occurred
 	
 	Debugged: Don't need to be debugged
 */
{
  //Get the error
  switch (GetError())
    //Give a little description
  {
  case COMMANDSTRINGTOOLONG:
    Serial.println(ERROR[COMMANDSTRINGTOOLONG]);
    break;

  case SUBCOMMANDSTRINGTOOLONG:
    Serial.println(ERROR[SUBCOMMANDSTRINGTOOLONG]);
    break;

  case COMMANDNOTRECOGNIZED:
    Serial.println(ERROR[COMMANDNOTRECOGNIZED]);
    break;

  case NOCOMMANDPARAMETERS:
    Serial.println(ERROR[NOCOMMANDPARAMETERS]);
    break;

  case SUBCOMMANDNOTRECOGNIZED:
    Serial.println(ERROR[SUBCOMMANDNOTRECOGNIZED]);
    break;

  case NAMENOTDEFINED:
    Serial.println(ERROR[NAMENOTDEFINED]);
    break;

  case NAMETOOLONG:
    Serial.println(ERROR[NAMETOOLONG]);
    break;

  case ASNOTDEFINED:
    Serial.println(ERROR[ASNOTDEFINED]);
    break;

  case BINARYNUMBERTOOLONG:
    Serial.println(ERROR[BINARYNUMBERTOOLONG]);
    break;

  case EEPROMISFULL:
    Serial.println(ERROR[EEPROMISFULL]);
    break;

  case PERIPHERALNOTFOUND:
    Serial.println(ERROR[PERIPHERALNOTFOUND]);
    break;

  case STRANGETURNPARAMETER:
    Serial.println(ERROR[STRANGETURNPARAMETER]);
    break;

  case BADTHINGSHAPPEN:
    Serial.println(ERROR[BADTHINGSHAPPEN]);
    break;

  case PERIPHERALZERONOTALLOWED:
    Serial.println(ERROR[PERIPHERALZERONOTALLOWED]);
    break;

  case PERIPHERALMAXIMUMNUMBERREACH:
    Serial.println(ERROR[PERIPHERALMAXIMUMNUMBERREACH]);
    break;

  case PERIPHERALNAMENOTUNIQUE:
    Serial.println(ERROR[PERIPHERALNAMENOTUNIQUE]);
    break;

  case PERIPHERALNUMBERNOTUNIQUE:
    Serial.println(ERROR[PERIPHERALNUMBERNOTUNIQUE]);
    break;

  case CANTFINDFREENAME:
    Serial.println(ERROR[CANTFINDFREENAME]);
    break;

  case CANTFINDFREENUMBER:
    Serial.println(ERROR[CANTFINDFREENUMBER]);
    break;

  case VOLTAGETOOLOW:
    Serial.println(ERROR[VOLTAGETOOLOW]);
    break;

  case VOLTAGETOOHIGH:
    Serial.println(ERROR[VOLTAGETOOHIGH]);
    break;

  case DECIMALNUMBERTOOBIG:
    Serial.println(ERROR[DECIMALNUMBERTOOBIG]);
    break;

  case THEREAREZEROPERIPHERAL:
    Serial.println(ERROR[THEREAREZEROPERIPHERAL]);
    break;

  default:
    Serial.println(ERROR[BADTHINGSHAPPEN]);
    break;
  }

  _lastError = OK;

  return;
}

byte DomoS::GetError()
/*	Return the last error
 	
 	Debugged: Don't need to be debugged
 */
{
  return _lastError;
}

byte DomoS::ConvertBinaryStringToDecimal()
/*	Convert the _subCommand string from binary to decimal
 	
 	Debugged: OK
 	Corrected some typos [base/2 -> base = base/2], [_subCommand[i] == 1 -> == '1']
 */
{
  byte base;
  byte i;
  byte number;

  base = 1 << (_numAddressPin - 1);

  for (i = 0, number = 0; i < _numAddressPin; i++, base = base/2)
    if (_subCommand[i] == '1')
      number += base;

  return number;
}

boolean DomoS::IsOn()
/*	Tell if the system is active
 	
 	Debugged: Don't need to be debugged
 */
{
  return _on;
}

void DomoS::Reset()
/*
*/
{
  EEPROM.write(0,0);
  EEPROM.write(1,0);

  Serial.println(PHRASE[8]);

  return;
}

void DomoS::ComposeStringPeripheral(DomoSFileBody peripheral, byte phrase)
/*
*/
{
  char output[82];
  char buff[9];
  byte i, j, k;

  i = 0; 
  j = 0; 
  k = 0;
  while (PHRASE[phrase][i] != '\0')
  {
    switch (PHRASE[phrase][i])
    {
    case 'N':
      k = 0;
      while(peripheral.name[k] != '\0')
      {
        output[j] = peripheral.name[k];
        j++; 
        k++;
      }
      break;

    case 'M':
      itoa(peripheral.number, buff, 10);
      k = 0;
      while(buff[k] != '\0')
      {
        output[j] = buff[k];
        j++; 
        k++;
      }
      break;

    case 'B':
      itoa(peripheral.number, buff, 2);
      k = 0;
      while(buff[k] != '\0')
      {
        output[j] = buff[k];
        j++; 
        k++;
      }
      break;

    default:
      output[j] = PHRASE[phrase][i];
      j++;
      break;
    }

    i++;
  }
  output[j] = '\0';

  Serial.println(output);

  return;
}

void DomoS::List()
/*
*/
{
  byte i;
  DomoSFileBody peripheral;

  for(i = 0; i < _numPeripheral; i++)
  {
    GetPeripheralName(i, peripheral.name);
    GetPeripheralNumber(i, peripheral.number);

    ComposeStringPeripheral(peripheral, 10);
  }

  if(i == 0)
    _lastError = THEREAREZEROPERIPHERAL;

  return;
}

void DomoS::Delete()
/*
*/
{
  byte position;
  DomoSFileBody newPeripheral;

  SeparateCommandBySpace();

  position = SearchPeripheralByName(_subCommand);
  if (position != (byte)-1)
  {
    if (position < (_numPeripheral - 1))
    {
      GetPeripheralName(_numPeripheral - 1, newPeripheral.name);
      GetPeripheralNumber(_numPeripheral - 1, newPeripheral.number);

      WritePeripheral(newPeripheral, position);
    }
    UpdateNumPeripheral('-');

    Serial.println(PHRASE[11]);
  }
  else
    _lastError = PERIPHERALNOTFOUND;

  return;
}

