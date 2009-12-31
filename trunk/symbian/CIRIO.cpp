/*
 * CIRIO.cpp
 *
 *  Created on: 4-dec-2009
 *      Author: Rick
 */

#include <e32std.h>
#include <hal.h>
#include <hal_data.h>

#include "../shared/shared.h"
#include "CIRIO.h"
#include "asuroqt.h"
#include "utils.h"

CIRIO::CIRIO(asuroqt *owner) : CActive(CActive::EPriorityStandard), SIRState(SIR_IDLE), asuroUI(owner), writeQueue(10),
							   pulseCode(0x5B), sendUpdate(true)
{
	CActiveScheduler::Add(this);
}

CIRIO::~CIRIO()
{
	Cancel();
	commPort.Close();
	server.Close();
	writeQueue.Reset();
}

void CIRIO::doRead()
{
	const TInt timeout = 150000; // 0.15s
	commPort.Read(iStatus, timeout, readBuffer, 1);
	if (!IsActive())
		SetActive();
}

void CIRIO::doSendRC5(const TDesC &code)
{
	TCommConfig portSettings;
	commPort.Config(portSettings);
	portSettings().iRate = EBps115200;
	portSettings().iDataBits = EData7;	
	User::LeaveIfError(commPort.SetConfig(portSettings));

	TBuf8<29> manchester; // HACK: Assume max is 29
	const TInt len = code.Length();
	
	for (TInt i=0; i<len; i++)
	{
		if (code[i] == '0')
		{
			manchester.Append('0');
			manchester.Append('1');
		}
		else
		{
			manchester.Append('1');
			manchester.Append('0');
		}
	}
	
	manchester.Append('e');

	/*QString qs;
	for (int i=0;i<29;i++)
		qs += manchester[i];
	asuroUI->appendLogText("Send RC5(m): " + qs + "\n");*/
	
	int ind = 0;
	QStringList sl;
	TInt tperiod;
	
	HAL::Get(HALData::ENanoTickPeriod, tperiod);
	
	while(manchester[ind] != 'e')
	{	
		TUint32 ticks = User::NTickCount();
		if (manchester[ind] == '0')
			burst();
		else
			User::AfterHighRes(4000); // +1000 for some reason?!?
		
		sl << QString::number((User::NTickCount()-ticks)*tperiod);
				
		ind++;
	}
	
	//asuroUI->appendLogText(QString("write times: %1\n").arg(sl.join(", ")));
	
	commPort.Config(portSettings);
	portSettings().iRate = EBps2400;
	portSettings().iDataBits = EData8;	
	User::LeaveIfError(commPort.SetConfig(portSettings));
}

void CIRIO::burst()
{
	const int amount = 65; // This seem to give a 5 msec pulse
	TBuf8<amount> buf;
	for (int i=0; i<amount; i++)
		buf.Append(pulseCode);
	
	TRequestStatus status;
	commPort.Write(status, buf);
	User::WaitForRequest(status); // Not very clean, but we have to make sure of the right timing
}

CIRIO *CIRIO::NewL(asuroqt *owner)
{
	CIRIO* self = CIRIO::NewLC(owner);
	CleanupStack::Pop(self);
	return self;
}

CIRIO *CIRIO::NewLC(asuroqt *owner)
{
	CIRIO *self = new (ELeave) CIRIO(owner);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
}

