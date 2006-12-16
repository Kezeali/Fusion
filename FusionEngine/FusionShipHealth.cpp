
#include "FusionShipHealth.h"

#include "FusionNode.h"

namespace FusionEngine
{

	ShipHealthDrawable::ShipHealthDrawable()
	{
		// Create a motion controller to control fancy "gravity" motion
		L_MotionController motionCtrl;
		motionCtrl.set_1d_acceleration(-0.2f);
		m_ParticleType = new L_Particle(m_Image, 3);
		m_ParticleType->set_motion_controller(&motionCtrl);

		L_Vector up; up.set2(0.4, L_DEGREE_TO_RADIAN(-90));

		m_Effect = new L_ShootingEffect(
			m_ParentNode->GetPosition().x, m_ParentNode->GetPosition().y, // Start pos
			L_Vector(0.f, 1.f), // Direction
			1, // Minimum time between particle creation
			32); // Particles created per effect->trigger(); effect->run()
		m_Effect->set_life_distortion(2);
		m_Effect->set_angle_interval(L_DEGREE_TO_RADIAN(45));
		m_Effect->add(m_ParticleType);
	}

	void ShipHealthDrawable::SetWidth(int width)
	{
		m_Width = width;
		m_Effect->set_width_interval(width);
	}

	void ShipHealthDrawable::SetMax(int max)
	{
		if (max != 0)
			m_Max = max;
	}

	void ShipHealthDrawable::SetHealth(int value)
	{
		cl_assert(value < m_Max);

		// Add health-lost particle effect
		if (value < m_Value)
		{
			m_Effect->trigger();
		}

		m_Value = value;
	}

	void ShipHealthDrawable::Update(unsigned int split)
	{
		m_Effect->run(split);
	}

	void ShipHealthDrawable::Draw()
	{
		CL_Vector2 pos = m_ParentNode->GetPosition();

		float len = (float(m_Value)/float(m_Max)) * float(m_Width);
		CL_Rectf rect(pos.x, pos.y - 6, pos.x + len, pos.y + 1);

		CL_Display::fill_rect(rect, CL_Color::red);
	}

}