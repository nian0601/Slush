#pragma once
#include <FW_Rect.h>
#include <FW_GrowingArray.h>
#include <FW_Vector2.h>
#include <FW_Matrix22.h>
#include <FW_Any.h>

#include "PhysicsShapes.h"

// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-core-engine--gamedev-7493
// https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-oriented-rigid-bodies--gamedev-8032?_ga=2.10454710.2040535734.1649969427-1859424014.1649969427
// https://github.com/tutsplus/ImpulseEngine

// Goes over constraints in p3
// https://www.toptal.com/game/video-game-physics-part-i-an-introduction-to-rigid-body-dynamics

namespace Slush
{
	struct PhysicsObject
	{
		PhysicsObject(PhysicsShape* aShape);
		~PhysicsObject();

		Vector2f myPosition; // += myVelocity * deltaTime
		Vector2f myVelocity; // += myAcceleration
		Vector2f myAcceleration; // myForces / myMass
		Vector2f myForces; // Cleared every frame

		float myOrientation = 0.f;
		float myAngularVelocity = 0.f;
		float myTorque = 0.f;

		float myRestitution = 0.7f; //Bouncyness, 0 == BeanBag, 1 == SuperBall

		float myMass = 1.f;
		float myInvMass = 1.f;

		float myInertia = 1.f;
		float myInvInertia = 1.f;

		float myStaticFriction = 0.5f;
		float myDynamicFriction = 0.25f;

		int myColor = 0xFFFFFFFF;

		unsigned int myCollisionMask = 0;
		unsigned int myCollidesWithMask = 0;
		unsigned int myReportCollisionsWith = 0xffffffff;

		PhysicsShape* myShape = nullptr;

		void IntegrateForces(float aDelta);
		void IntegrateVelocity(float aDelta);

		void ApplyImpulse(const Vector2f& aImpulse, const Vector2f& aContactVector);

		void SetMass(float aMass);
		void SetInertia(float aIntertia);
		void SetDensity(float aDensity);

		void MakeStatic();
		void MakeSensor();

		void SetPosition(const Vector2f& aPosition);
		void SetOrientation(float aRadians);

		bool CollidesWith(PhysicsObject* aOther) const;
		bool ReportsCollisionsWith(PhysicsObject* aOther) const;

		Vector2f myPreviousPosition;
		Vector2f myPreviousVelocity;
		float myPreviousOrientation = 0.f;
		bool mySensorFlag = false;

		FW_Any myUserData;
	};

	struct Manifold
	{
		PhysicsObject* myObjectA = nullptr;
		PhysicsObject* myObjectB = nullptr;
		Vector2f myHitNormal;
		Vector2f myContacts[2];
		int myContactCount = 0;
		float myPenetrationDepth = 0.f;

		bool myNeedsToResolveCollision = true;
	};

	struct Contact
	{
		PhysicsObject* myFirst = nullptr;
		PhysicsObject* mySecond = nullptr;
	};

	struct MaxDistanceConstraint
	{
		PhysicsObject* myObjectA = nullptr;
		PhysicsObject* myObjectB = nullptr;
		float myMaxDistance = 0.f;

		void ResolveConstraint();
	};

	class PhysicsWorld
	{
	public:
		PhysicsWorld();
		~PhysicsWorld();

		void Tick();
		void TickLimited(float aDeltaTime);

		void RenderAllObjects();
		void RenderContacts();

		void ApplyForceInRadius(const Vector2f& aCenter, float aRadius, float aMinForce, float aMaxForce);

		void DeleteAllObjects();

		void AddObject(PhysicsObject* aObject);
		void AddConstraint(MaxDistanceConstraint* aConstraint);

		void RemoveObject(PhysicsObject* aObject);

		const FW_GrowingArray<PhysicsObject*>& GetObjects() const { return myObjects; }
		const FW_GrowingArray<Contact>& GetContacts() const { return myContacts; }
		float GetFixedDeltaTime() const { return myFixedDeltaTime; }

		static float ourGravityScale;
		static const Vector2f ourGravity;

	private:
		void ResolveCollision(const Manifold& aManifold);
		void PositionalCorrection(const Manifold& aManifold);

		float myFixedDeltaTime;
		FW_GrowingArray<Manifold> myManifolds;
		FW_GrowingArray<Contact> myContacts;
		FW_GrowingArray<PhysicsObject*> myObjects;
		FW_GrowingArray<MaxDistanceConstraint*> myMaxDistanceConstraints;

		PhysicsObject* myCircleObject;
		CircleShape* myCircleShape;
	};
}