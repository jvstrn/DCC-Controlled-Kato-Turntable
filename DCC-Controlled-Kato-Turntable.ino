#include <DRV8835MotorShield.h>                            // Pololu DRV8835 Dual Motor Driver Shield for Arduino
#include <DCC_Decoder.h>                                   // Mynabay DCC library

#define kDCC_INTERRUPT                   0                 // DCC Interrupt 0
#define DCC_PIN                          2                 // DCC signal = Interrupt 0
#define LED_PIN                         13                 // Onboard Arduino LED Pin
#define TURNTABLE_SWITCH_PIN             6                 // Kato Turntable Pin 1
#define MAX_DCC_Accessories              3                 // Number of DCC Accessory Decoders
#define maxSpeed                       200                 // Speed between -400 = Reversed to 400 = Forward (-3 to +3 VDC)
#define maxTrack                        36                 // Total Number of Turntable Tracks

byte Output_Pin                     =   13;                // Arduino LED Pin
int Turntable_Count                 =    0;                // Track Counter
int Turntable_Current               =    0;                // Current Turntable Track
int Turntable_NewTrack              =    0;                // New Turntable Track
int speedValue                      =    0;                // Turntable Motor Speed
int Turntable_NewSwitchState        = HIGH;                // New Switch Status (From HIGH to LOW = Bridge in position)
int Turntable_OldSwitchState        = HIGH;                // Old Switch Status (HIGH = Bridge not in position)
unsigned long Turntable_TurnStart   =    0;                // Start time to turn before stop
unsigned long Turntable_TurnTime    = 1000;                // Minimun time to turn before stop
unsigned long Turntable_SwitchTime  =    0;                // Last time the output pin was toggled
unsigned long Turntable_SwitchDelay =    5;                // Debounce time in ms

const char* Turntable_State[] =                            // Possible Turntable States
{
  "T1CW",                                                  // Turn 1 Step ClockWise
  "T1CCW",                                                 // Turn 1 Step Counter ClockWise
  "TCW",                                                   // Turn ClockWise
  "TCCW",                                                  // Turn Counter ClockWise
  "T180",                                                  // Turn 180
  "STOP",                                                  // Stop Turning
  "POS",                                                   // Bridge in Position
  "MCW",                                                   // Motor ClockWise
  "MCCW"                                                   // Motor Counter ClockWise
};

enum Turntable_NewActions                                  // Possible Turntable Actions
{
  T1CW,                                                    // Turn 1 Step ClockWise
  T1CCW,                                                   // Turn 1 Step Counter ClockWise
  TCW,                                                     // Turn ClockWise
  TCCW,                                                    // Turn Counter ClockWise
  T180,                                                    // Turn 180
  STOP,                                                    // Stop Turning
  POS,                                                     // Bridge in Position
  MCW,                                                     // Motor ClockWise
  MCCW                                                     // Motor Counter ClockWise
};

enum Turntable_NewActions Turntable_OldAction = POS;       // Stores Turntable Previous Action
enum Turntable_NewActions Turntable_NewAction = POS;       // Stores Turntable New Action

typedef struct                                             // Begin DCC Accessory Structure
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
DCC_Accessory_Structure;                                   // End DCC Accessory Structure

DCC_Accessory_Structure DCC_Accessory[MAX_DCC_Accessories];// DCC Accessory
DRV8835MotorShield      Turntable;                         // Turntable Motor M1 = Bridge, Motor M2 = Lock


