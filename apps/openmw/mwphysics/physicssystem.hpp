#ifndef GAME_MWWORLD_PHYSICSSYSTEM_H
#define GAME_MWWORLD_PHYSICSSYSTEM_H

#include <memory>

#include <OgreVector3.h>

#include <btBulletCollisionCommon.h>

#include <osg/ref_ptr>

#include "../mwworld/ptr.hpp"

namespace osg
{
    class Group;
}

namespace MWRender
{
    class DebugDrawer;
}

class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

namespace MWPhysics
{
    typedef std::vector<std::pair<MWWorld::Ptr,Ogre::Vector3> > PtrVelocityList;

    enum CollisionType {
        CollisionType_World = 1<<0,
        CollisionType_Actor = 1<<1,
        CollisionType_HeightMap = 1<<2,
        CollisionType_Projectile = 1<<4,
        CollisionType_Water = 1<<5
    };

    class HeightField;

    class PhysicsSystem
    {
        public:
            PhysicsSystem (osg::ref_ptr<osg::Group> parentNode);
            ~PhysicsSystem ();

            void enableWater(float height);
            void setWaterHeight(float height);
            void disableWater();

            void addObject (const MWWorld::Ptr& ptr, const std::string& mesh, bool placeable=false);

            void addActor (const MWWorld::Ptr& ptr, const std::string& mesh);

            void addHeightField (float* heights, int x, int y, float triSize, float sqrtVerts);

            void removeHeightField (int x, int y);

            bool toggleCollisionMode();

            void stepSimulation(float dt);

            std::vector<std::string> getCollisions(const MWWorld::Ptr &ptr, int collisionGroup, int collisionMask); ///< get handles this object collides with
            Ogre::Vector3 traceDown(const MWWorld::Ptr &ptr, float maxHeight);

            std::pair<std::string,Ogre::Vector3> getHitContact(const std::string &name,
                                                               const Ogre::Vector3 &origin,
                                                               const Ogre::Quaternion &orientation,
                                                               float queryDistance);

            // cast ray, return true if it hit something.
            bool castRay(const Ogre::Vector3& from, const Ogre::Vector3& to,bool ignoreHeightMap = false);

            std::pair<bool, Ogre::Vector3>
            castRay(const Ogre::Vector3 &orig, const Ogre::Vector3 &dir, float len);

            /// Queues velocity movement for a Ptr. If a Ptr is already queued, its velocity will
            /// be overwritten. Valid until the next call to applyQueuedMovement.
            void queueObjectMovement(const MWWorld::Ptr &ptr, const Ogre::Vector3 &velocity);

            /// Apply all queued movements, then clear the list.
            const PtrVelocityList& applyQueuedMovement(float dt);

            /// Clear the queued movements list without applying.
            void clearQueuedMovement();

            /// Return true if \a actor has been standing on \a object in this frame
            /// This will trigger whenever the object is directly below the actor.
            /// It doesn't matter if the actor is stationary or moving.
            bool isActorStandingOn(const MWWorld::Ptr& actor, const MWWorld::Ptr& object) const;

            /// Get the handle of all actors standing on \a object in this frame.
            void getActorsStandingOn(const MWWorld::Ptr& object, std::vector<std::string>& out) const;

            /// Return true if \a actor has collided with \a object in this frame.
            /// This will detect running into objects, but will not detect climbing stairs, stepping up a small object, etc.
            bool isActorCollidingWith(const MWWorld::Ptr& actor, const MWWorld::Ptr& object) const;

            /// Get the handle of all actors colliding with \a object in this frame.
            void getActorsCollidingWith(const MWWorld::Ptr& object, std::vector<std::string>& out) const;

            bool toggleDebugRendering();

        private:

            void updateWater();

            btBroadphaseInterface* mBroadphase;
            btDefaultCollisionConfiguration* mCollisionConfiguration;
            btSequentialImpulseConstraintSolver* mSolver;
            btCollisionDispatcher* mDispatcher;
            btDiscreteDynamicsWorld* mDynamicsWorld;

            typedef std::map<std::pair<int, int>, HeightField*> HeightFieldMap;
            HeightFieldMap mHeightFields;

            bool mDebugDrawEnabled;

            std::map<std::string, std::string> handleToMesh;

            // Tracks all movement collisions happening during a single frame. <actor handle, collided handle>
            // This will detect e.g. running against a vertical wall. It will not detect climbing up stairs,
            // stepping up small objects, etc.
            std::map<std::string, std::string> mCollisions;

            std::map<std::string, std::string> mStandingCollisions;

            PtrVelocityList mMovementQueue;
            PtrVelocityList mMovementResults;

            float mTimeAccum;

            float mWaterHeight;
            float mWaterEnabled;

            std::auto_ptr<btCollisionObject> mWaterCollisionObject;
            std::auto_ptr<btCollisionShape> mWaterCollisionShape;

            std::auto_ptr<MWRender::DebugDrawer> mDebugDrawer;

            osg::ref_ptr<osg::Group> mParentNode;

            PhysicsSystem (const PhysicsSystem&);
            PhysicsSystem& operator= (const PhysicsSystem&);
    };
}

#endif