void CIRIO::ConstructL()
{
	TInt r = User::LoadPhysicalDevice(_L("EUART1"));
	if (r != KErrNone && r != KErrAlreadyExists)
		User::Leave (r);
	r = User::LoadLogicalDevice(_L("ECOMM"));
	if (r != KErrNone && r != KErrAlreadyExists)
		User::Leave (r);
	
	User::LeaveIfError(server.Connect());

	r = server.LoadCommModule(_L("ECUART"));
	User::LeaveIfError(r);
	
	r = commPort.Open(server, _L("COMM::1"), ECommShared);
	User::LeaveIfError (r);

	TCommConfig portSettings;
	commPort.Config(portSettings);
	portSettings().iRate = EBps2400;
	portSettings().iParity = EParityNone;
	portSettings().iDataBits = EData8;
	portSettings().iStopBits = EStop1;
	portSettings().iSIREnable   = ESIREnable;
	portSettings().iSIRSettings = KConfigSIRPulseWidthMaximum;
	portSettings().iFifo = EFifoEnable;
	portSettings().iHandshake = 0;
		
	r = commPort.SetConfig (portSettings);
	User::LeaveIfError (r);

	commPort.SetReceiveBufferLength(4096);
	
	// Null read // UNDONE: Transfer to state?
	TBuf8<512> buf;
	TRequestStatus rs; // UNDONE
	commPort.Read(rs, 1000000, buf, 0);
	User::WaitForRequest(rs);
}

void CIRIO::RunL()
{
	if (iStatus != KErrNone)
	{
		// Something went wrong or timeout, reset
		
		// Timeout: check if we can do some writing in the meanwhile
		if (iStatus == KErrTimedOut)
		{
			/*
			doSendRC5(writeQueue[0]);
			//asuroUI->appendLogText("Send RC5(1): " + toQString(writeQueue[0]) + "\n");
			writeQueue.Remove(0);*/
			
			if (sendUpdate)
				sendIR(CMD_UPDATE, 0); // Always do as last! (call appends it to queue)
			
			sendUpdate = !sendUpdate; // Only update half of the time
			
			int count = writeQueue.Count();
			for (int i=0; i<count; i++)
			{
				doSendRC5(writeQueue[i]);
				User::AfterHighRes(35000); // Wait before next msg
			}
			
			writeQueue.Reset();
		}
		
		Start();			
		return;
	}
	
	switch (SIRState)
	{
	case SIR_IDLE:
		break;
	case SIR_WAIT:
		// Got data
		//asuroUI->appendLogText(QString("%1 ").arg((int)readBuffer[0], 0, 16));
		if (readBuffer[0] == SIR_START)
		{
			SIRState = SIR_READING;
		}
		
		doRead();
		break;
	case SIR_READING:
		//asuroUI->appendLogText(QString("%1 ").arg((int)readBuffer[0], 0, 16));
		if (readBuffer[0] == SIR_END)
		{
			if (readByte.Length() == 8)
			{
				//asuroUI->appendLogText("Read byte: " + toQString(readByte));
				// Convert to real byte
				unsigned char b = 0;
				for (int i=0; i<8; i++)
				{
					if (readByte[i] == '1')
						b |= (1<<i);
				}
				//asuroUI->parseIRByte(b);
				asuroUI->IRByte(b);
			}
			Start();
			break;
		}
		else if (readBuffer[0] == SIR_START)
			; // Nothing (shouldn't happen)
		else
		{
			if (readByte.Length() < 8)
			{
				if (readBuffer[0] == SIR_ONE)
					readByte.Append('1');
				else if (readBuffer[0] == SIR_ZERO)
					readByte.Append('0');
			}
		}		
		doRead();
		break;
	}
}

void CIRIO::DoCancel()
{
	commPort.ReadCancel();
	readByte.Zero();
}

void CIRIO::Start()
{
	Cancel();
	SIRState = SIR_WAIT;
	readByte.Zero();
	doRead();
}

void CIRIO::sendIR(unsigned char cmd, unsigned char data)
{
	TBuf<14> bytecode;
	
	// Start byte
	bytecode.Append('1');	

	// Command
	for (int i=4; i>=0; i--)
	{
		if (cmd & (1<<i))
			bytecode.Append('1');
		else
			bytecode.Append('0');
	}
	
	// Data
	for (int i=7; i>=0; i--)
	{
		if (data & (1<<i))
			bytecode.Append('1');
		else
			bytecode.Append('0');
	}
	
	writeQueue.Append(bytecode);
}

void CIRIO::sendIR(const TDesC &code, char pulse)
{
	writeQueue.Append(code);
	pulseCode = pulse;
}
