#include <DAVE.h>

#define LIGHT_PRESCALER 35
#define PWM_PRESCALER 100

#define DATA_MANUAL    0x01
#define DATA_AUTOMATIC 0x02
#define DATA_LIGHT_ON  0x03
#define DATA_LIGHT_OFF 0x04
#define DATA_INTENSITY 0x05

uint32_t measuredValue = 0;

uint32_t ledValue = 0;
uint32_t ledTargetValue = 0;

typedef enum{
	MANUAL,
	AUTOMATIC
} OperatingMode;

OperatingMode operatingMode;

uint8_t packetData[2];

void setLedState(uint32_t distance);
void mainLoop();
void readNextPacket();
void delayCretin();

void DistanceEventHandler(){
	// Check pin state
	if(PIN_INTERRUPT_GetPinValue(&ECHO_INTERRUPT)){
		// rising edge
		TIMER_Clear(&TIMER_0);
		TIMER_Start(&TIMER_0);
	}else{
		// falling edge
		measuredValue = TIMER_GetTime(&TIMER_0);
		setLedState(measuredValue);
		TIMER_Stop(&TIMER_0);
	}
}

uint8_t toggled = 0;

void setLedState(uint32_t distance){
	if(distance < 100000 && !toggled){
		if(operatingMode == AUTOMATIC){
			if(ledTargetValue){
				ledTargetValue = 0;
			}else{
				ledTargetValue = 100 * PWM_PRESCALER * LIGHT_PRESCALER;
			}
		}
		toggled = 1;
	}

	if(distance > 130000){
		toggled = 0;
	}
}

void setLedPercentage(uint32_t percentage){
	if(percentage <= 100){
		ledTargetValue = percentage * PWM_PRESCALER * LIGHT_PRESCALER;
	}
}

int main(void){
	DAVE_STATUS_t status;

	status = DAVE_Init();

	if(status != DAVE_STATUS_SUCCESS){
	  XMC_DEBUG("DAVE APPs initialization failed\n");
	}

	operatingMode = MANUAL;

	UART_Receive(&UART_0, packetData, 2);

	// Main loop
	while(1U){
		mainLoop();
//		readNextPacket();
//		delayCretin();
	}
}

void mainLoop(){
	PWM_SetDutyCycle(&LED_PWM, ledValue/LIGHT_PRESCALER);
	if(ledValue > ledTargetValue){
		--ledValue;
	}
	if(ledValue < ledTargetValue){
		++ledValue;
	}
}


void readNextPacket(){
	// 2-byte packets
	// First byte is packet id
	// Second byte is additional data; ignore if not needed

	// Here we have some valid data
	uint8_t packetType = packetData[0];

	switch(packetType){
		case DATA_MANUAL:
			operatingMode = MANUAL;
			break;
		case DATA_AUTOMATIC:
			operatingMode = AUTOMATIC;
			break;
		case DATA_LIGHT_ON:
			if(operatingMode == MANUAL){
				ledTargetValue = 100 * PWM_PRESCALER * LIGHT_PRESCALER;
			}
			break;
		case DATA_LIGHT_OFF:
			if(operatingMode == MANUAL){
				ledTargetValue = 0;
			}
			break;
		case DATA_INTENSITY:
			if(operatingMode == MANUAL){
				uint8_t intensityValue = packetData[1];
				ledTargetValue = intensityValue * PWM_PRESCALER * LIGHT_PRESCALER;
			}
			break;
	}
	UART_Receive(&UART_0, packetData, 2);
}

void delayCretin(){
	for(int i = 0; i < 0xFFFF; ++i){
		++i;
		--i;
		for(int j = 0; j < 0xFF; ++j){
			++j;
			--j;
		}
	}
}

