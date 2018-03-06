#pragma once
#include "..\stdafx.h"

/*Basics*/
#include "Basic\Basic.h"
#include "Basic\FFmpeg.h"
#include "Basic\Functions.h"
#include "Basic\Timer.h"
#include "Basic\Jitter.h"

/*Interfaces*/
#include "Interfaces\Caller.h"
#include "Interfaces\Communication.h"
#include "Interfaces\Config.h"
#include "Interfaces\Socket.h"
#include "Interfaces\Logs.h"

/*Protocols*/
#include "Protocols\DTMF.h"
#include "Protocols\IPL.h"
#include "Protocols\MGCP.h"
#include "Protocols\RTP.h"
#include "Protocols\SDP.h"
#include "Protocols\SIP.h"