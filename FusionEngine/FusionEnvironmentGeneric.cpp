
#include "FusionEnvironmentGeneric.h"

namespace FusionEngine
{

	void GenericEnvironment::DetonateProjectile(ObjectID index)
	{
		WeaponResourceBundle *rsc = 
			ResourceManager::getSingletonPtr()->GetWeaponResourceBundle(m_WeaponResources[index]);
	}

	const FusionShip* GenericEnvironment::GetShip(ObjectID index)
	{
		return m_Ships[index];
	}

	const FusionProjectile* GenericEnvironment::GetProjectile(ObjectID index)
	{
		return m_Projectiles[index];
	}

	void GenericEnvironment::limitFrames(unsigned int split)
	{
		// m_FrameTime is set to 1000/Options::MaxFPS
		if ((m_FrameTime-split) > 0)
			CL_System::sleep(m_FrameTime-split);
	}

	void GenericEnvironment::_abort(ErrorType type, const std::string& message)
	{
		throw Error(type, message);
		m_Abort = true;
	}

	void GenericEnvironment::installShipFrameFromMessage(FusionMessage *m)
	{
		ShipState state;

		RakNet::BitStream bs(m->ReadWithoutHeader(), m->GetDataLength(), false);
		// ReadWithoutHeader wont return a timestamp or other header info, so we don't worry about that

		bs.Read(state.PID);

		bs.Read(state.Position.x);
		bs.Read(state.Position.y);

		bs.Read(state.Velocity.x);
		bs.Read(state.Velocity.y);

		bs.Read(state.Rotation);
		bs.Read(state.RotationalVelocity);

		bs.Read(state.health);

		bs.Read(state.current_primary);
		bs.Read(state.current_secondary);
		bs.Read(state.current_bomb);

		bs.Read(state.engines);
		bs.Read(state.weapons);

		m_Ships[state.PID]->SetShipState(state);
	}

	void GenericEnvironment::installShipInputFromMessage(FusionMessage *m)
	{
		ShipInput state;

		RakNet::BitStream bs(m->ReadWithoutHeader(), m->GetLength(), false);

		// Data in Messages shouldn't have a timestamp anyway, so we don't worry about that
		bs.Read(state.pid);

		bs.Read(state.thrust);
		bs.Read(state.reverse);
		bs.Read(state.left);
		bs.Read(state.right);

		bs.Read(state.primary);
		bs.Read(state.secondary);
		bs.Read(state.bomb);

		m_Ships[state.pid]->SetInputState(state);
	}

	void GenericEnvironment::installProjectileFrameFromMessage(FusionMessage *m)
	{
		ProjectileState state;

		RakNet::BitStream bs(m->ReadWithoutHeader(), m->GetLength(), false);

		bs.Read(state.PID);

		bs.Read(state.OID);

		bs.Read(state.Position.x);
		bs.Read(state.Position.y);

		bs.Read(state.Velocity.x);
		bs.Read(state.Velocity.y);

		bs.Read(state.Rotation);
		bs.Read(state.RotationalVelocity);

		m_Projectiles[state.OID]->SetState(state);
	}

	void GenericEnvironment::updateAllPositions(unsigned int split)
	{
		m_PhysicsWorld->RunSimulation(split);

		// Update ships
		ShipList::iterator it = m_Ships.begin();
		for (; it != m_Ships.end(); ++it)
		{
			FusionPhysicsBody *body = (*it)->GetPhysicalBody();

			(*it)->SetVelocity(body->GetVelocity());
			(*it)->SetPosition(body->GetPosition());
			(*it)->SetRotationalVelocity(body->GetRotationalVelocity());
			(*it)->SetRotation(body->GetRotation());
		}
	}

}

/* [depreciated] See FusionPhysicsWorld
//Begin main server loop
for(int i=0; i<numPlayers; i++) //move and uncheck collision
{
m_Ships[i].CurrentState.Velocity.x += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * sin(m_Ships[i].CurrentState.Rotation);
m_Ships[i].CurrentState.Velocity.y += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * cos(m_Ships[i].CurrentState.Rotation);
}

for(int i=0; i<numPlayers; i++)
{
for(int j=0; j<numPlayers; i++)
{
if(i==j)
break;

//Ship Ship collision
if(sqrt((m_Ships[i].CurrentState.Position.x - m_Ships[j].CurrentState.Position.x)^2 + (m_Ships[i].CurrentState.Position.y - m_Ships[j].CurrentState.Position.y)^2)=<(m_Ships[i].CurrentState.Radius+m_Ships[j].CurrentState.Radius))
{
CL_Vector tempvelocity;
float temprotation;

temprotation = m_Ships[i].CurrentState.Rotation;
tempvelocity.x = m_ShipResource[i].Velocity.x;
tempvelocity.y = m_ShipResource[i].Velocity.y;

m_Ships[i].CurrentState.Rotation = m_Ships[j].CurrentState.Rotation;
m_Ships[i].CurrentState.Velocity.x = m_Ships[j].CurrentState.Velocity.x;
m_Ships[i].CurrentState.Velocity.y = m_Ships[j].CurrentState.Velocity.y;

m_Ships[j].CurrentState.Rotation = temprotation;
m_Ships[j].CurrentState.Velocity.x = tempvelocity.x;
m_Ships[j].CurrentState.Velocity.y = tempvelocity.y;

m_Ships[i].CurrentState.Velocity.x += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * sin(m_Ships[i].CurrentState.Rotation);
m_Ships[i].CurrentState.Velocity.y += m_Ships[i].CurrentState.ShipMass / m_Ships[i].CurrentState.EngineForce * cos(m_Ships[i].CurrentState.Rotation);
m_Ships[j].CurrentState.Velocity.x += m_Ships[j].CurrentState.ShipMass / m_Ships[j].CurrentState.EngineForce * sin(m_Ships[j].CurrentState.Rotation);
m_Ships[j].CurrentState.Velocity.y += m_Ships[j].CurrentState.ShipMass / m_Ships[j].CurrentState.EngineForce * cos(m_Ships[j].CurrentState.Rotation);
}
}
}

//ship - projectile
//ship - terrain

//If hit terrain move back + damage -- MrCai: damage? 
//   Well, that should be an option... eventually :P
*/

//void GenericEnvironment::updateSceneGraph()
//{
//}

// IGNORE THE FOLLOWING CODE, the scene now draws everthing!
//
//void GenericEnvironment::drawLevel()
//{
//}

//void GenericEnvironment::drawShip(FusionShip ship)
//{

//
//	ShipResource *res = m_ShipResources[ship.ResourceID];
//	Node
//
//	res->images.Body.GetImage()->draw(
//		positions.Body.x,
//		positions.Body.y);
//
//	res->images.LeftEngine->draw(
//		positions.LeftEngine.x,
//		positions.LeftEngine.y);
//
//	res->images.RightEngine->draw(
//		positions.RightEngine.x,
//		positions.RightEngine.y);
//
//	res->images.PrimaryWeapon->draw(
//		positions.PrimaryWeapon.x,
//		positions.PrimaryWeapon.y);
//
//	res->images.SecondaryWeapon->draw(
//		positions.SecondaryWeapon.x,
//		positions.SecondaryWeapon.y);
//
//    ;
//}
