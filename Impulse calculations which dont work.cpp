// Stupid hack that doesn't really work 
//  (but it gives a kinda funny 'gravity beam' effect :P)
//cBod->ApplyForce(norm);

//// Get the difference in position
//CL_Vector2 d_pos;

//if (m_Wrap)
//{
//	CL_Vector2 pos_a = cBod->GetPosition();
//	CL_Vector2 pos_b = Other->GetPosition();
//	float dx = MIN_DIFF(pos_a.x, pos_b.x, m_Width);
//	float dy = MIN_DIFF(pos_a.y, pos_b.y, m_Height);

//	d_pos.x = dx; d_pos.y = dy;
//}
//else
//{
//	CL_Vector2 d_pos = cBod->GetPosition() - Other->GetPosition();
//}

//// Get the difference in velocity
//CL_Vector2 d_v = cBod->GetVelocity() - Other->GetVelocity();

//// Get the length of the difference in positions.
//float distance = d_pos.length();

//if (distance == 0) return; // Can't handle this...

//// Get the inverse of half the distance, and multiply it by the dot product of the difference in position and the difference in velocity,
//// Put simply, impulse = 1/(distance/2.0) * (dpos)dot(dvel)
//float impulse = -2.0f / distance * d_pos.dot(d_v);

//// Convert the differences to ratios (normalise)
//d_pos /= d_v.length();

//// Adjust velocities for newtonian physics
//// Weird collision (should only happen in unusual circumstances)
//if (impulse < 0) return;

//// Get the impulse force ratio (impulse vector / total mass)
//impulse = impulse / (cBod->GetMass() + Other->GetMass());

//// Calculate the new velocities (impulse force * other mass * normal)
//CL_Vector2 i_veloc = norm * (impulse * Other->GetMass());
//cBod->m_Velocity = i_veloc;

//i_veloc = norm * (impulse * cBod->GetMass());
//Other->m_Velocity = i_veloc;