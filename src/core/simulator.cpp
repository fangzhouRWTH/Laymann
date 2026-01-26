#include "simulator.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>

namespace lmcore
{
    btVector3 convert(Vec3f vec)
    {
        return {vec.x(), vec.y(), vec.z()};
    }

    btQuaternion convert(Quaternion qua)
    {
        return {qua.x(), qua.y(), qua.z(), qua.w()};
    }

    Iso3f convert(btVector3 translation, btQuaternion rotation)
    {
        Iso3f iso = Iso3f::Identity();
        Quaternion q;
        q.x() = rotation.x();
        q.y() = rotation.y();
        q.z() = rotation.z();
        q.w() = rotation.w();
        Vec3f t;
        t.x() = translation.x();
        t.y() = translation.y();
        t.z() = translation.z();
        iso.rotate(q);
        iso.translate(t);

        return iso;
    }

    struct RegisteredObject
    {
        std::shared_ptr<btCollisionShape> collisionShape;
        std::shared_ptr<btDefaultMotionState> motionState;
        std::shared_ptr<btRigidBody> body;

        PhysicalState state;
    };

    struct ObjectPool
    {
        std::vector<bool> aliveObjs;
        std::vector<RegisteredObject> registered;

        PhysicalObjectHandle add(RegisteredObject obj)
        {
            auto h = registered.size();
            aliveObjs.push_back(true);
            registered.push_back(obj);
            return h;
        }
    };

    class Simulator::Impl
    {
    public:
        void Init()
        {
            mCollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
            mDispatcher = std::make_unique<btCollisionDispatcher>(mCollisionConfig.get());
            mBroadphase = std::make_unique<btDbvtBroadphase>();
            mSolver = std::make_unique<btSequentialImpulseConstraintSolver>();
            mWorld = std::make_unique<btDiscreteDynamicsWorld>(mDispatcher.get(), mBroadphase.get(), mSolver.get(), mCollisionConfig.get());

            mWorld->setGravity(btVector3(0.f, 0.f, -0.981f));
        }

        void Update(float deltaTime)
        {
            btScalar dt = deltaTime;
            mWorld->stepSimulation(dt, mSubSteps);

            auto size = mPool.aliveObjs.size();
            for (auto i = 0; i < size; i++)
            {
                if (!mPool.aliveObjs[i])
                    continue;

                auto &obj = mPool.registered[i];
                btTransform trans;
                // todo test sync
                obj.body->getMotionState()->getWorldTransform(trans);
                auto t = trans.getOrigin();
                auto r = trans.getRotation();

                auto iso = convert(t, r);
                obj.state.pose = iso;
            }
        }

        void Destroy()
        {
            for (int i = mWorld->getNumCollisionObjects() - 1; i >= 0; --i)
            {
                btCollisionObject *obj = mWorld->getCollisionObjectArray()[i];
                btRigidBody *body = btRigidBody::upcast(obj);

                mWorld->removeCollisionObject(obj);
            }
        }

        PhysicalObjectHandle RegisterPlan(Vec3f normal, Vec3f location)
        {
            RegisteredObject obj;
            obj.collisionShape = std::make_unique<btStaticPlaneShape>(btVector3(normal.x(), normal.y(), normal.z()), 0.f);
            btTransform t;
            t.setIdentity();
            t.setOrigin(btVector3(location.x(), location.y(), location.z()));
            btScalar mass = 0.f;
            btVector3 inertia(0.f, 0.f, 0.f);
            obj.motionState = std::make_shared<btDefaultMotionState>(t);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, obj.motionState.get(), obj.collisionShape.get(), inertia);
            obj.body = std::make_shared<btRigidBody>(rbInfo);

            mWorld->addRigidBody(obj.body.get());
            auto h = mPool.add(obj);
            return h;
        }

        PhysicalObjectHandle RegisterPhysicalObject(BBox boundingBox, Iso3f transform, bool isStatic)
        {
            RegisteredObject obj;
            obj.collisionShape = std::make_shared<btBoxShape>(convert(boundingBox.xyz));
            btTransform t;
            Vec3f trans = transform.translation();
            Mat3f rot = transform.rotation();
            Quaternion qua(rot);
            t.setIdentity();
            t.setOrigin(convert(trans));
            t.setRotation(convert(qua));
            btScalar mass = 1.f;
            btVector3 inertia(0.f, 0.f, 0.f);
            obj.collisionShape->calculateLocalInertia(mass, inertia);
            obj.motionState = std::make_shared<btDefaultMotionState>(t);
            btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, obj.motionState.get(), obj.collisionShape.get(), inertia);
            obj.body = std::make_shared<btRigidBody>(rbInfo);

            obj.body->setDamping(0.05f, 0.85f);
            mWorld->addRigidBody(obj.body.get());

            auto h = mPool.add(obj);
            return h;
        }

        PhysicalState GetPhysicalState(PhysicalObjectHandle handle)
        {
            if(handle >= mPool.aliveObjs.size()||!mPool.aliveObjs[handle])
                return {};
            
            return mPool.registered[handle].state;
        }

    private:
        std::unique_ptr<btDefaultCollisionConfiguration> mCollisionConfig;
        std::unique_ptr<btCollisionDispatcher> mDispatcher;
        std::unique_ptr<btBroadphaseInterface> mBroadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> mSolver;
        std::unique_ptr<btDiscreteDynamicsWorld> mWorld;

        uint32_t mSubSteps = 10u;

        ObjectPool mPool;
    };

    Simulator::Simulator() : impl(std::make_unique<Impl>())
    {
    }
    Simulator::~Simulator()
    {
    }
    void Simulator::Init()
    {
        impl->Init();
    }
    void Simulator::Update(float deltaTime)
    {
        impl->Update(deltaTime);
    }
    void Simulator::Destroy()
    {
        impl->Destroy();
    }
    PhysicalObjectHandle Simulator::RegisterPlan(Vec3f normal, Vec3f location)
    {
        return impl->RegisterPlan(normal, location);
    }
    PhysicalObjectHandle Simulator::RegisterPhysicalObject(BBox boundingBox, Iso3f transform, bool isStatic)
    {
        return impl->RegisterPhysicalObject(boundingBox, transform, isStatic);
    }
    PhysicalState Simulator::GetPhysicalState(PhysicalObjectHandle handle)
    {
        return impl->GetPhysicalState(handle);
    }
}