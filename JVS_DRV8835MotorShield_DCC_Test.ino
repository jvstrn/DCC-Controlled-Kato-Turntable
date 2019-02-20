#include <DRV8835MotorShield.h>
#include <DCC_Decoder.h>                                   // Mynabay DCC library

#define kDCC_INTERRUPT             0                       // DCC Interrupt 0
#define DCC_PIN                    2                       // DCC signal = Interrupt 0
#define LED_PIN                   13                       // Onboard Arduino LED Pin
#define TURNTABLE_SWITCH_PIN       6                       // Kato Turntable Pin 1
#define MAX_DCC_Accessories        3                       // Number of DCC Accessory Decoders
#define maxSpeed                 400                       // Speed between -400 = Reversed to 400 = Forward (-3 to +3 VDC)
#define maxTrack                  36                       // Total Number of Turntable Tracks

byte Output_Pin               =   13;                      // Arduino LED Pin
int Turntable_Count           =    0;                      // Track Counter
int Turntable_Current         =    0;                      // Current Turntable Track
int Turntable_NewTrack        =    0;                      // New Turntable Track
int speedValue                =  200;                      // Turntable Motor Speed
int Turntable_NewSwitchState  = HIGH;                      // New Switch Status (From HIGH to LOW = Bridge in position)
int Turntable_OldSwitchState  = HIGH;                      // Old Switch Status (HIGH = Bridge not in position)
String New_Message            =   "";                      // New Message to Serial Print
String Old_Message            =   "";                      // Old Message to Serial Print

enum Turntable_NewActions
{
  T1CW,                                                    // Turn 1 Step ClockWise
  T1CCW,                                                   // Turn 1 Step Counter ClockWise
  TCW,                                                     // Turn ClockWise
  TCCW,                                                    // Turn Counter ClockWise
  T180,                                                    // Turn 180
  STOP,                                                    // Stop Turning
  POS                                                      // Bridge in Position
};
enum Turntable_NewActions Turntable_OldAction = POS;
enum Turntable_NewActions Turntable_NewAction = POS;

const char* Turntable_State[] = {"T1CW", "T1CCW", "TCW", "TCCW", "T180", "STOP", "POS"};

DRV8835MotorShield        Turntable;                       // Turntable Motor M1 = Bridge, Motor M2 = Lock

typedef struct
{
  int               Address;                               // DCC Address to respond to
  byte              Button;                                // Accessory Button: 0 = Off (Red), 1 = On (Green)
  int               Track1;                                // Turntable Track 1
  int               Track2;                                // Turntable Track 2
  int               OutputPin1;                            // Arduino Output Pin 1
  int               OutputPin2;                            // Arduino Output Pin 2
  boolean           Finished;                              // Command Busy = 0 or Finished = 1 (Ready for next command)
  boolean           Active;                                // Command Not Active = 0, Active = 1
  unsigned long     durationMilli;                         // Pulse Time in ms
  unsigned long     offMilli;                              // For internal use // Do not change this value
}
DCC_Accessory_Address;
DCC_Accessory_Address DCC_Accessory[MAX_DCC_Accessories];


void Turntable_Stop()
{
  digitalWrite(Output_Pin, LOW);                           // LED OFF
  Turntable.setM1Speed(0);                                 // Motor M1 Stop
} // END Turntable_Stop


void Turntable_MotorCW()
{
  digitalWrite(Output_Pin, HIGH);                          // LED ON
  Turntable.setM1Speed(speedValue);                        // Motor M1 Forward
} // END Turntable_MotorCW


void Turntable_MotorCCW()
{
  digitalWrite(Output_Pin, HIGH);                          // LED ON
  Turntable.setM1Speed(-speedValue);                       // Motor M1 Reverse
} // END Turntable_MotorCCW


void setup()
{
  Serial.begin(38400);
  Serial.println("DCC Packet Analyze");
  pinMode(TURNTABLE_SWITCH_PIN, INPUT);                    // Kato Turntable Pin 1
  pinMode(LED_PIN, OUTPUT);                                // Internal LED Arduino Pin 13
  digitalWrite(LED_PIN,LOW);                               // Turn Off Arduino LED at startup
  pinMode(DCC_PIN,INPUT_PULLUP);                           // Interrupt 0 with internal pull up resistor (can get rid of external 10k)
  DCC.SetBasicAccessoryDecoderPacketHandler(BasicAccDecoderPacket_Handler, true);
  DCC_Accessory_ConfigureDecoderFunctions();
  DCC.SetupDecoder( 0x00, 0x00, kDCC_INTERRUPT );
  for (int i = 0; i < MAX_DCC_Accessories; i++)
  {
    DCC_Accessory[i].Button = 0;                            // Disable all DCC decoders
  }
} // END setup


