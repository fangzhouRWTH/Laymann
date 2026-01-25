#include <btBulletDynamicsCommon.h>
#include <cstdio>
#include <memory>

// 简单的 RAII 删除器（Bullet 里很多对象需要手动 delete）
template <typename T>
using bullet_ptr = std::unique_ptr<T>;

int main()
{
    // 1) 基础配置：碰撞配置 / 碰撞调度 / broadphase / 约束求解器 / 世界
    bullet_ptr<btDefaultCollisionConfiguration> collisionConfig(
        new btDefaultCollisionConfiguration());

    bullet_ptr<btCollisionDispatcher> dispatcher(
        new btCollisionDispatcher(collisionConfig.get()));

    bullet_ptr<btBroadphaseInterface> broadphase(
        new btDbvtBroadphase());

    bullet_ptr<btSequentialImpulseConstraintSolver> solver(
        new btSequentialImpulseConstraintSolver());

    bullet_ptr<btDiscreteDynamicsWorld> world(
        new btDiscreteDynamicsWorld(dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get()));

    world->setGravity(btVector3(0, -9.81f, 0));

    // 2) 创建碰撞形状（shape 通常复用，生命周期要覆盖整个 world）
    bullet_ptr<btCollisionShape> groundShape(new btStaticPlaneShape(btVector3(0, 1, 0), 0)); // y=0 的平面
    bullet_ptr<btCollisionShape> boxShape(new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)));     // 半边长

    // 3) 创建静态刚体（地面）
    {
        btTransform t;
        t.setIdentity();
        t.setOrigin(btVector3(0, 0, 0));

        btScalar mass = 0.0f; // mass=0 => static body
        btVector3 inertia(0, 0, 0);

        auto motionState = new btDefaultMotionState(t);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape.get(), inertia);
        btRigidBody* groundBody = new btRigidBody(rbInfo);

        world->addRigidBody(groundBody);
        // 注意：这里为了示例简洁，后面会统一手动释放 bodies/motionState
    }

    // 4) 创建动态刚体（盒子）
    btRigidBody* boxBody = nullptr;
    {
        btTransform t;
        t.setIdentity();
        t.setOrigin(btVector3(0, 5, 0)); // 初始高度 y=5

        btScalar mass = 1.0f; // dynamic body
        btVector3 inertia(0, 0, 0);
        boxShape->calculateLocalInertia(mass, inertia);

        auto motionState = new btDefaultMotionState(t);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, boxShape.get(), inertia);
        boxBody = new btRigidBody(rbInfo);

        // 可选：增加一点阻尼
        boxBody->setDamping(0.05f, 0.85f);

        world->addRigidBody(boxBody);
    }

    // 5) 运行模拟：60Hz，模拟 5 秒
    const btScalar dt = 1.0f / 60.0f;
    const int steps = 5 * 60;

    for (int i = 0; i < steps; ++i)
    {
        world->stepSimulation(dt, 10); // dt, maxSubSteps

        btTransform trans;
        boxBody->getMotionState()->getWorldTransform(trans);

        btVector3 p = trans.getOrigin();
        if (i % 10 == 0) {
            std::printf("step=%d  box y=%.3f\n", i, p.getY());
        }
    }

    // 6) 清理：先从 world 移除，再 delete body（Bullet 不会帮你 delete）
    // world->getNumCollisionObjects() 会包含刚体等 collision objects
    for (int i = world->getNumCollisionObjects() - 1; i >= 0; --i)
    {
        btCollisionObject* obj = world->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);

        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }

        world->removeCollisionObject(obj);
        delete obj;
    }

    return 0;
}
