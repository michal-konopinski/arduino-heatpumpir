#include <Arduino.h>
#include <SamsungHeatpumpIR.h>

SamsungHeatpumpIR::SamsungHeatpumpIR() : HeatpumpIR()
{
  static const prog_char model[] PROGMEM = "samsung";
  static const prog_char info[]  PROGMEM = "{\"mdl\":\"samsung\",\"dn\":\"Samsung\",\"mT\":16,\"xT\":27,\"fs\":4}";

  _model = model;
  _info = info;
}


void SamsungHeatpumpIR::send(IRSender& IR, byte powerModeCmd, byte operatingModeCmd, byte fanSpeedCmd, byte temperatureCmd, byte swingVCmd, byte swingHCmd)
{
  // Sensible defaults for the heat pump mode

  byte powerMode = SAMSUNG_AIRCON1_MODE_ON;
  byte operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
  byte fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
  byte temperature = 23;

  if (powerModeCmd == POWER_OFF)
  {
    powerMode = SAMSUNG_AIRCON1_MODE_OFF;
  }
  else
  {
    switch (operatingModeCmd)
    {
      case MODE_AUTO:
        operatingMode = SAMSUNG_AIRCON1_MODE_AUTO;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in AUTO mode
        break;
      case MODE_HEAT:
        operatingMode = SAMSUNG_AIRCON1_MODE_HEAT;
        break;
      case MODE_COOL:
        operatingMode = SAMSUNG_AIRCON1_MODE_COOL;
        break;
      case MODE_DRY:
        operatingMode = SAMSUNG_AIRCON1_MODE_DRY;
        fanSpeedCmd = FAN_AUTO; // Fan speed is always 'AUTO' in DRY mode
        break;
      case MODE_FAN:
        operatingMode = SAMSUNG_AIRCON1_MODE_FAN;
        if ( fanSpeedCmd == FAN_AUTO ) {
          fanSpeedCmd = FAN_1; // Fan speed cannot be 'AUTO' in FAN mode
        }
        break;
    }
  }

  switch (fanSpeedCmd)
  {
    case FAN_AUTO:
      fanSpeed = SAMSUNG_AIRCON1_FAN_AUTO;
      break;
    case FAN_1:
      fanSpeed = SAMSUNG_AIRCON1_FAN1;
      break;
    case FAN_2:
      fanSpeed = SAMSUNG_AIRCON1_FAN2;
      break;
    case FAN_3:
      fanSpeed = SAMSUNG_AIRCON1_FAN3;
      break;
  }

  if ( temperatureCmd > 15 && temperatureCmd < 28)
  {
    temperature = temperatureCmd;
  }

  sendSamsung(IR, powerMode, operatingMode, fanSpeed, temperature);
}

// Send the Samsung code

void SamsungHeatpumpIR::sendSamsung(IRSender& IR, byte powerMode, byte operatingMode, byte fanSpeed, byte temperature)
{

  byte SamsungTemplate[] = { 0x02, 0x92, 0x0F, 0x00, 0x00, 0x00, 0xF0,   // Header part
                             0x01, 0xD2, 0x0F, 0x00, 0x00, 0x00, 0x00,   // Always the same data on POWER messages
                             0x01, 0x00, 0xFE, 0x71, 0x00, 0x00, 0xF0 }; // The actual data is in this part, on bytes 14-20

  // Set the power mode on the template message
  SamsungTemplate[1] = powerMode;

  // Set the fan speed and the operating mode on the template message
  SamsungTemplate[19] = operatingMode | fanSpeed;

  // Set the temperature on the template message
  SamsungTemplate[18] = (temperature - 16) << 4;

  // Byte 15 has some meaning on the protocol, it's one of these: 0xB2, 0xC2, 0xD2 or 0xE2
  SamsungTemplate[15] = 0xE2;

  // 40 kHz PWM frequency
  IR.setFrequency(40);

  // Header
  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload header part
  for (int i=0; i<7; i++) {
    IR.sendIRByte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(SAMSUNG_AIRCON1_MSG_SPACE);

  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload power message part
  for (int i=7; i<14; i++) {
    IR.sendIRByte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // Pause + new header
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(SAMSUNG_AIRCON1_MSG_SPACE);

  IR.mark(SAMSUNG_AIRCON1_HDR_MARK);
  IR.space(SAMSUNG_AIRCON1_HDR_SPACE);

  // Payload data message part
  for (int i=14; i<21; i++) {
    IR.sendIRByte(SamsungTemplate[i], SAMSUNG_AIRCON1_BIT_MARK, SAMSUNG_AIRCON1_ZERO_SPACE, SAMSUNG_AIRCON1_ONE_SPACE);
  }

  // End mark
  IR.mark(SAMSUNG_AIRCON1_BIT_MARK);
  IR.space(0);
}