void BasicAccDecoderPacket_Handler(int address, boolean activate, byte data)
{
  address -= 1;
  address *= 4;
  address += 1;
  address += (data & 0x06) >> 1;                           // Convert NMRA packet address format to human address
  boolean enable = (data & 0x01) ? 1 : 0;
  for(int i = 0; i < MAX_DCC_Accessories; i++)
  {
    if (address == DCC_Accessory[i].Address)
    {
      DCC_Accessory[i].Active = 1;                         // DCC Accessory Active
      if (enable)
        DCC_Accessory[i].Button = 1;                       // Green Button
      else
        DCC_Accessory[i].Button = 0;                       // Red Button
    }
  }
} // END BasicAccDecoderPacket_Handler


void DCC_Accessory_ConfigureDecoderFunctions()
{
  DCC_Accessory[0].Address       =   225;                  // DCC Address LDT 225 0 = END, 1 = INPUT (For now both will stop Turntable)
  DCC_Accessory[0].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[0].Track1        =     0;                  // Turntable Track 1
  DCC_Accessory[0].Track2        =     0;                  // Turntable Track 2
  DCC_Accessory[2].OutputPin1    =    13;                  // Arduino Output Pin 13 = Onboard LED
  DCC_Accessory[2].OutputPin2    =    13;                  // Arduino Output Pin 13 = Onboard LED
  DCC_Accessory[0].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[0].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[0].durationMilli =   250;                  // Pulse Time in ms

  DCC_Accessory[1].Address       =   226;                  // DCC Address LDT 226 0 = CLEAR, 1 = TURN 180
  DCC_Accessory[1].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[1].Track1        =     0;                  // Turntable Track 1
  DCC_Accessory[1].Track2        =     0;                  // Turntable Track 2
  DCC_Accessory[2].OutputPin1    =    13;                  // Arduino Output Pin 13 = Onboard LED
  DCC_Accessory[2].OutputPin2    =    13;                  // Arduino Output Pin 13 = Onboard LED
  DCC_Accessory[1].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[1].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[1].durationMilli =   250;                  // Pulse Time in ms
  
  DCC_Accessory[2].Address       =   227;                  // DCC Address LDT 227 0 = 1 STEP CW, 1 = 1 STEP CCW
  DCC_Accessory[2].Button        =     0;                  // Accessory Button: 0 = Off (Red), 1 = On (Green)
  DCC_Accessory[2].Track1        =     0;                  // Turntable Track 1
  DCC_Accessory[2].Track2        =     0;                  // Turntable Track 2
  DCC_Accessory[2].OutputPin1    =    11;                  // Arduino Output Pin 11 = Red LED
  DCC_Accessory[2].OutputPin2    =    12;                  // Arduino Output Pin 12 = Green LED
  DCC_Accessory[2].Finished      =     1;                  // Command Busy = 0 or Finished = 1
  DCC_Accessory[2].Active        =     0;                  // Command Not Active = 0, Active = 1
  DCC_Accessory[2].durationMilli =   250;                  // Pulse Time in ms

  for (int i = 0; i < MAX_DCC_Accessories; i++)            // Configure Arduino Output Pin
  {
    if (DCC_Accessory[i].OutputPin1)
    {
      pinMode(DCC_Accessory[i].OutputPin1, OUTPUT);
      digitalWrite(DCC_Accessory[i].OutputPin1, LOW);
    }
    if (DCC_Accessory[i].OutputPin2)
    {
      pinMode(DCC_Accessory[i].OutputPin2, OUTPUT);
      digitalWrite(DCC_Accessory[i].OutputPin2, LOW);
    }
  }
} // END DCC_Accessory_ConfigureDecoderFunctions