void setup()
{
  Serial.begin(38400);
  Serial.println("DCC Packet Analyze");
  /*
  for (i=0; i< NUMBUTTONS; i++)
  {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }
  */
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
    Serial.print(" (");
    Serial.print( (DCC_Accessory[addr].Button) ? "Green" : "Red" ); // 0 = Red, 1 = Green
    Serial.print(")");
    Serial.println();
    switch (DCC_Accessory[addr].Address)
    {
      case (225):                                          // DCC Address LDT 225 0 = END, 1 = INPUT
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = END
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;                      // Action: Stop Motor M1
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = INPUT
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = STOP;                      // Action: Stop Motor M1
        }
        //PrintStatus();
        break;
      case (226):                                          // DCC Address LDT 226 0 = CLEAR, 1 = TURN 180
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = CLEAR
        {
          Turntable_Current = 0;                           // Bridge in Home Position
          Turntable_NewTrack = 0;                          // Bridge in Home Position
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = POS;                       // Action: Bridge in Position
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = TURN 180
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current + (maxTrack / 2);
          if (Turntable_NewTrack >= maxTrack)
            Turntable_NewTrack = Turntable_NewTrack - maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T180;                      // Action: Turn Motor M1 18 Steps ClockWise
        }
        //PrintStatus();
        break;
      case (227):                                          // DCC Address LDT 227 0 = 1 STEP CW, 1 = 1 STEP CCW
        if (DCC_Accessory[addr].Button == 0)               // Red Button   : 0 = TURN 1 Step ClockWise
        {
          Output_Pin = DCC_Accessory[addr].OutputPin1;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current + 1;
          if (Turntable_NewTrack >= maxTrack)              // From Track 35 to Track 0
            Turntable_NewTrack = Turntable_NewTrack - maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T1CW;                      // Action: Turn Motor M1 1 Step ClockWise
        }
        if (DCC_Accessory[addr].Button == 1)               // Green Button : 1 = TURN 1 Step Counter ClockWise
        {
          Output_Pin = DCC_Accessory[addr].OutputPin2;     // Set Arduino Output Pin
          Turntable_NewTrack = Turntable_Current - 1;
          if (Turntable_NewTrack < 0)                      // From Track 0 to Track 35
            Turntable_NewTrack = Turntable_NewTrack + maxTrack;
          Turntable_OldAction = Turntable_NewAction;
          Turntable_NewAction = T1CCW;                     // Action: Turn Motor M1 1 Step Counter ClockWise
        }
        //PrintStatus();
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
        //PrintStatus();
        break;
    }
    Serial.print("DCC_Accessory_CheckStatus --> ");
    PrintStatus();                                         // Print Actions and Track Numbers
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
  if (((millis() - Turntable_TurnStart) > Turntable_TurnTime) && (Turntable_NewAction != POS))
  {
    Turntable_CheckSwitch();                               // Check Kato Turntable Pin 1
  }
  if ((Turntable_OldAction != STOP) && (Turntable_NewAction == STOP))
  {
    
    if ((Turntable_OldAction == MCW || Turntable_OldAction == TCW))
    {
      Turntable_Current = Turntable_Current + 1;
      if (Turntable_Current >= maxTrack)                   // From Track 35 to Track 0
        Turntable_Current = Turntable_Current - maxTrack;
      digitalWrite(Output_Pin, LOW);                       // LED OFF
    }
    
    if (Turntable_OldAction == MCCW || Turntable_OldAction == TCCW)
    {
      Turntable_Current = Turntable_Current - 1;
      if (Turntable_Current < 0)                           // From Track 0 to Track 35
        Turntable_Current = Turntable_Current + maxTrack;
      digitalWrite(Output_Pin, LOW);                       // LED OFF
    }
    
    if (Turntable_Current == Turntable_NewTrack)           // Bridge in Position
    {
      Turntable_Stop();                                    // Motor M1 Stop
      Turntable_OldAction = Turntable_NewAction;
      Turntable_NewAction = POS;                           // Action: Bridge in Position
      Serial.print("Loop Current = NewTrack   --> ");
      PrintStatus();                                       // Print Actions and Track Numbers
    }
  }

  if ((Turntable_OldAction != T1CW) && (Turntable_NewAction == T1CW))
  {
    Turntable_MotorCW();                                   // Motor M1 Forward
    Turntable_OldAction = Turntable_NewAction;
    Turntable_NewAction = MCW;                             // Action: Move Motor M1 ClockWise
    Serial.print("Loop != T1CW + T1CW       --> ");
    PrintStatus();                                         // Print Actions and Track Numbers
  }
  
  if ((Turntable_OldAction != T1CCW) && (Turntable_NewAction == T1CCW))
  {
    Turntable_MotorCCW();                                  // Motor M1 Reverse
    Turntable_OldAction = Turntable_NewAction;
    Turntable_NewAction = MCCW;                            // Action: Move Motor M1 Counter ClockWise
    Serial.print("Loop != T1CCW + T1CCW     --> ");
    PrintStatus();                                         // Print Actions and Track Numbers
  }

} // END loop


void Turntable_Stop()                                      // Motor M1 Stop
{
  digitalWrite(LED_PIN, HIGH);                             // LED ON
  digitalWrite(Output_Pin, LOW);                           // LED OFF
  switch (Turntable_OldAction)
  {
    case MCW:                                              // Motor was turning ClockWise
      for (int speed = speedValue; speed > 0; speed--)
      {
        delay(3);                                          // Delay to get better bridge to track position
        Turntable.setM1Speed(speed);                       // Motor M1 Speed 0
      }
      break;
    case MCCW:                                             // Motor was turning Counter ClockWise
      for (int speed = speedValue; speed < 0; speed++)
      {
        delay(3);                                          // Delay to get better bridge to track position
        Turntable.setM1Speed(speed);                       // Motor M1 Speed 0
      }
      break;
  }
} // END Turntable_Stop


void Turntable_MotorCW()                                   // Motor M1 Forward
{
  digitalWrite(LED_PIN, LOW);                              // LED OFF
  digitalWrite(Output_Pin, HIGH);                          // LED ON
  speedValue = maxSpeed;                                   // Positive = Forward
  Turntable.setM1Speed(speedValue);                        // Motor M1 Speed value
  Turntable_TurnStart = millis();                          // Time when turn starts
  //Serial.println(Turntable_TurnStart);
} // END Turntable_MotorCW


void Turntable_MotorCCW()                                  // Motor M1 Reverse
{
  digitalWrite(LED_PIN, LOW);                              // LED OFF
  digitalWrite(Output_Pin, HIGH);                          // LED ON
  speedValue = -maxSpeed;                                  // Negative = Reverse
  Turntable.setM1Speed(speedValue);                        // Motor M1 Speed value
  Turntable_TurnStart = millis();                          // Time when turn starts
  //Serial.println(Turntable_TurnStart);
} // END Turntable_MotorCCW


void Turntable_CheckSwitch()                               // From HIGH to LOW = Bridge in next position
{
  int SwitchState = digitalRead(TURNTABLE_SWITCH_PIN);
  if (SwitchState != Turntable_OldSwitchState)
  {
	  Turntable_SwitchTime = millis();
  }
  if ((millis() - Turntable_SwitchTime) > Turntable_SwitchDelay)
  {
    if (SwitchState != Turntable_NewSwitchState)
    {
      Turntable_NewSwitchState = SwitchState;
      if (Turntable_NewSwitchState == LOW)
      {
        digitalWrite(Output_Pin, LOW);                     // LED OFF
        Turntable_OldAction = Turntable_NewAction;
        Turntable_NewAction = STOP;                        // Stop Motor M1
        Serial.print("Turntable_CheckSwitch     --> ");
        PrintStatus();                                     // Print Actions and Track Numbers
      }
    }
  }
  Turntable_OldSwitchState = SwitchState;
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


