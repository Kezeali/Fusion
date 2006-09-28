
#include "FusionNetworkSender.h"

using namespace FusionEngine;

FusionNetworkSender::FusionNetworkSender(FusionNetworkMessageQueue &q)
: m_Queue(q)
{
}

FusionNetworkSender::~FusionNetworkSender()
{
}

FusionNetworkSender::run()
{
		while (true)
		{
			if (true)
			{
				//! Does all the work so I don't have to
				BitStream message();
			}

			CL_System::sleep(10);
		}
}