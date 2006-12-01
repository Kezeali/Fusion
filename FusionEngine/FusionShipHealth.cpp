
#include "FusionShipHealth.h"

#include "FusionNode.h"

namespace FusionEngine
{

	ShipHealthDrawable::ShipHealthDrawable()
	{
	}

	void ShipHealthDrawable::SetWidth(int width)
	{
		m_Width = width;
	}

	void ShipHealthDrawable::SetMax(int max)
	{
		if (max != 0)
			m_Max = max;
	}

	void ShipHealthDrawable::SetHealth(int value)
	{
		assert(value < m_Max);

		// Add health-lost particle effect
		if (value < m_Value)
		{
			//nothing atm, still have to work out how to run the particle system
			// with the split time from the Env update
		}

		m_Value = value;
	}

	void ShipHealthDrawable::Draw()
	{
		CL_Vector2 pos = m_ParentNode->GetPosition();

		float len = (float(m_Value)/float(m_Max)) * m_Width;
		CL_Rectf rect(pos.x, pos.y - 6, pos.x + len, pos.y + 1);

		CL_Display::fill_rect(rect, CL_Color::red);
	}

}