void DCC_Accessory_CheckStatus()
{
  static int addr = 0;
  DCC.loop();                                              // Loop DCC Library
  if (DCC_Accessory[addr].Finished && DCC_Accessory[addr].Active)
  {
    DCC_Accessory[addr].Finished = 0;
    DCC_Accessory[addr].offMilli = millis() + DCC_Accessory[addr].durationMilli;
    Serial.print("Address: ");
    Serial.print(DCC_Accessory[addr].Address);
    Serial.print(", ");
    Serial.print("Button: ");
    Serial.print(DCC_Accessory[addr].Button);
    Serial.println();
    switch (DCC_Accessory[addr].Address)
    {
      case (225):                                          // DCC Address LDT 225 0 = END, 1 = INPUT
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = END
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;
          //PrintStatus();
          //Turntable_Stop();                                // Motor M1 Stop
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = INPUT
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;
          //PrintStatus();
          //Turntable_Stop();                                // Motor M1 Stop
        }
        break;
      case (226):                                          // DCC Address LDT 226 0 = CLEAR, 1 = TURN 180
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = CLEAR
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;
          //PrintStatus();
          //Turntable_Stop();                                // Motor M1 Stop
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = TURN 180
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current + (maxTrack / 2);
          if (Turntable_NewTrack >= maxTrack)
            Turntable_NewTrack = Turntable_NewTrack - maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T180;
          //PrintStatus();
        }
        break;
      case (227):                                          // DCC Address LDT 227 0 = 1 STEP CW, 1 = 1 STEP CCW
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = TURN 1 Step ClockWise
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current + 1;
          if (Turntable_NewTrack >= maxTrack)              // From Track 35 to Track 0
            Turntable_NewTrack = Turntable_NewTrack - maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T1CW;
          //PrintStatus();
          //Turntable_MotorCW();                             // Motor M1 Forward
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = TURN 1 Step Counter ClockWise
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current - 1;
          if (Turntable_NewTrack < 0)                      // From Track 0 to Track 35
            Turntable_NewTrack = Turntable_NewTrack + maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T1CCW;
          //PrintStatus();
          //Turntable_MotorCCW();                            // Motor M1 Reverse
        }
        break;
      default:
	    /*
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = Goto Track Track1
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_NewTrack = (DCC_Accessory[addr].Track1);
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = Goto Track Track2
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_NewTrack = (DCC_Accessory[addr].Track2);
        }
        if (Turntable_NewTrack > Turntable_Current)        // Turn Bridge ClockWise
        {
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = TCW;
        }
        if (Turntable_NewTrack < Turntable_Current)        // Turn Bridge Counter ClockWise
        {
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = TCCW;
        }
        if (Turntable_NewTrack = Turntable_Current)        // Bridge at position
        {
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;
        }
		*/
        break;
    }
  }
  if ((!DCC_Accessory[addr].Finished) && (millis() > DCC_Accessory[addr].offMilli))
  {
    DCC_Accessory[addr].Finished = 1;
    DCC_Accessory[addr].Active = 0;
  }
  if (++addr >= MAX_DCC_Accessories)
    addr = 0;
} // END DCC_Accessory_CheckStatus


void loop()
{
  DCC_Accessory_CheckStatus();                             // Check DCC Accessory Status
  Turntable_CheckSwitch();                                 // Check Kato Turntable Pin 1
  if ((Turntable_NewAction == POS) && (Turntable_OldAction != POS))
  {
    //PrintStatus();
    if ((Turntable_OldAction == T1CW || Turntable_OldAction == TCW))
    {
      digitalWrite(Output_Pin, LOW);                       // LED OFF
      Turntable_Current = Turntable_Current + 1;
      if (Turntable_Current >= maxTrack)                   // From Track 35 to Track 0
        Turntable_Current = Turntable_Current - maxTrack;
    }
    if (Turntable_OldAction == T1CCW || Turntable_OldAction == TCCW)
    {
      Turntable_Current = Turntable_Current - 1;
      if (Turntable_Current < 0)                           // From Track 0 to Track 35
        Turntable_Current = Turntable_Current + maxTrack;
    }
    if (Turntable_Current == Turntable_NewTrack)           // Bridge in position
      Turntable_Stop();                                    // Motor M1 Stop
  }
  
  if ((Turntable_NewAction == T1CW) && (Turntable_OldAction != T1CW))
  {
    PrintStatus();
    Turntable_MotorCW();                                   // Motor M1 Forward
  }
  
  if ((Turntable_NewAction == T1CCW) && (Turntable_OldAction != T1CCW))
  {
    PrintStatus();
    Turntable_MotorCCW();                                  // Motor M1 Reverse
  }

} // END loop


void Turntable_CheckSwitch()                               // From HIGH to LOW = Bridge in next position
{
  Turntable_NewSwitchState = digitalRead(TURNTABLE_SWITCH_PIN);
  if (Turntable_NewSwitchState != Turntable_OldSwitchState)
  {
    if (Turntable_NewSwitchState == LOW)
    {	
      Turntable_OldAction = Turntable_NewAction;
      Turntable_NewAction = POS;                           // Bridge in next position
      //PrintStatus();
    }
  }
  Turntable_OldSwitchState = Turntable_NewSwitchState;
} // END Turntable_CheckSwitch


void PrintStatus()
{
  Serial.print("Old Action: ");
  Serial.print(Turntable_State[Turntable_OldAction]);
  Serial.print(", New Action: ");
  Serial.print(Turntable_State[Turntable_NewAction]);
  Serial.print(", Current Track: ");
  Serial.print(Turntable_Current);
  Serial.print(", NewTrack: ");
  Serial.print(Turntable_NewTrack);
  Serial.print(", Output_Pin: ");
  Serial.print(Output_Pin);
  Serial.println();
} // END PrintStatus


