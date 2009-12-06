/*
 * CIRIO.h
 *
 *  Created on: 4-dec-2009
 *      Author: Rick
 */

#ifndef CIRIO_H_
#define CIRIO_H_

#include <e32base.h>
#include <c32comm.h>
#include <badesca.h>

class asuroqt;

/* SIR Codes:
 * 1: 0xBD	<-- Start
 * 2: 0xBB	<-- End
 * 3: 0xBB
 * 4: 0xB7	<-- One
 * 5: 0xB5	<-- Zero
 * 6: 0xB7
 * 7: 0xB7
 * 8: 0xBF
 * 9: 0xBD
 * 0: 0xBF
 */

class CIRIO: public CActive
{
	virtual void RunL();
	virtual void DoCancel();
	
	enum ESIRState { SIR_IDLE, SIR_WAIT, SIR_READING };
	ESIRState SIRState;
	
	// See above comments
	enum ESIRCode { SIR_START = 0xBD, SIR_ONE = 0xB7, SIR_ZERO = 0xB5, SIR_END = 0xBB };
	
	asuroqt *asuroUI;
	
	RCommServ server;
	RComm commPort;
	
	TBuf8<1> readBuffer;
	TBuf<8> readByte;
	CDesCArraySeg writeQueue;
	
	void doRead(void);
	void doSendRC5(const TDesC &code);
	
	void burst(TInt amount);
	void shortBurst(void) { burst(11); }
	void longBurst(void) { burst(22); }
	
protected:
	CIRIO(asuroqt *owner);
	
	void ConstructL(void);
	
public:
	~CIRIO(void);
	
	static CIRIO *NewL(asuroqt *owner);
	static CIRIO *NewLC(asuroqt *owner);
	
	void Start(void);
	void sendRC5(const TDesC &code);
};

#endif /* CIRIO_H_ */
