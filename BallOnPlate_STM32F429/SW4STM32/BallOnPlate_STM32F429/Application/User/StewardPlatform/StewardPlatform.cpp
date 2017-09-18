/*
 * Master.cpp
 *
 *  Created on: 16 wrz 2017
 *      Author: Peter
 */

#include "StewardPlatform.h"
#include <Communicator/Command/CmdType.h>



/**
 *
 */
StewardPlatform::StewardPlatform() {

	this->Construct();

	Mode = NULL;
	CommunicationCenter.Bluetooth.begin();

}
/********************************************************/



/**
 *
 */
StewardPlatform::~StewardPlatform() {
	delete Mode;

	osThreadTerminate(rxTaskHandle);
	osThreadTerminate(txTaskHandle);
	osThreadTerminate(touchPanelTaskHandle);
}



/**
 *
 * @param modeType
 */
void StewardPlatform::SetMode(ModeType_e modeType) {
	if(Mode){
		delete Mode;
		Mode = NULL;
	}

	switch(modeType){

	case pidMode:
		Mode = new PIDMode(this);
		break;

	default:
		break;
	}

}
/********************************************************/



/**
 *
 */
void StewardPlatform::Construct() {
	/* definition and creation of rxTask */
	osThreadDef(StewardPlatformRxTask, RxTask, osPriorityRealtime, 0, 128);
	rxTaskHandle = osThreadCreate(osThread(StewardPlatformRxTask), &CommunicationCenter);

	/* definition and creation of txTask */
	osThreadDef(StewardPlatformTxTask, TxTask, osPriorityRealtime, 0, 128);
	txTaskHandle = osThreadCreate(osThread(StewardPlatformTxTask), &CommunicationCenter);

	/* definition and creation of touchPanelTask */
	osThreadDef(StewardPlatformTouchPanelTask, TouchPanelTask, osPriorityAboveNormal, 0, 256);
	touchPanelTaskHandle = osThreadCreate(osThread(StewardPlatformTouchPanelTask), &TouchPanel);

	/* definition and creation of communicationTask */
	osThreadDef(StewardPlatformCommunicationTask, CommunicationTask, osPriorityBelowNormal, 0, 512);
	communicationTaskHandle = osThreadCreate(osThread(StewardPlatformCommunicationTask), this);

}
/********************************************************/



/**
 *
 * @param cmd
 */
void StewardPlatform::Execute(Command cmd) {

	switch(cmd.getType()){
	case empty:
		CommunicationCenter.SendEmpty();
		break;

	case fail:
		break;

	case ok:
		break;

	case startMode:
		if(Mode) Mode->Start();
			else CommunicationCenter.SendFail();
		break;

	case stopMode:
		if(Mode) Mode->Stop();
			else CommunicationCenter.SendFail();
		break;

	case resetMode:
		if(Mode) Mode->Reset();
			else CommunicationCenter.SendFail();
		break;

	case setMode:
		SetMode((ModeType_e)cmd.getParam());
		break;

	default:
		if(Mode) Mode->Execute(cmd);
			else CommunicationCenter.SendFail();
		break;

	}
}
/********************************************************/



/**
 *
 * @param argument
 */
void StewardPlatform::TouchPanelTask(const void* argument) {
	PlatformTouchPanel* touchPanel;

	touchPanel = (PlatformTouchPanel*) argument;
	touchPanel->TouchPanelTask(NULL);
}
/********************************************************/



/**
 *
 * @param argument
 */
void StewardPlatform::TxTask(const void* argument) {
	PlatformCommunicator* communicationCenter;

	communicationCenter = (PlatformCommunicator*) argument;
	communicationCenter->TxTask(NULL);
}
/********************************************************/



/**
 *
 * @param argument
 */
void StewardPlatform::RxTask(const void* argument) {
	PlatformCommunicator* communicationCenter;

	communicationCenter = (PlatformCommunicator*) argument;
	communicationCenter->RxTask(NULL);
}
/********************************************************/



/**
 *
 * @param argument
 */
void StewardPlatform::CommunicationTask(const void* argument) {
	StewardPlatform* stewardPlatform = (StewardPlatform*)argument;
	PlatformCommunicator* communicationCenter = &stewardPlatform->CommunicationCenter;


	while(true){
		bool isCommand = false;

		Command cmd = communicationCenter->ReceiveCommmand(&isCommand);
		if(isCommand){

			communicationCenter->SendCommand(cmd);
			stewardPlatform->Execute(cmd);
		}else
		osDelay(30);

	}
}
/********************************************************/



/**
 *
 * @param huart
 */
void StewardPlatform::UART_RxCpltCallback(UART_HandleTypeDef* huart) {
	CommunicationCenter.UARTRxCpltCallback(huart);
}
/********************************************************/



/**
 *
 * @param huart
 */
void StewardPlatform::UART_TxCpltCallback(UART_HandleTypeDef* huart) {
	CommunicationCenter.UARTTxCpltCallback(huart);
}
/********************************************************/
