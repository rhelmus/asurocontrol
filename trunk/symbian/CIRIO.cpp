/*
 * CIRIO.cpp
 *
 *  Created on: 4-dec-2009
 *      Author: Rick
 */

#include "CIRIO.h"
#include "asuroqt.h"
#include "utils.h"

CIRIO::CIRIO(asuroqt *owner) : CActive(CActive::EPriorityStandard), SIRState(SIR_IDLE), asuroUI(owner), writeQueue(10)
{
	CActiveScheduler::Add(this);
}

CIRIO::~CIRIO()
{
	Cancel();
	commPort.Close();
	server.Close();
}

void CIRIO::doRead()
{
	const TInt timeout = 500000; // 0.5s
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
	
	int ind = 0;
	while(manchester[ind] != 'e')
	{
		if (manchester[ind] == '1' && manchester[ind+1] == '0')
			User::After(889);
		
		if (manchester[ind] == '1' && manchester[ind+1] == '1')
		{
			User::After(1778);
			ind++;
		}
		
		if (manchester[ind] == '0' && manchester[ind+1] == '1')
			shortBurst();
		
		if (manchester[ind] == '0' && manchester[ind+1] == '0')
		{
			longBurst();
			ind++;
		}
		
		if (manchester[ind] == '0' && manchester[ind+1] == 'e')
			shortBurst();
		
		ind++;
	}
	
	commPort.Config(portSettings);
	portSettings().iRate = EBps2400;
	portSettings().iDataBits = EData8;	
	User::LeaveIfError(commPort.SetConfig(portSettings));
}

void CIRIO::burst(TInt amount)
{
	TBuf8<22> buf; // HACK: 22 is max
	for (int i=0;i<amount;i++)
		buf.Append(0x5B);
	
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
		if ((iStatus == KErrTimedOut) && writeQueue.Count())
		{
			doSendRC5(writeQueue[0]);
			asuroUI->appendLogText("Send RC5: " + toQString(writeQueue[0]));
			writeQueue.Delete(0);
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
		if (readBuffer[0] == SIR_START)
		{
			SIRState = SIR_READING;
		}
		
		doRead();
		break;
	case SIR_READING:
		if (readBuffer[0] == SIR_END)
		{
			if (readByte.Length() == 8)
			{
				//asuroUI->appendLogText("Read byte: " + toQString(readByte));
				// Convert to real byte
				char b = 0;
				for (int i=0; i<8; i++)
				{
					if (readByte[i] == '1')
						b |= (1<<i);
				}
				asuroUI->parseIRByte(b);
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

void CIRIO::sendRC5(const TDesC &code)
{
	writeQueue.AppendL(code);
}
