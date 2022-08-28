#include "stdafx.h"

#include "physics/entity/simple_kinematic_entity.h"

hphys::SimpleKinematicEntity::SimpleKinematicEntity() :
    m_default_jump_strength(1.), m_max_step_up(0.), m_step_speed(1.)
{
    // Empty.
}

void hphys::SimpleKinematicEntity::init(btDynamicsWorld* world, const btVector3& dimensions, btScalar mass) {
    init(world, new btBoxShape(dimensions), mass);
}

void hphys::SimpleKinematicEntity::init(btDynamicsWorld* world, btScalar radius, btScalar height, btScalar mass) {
    init(world, new btCapsuleShape(radius, height), mass);
}

void hphys::SimpleKinematicEntity::init(btDynamicsWorld* world, btCollisionShape* shape, btScalar mass) {
    m_world = world;

    btQuaternion rotation;
    rotation.setEulerZYX(0.0f, 1.0f, 0.0f);

    btDefaultMotionState* motion_state = new btDefaultMotionState(btTransform(rotation));

    btVector3 inertia;
    shape->calculateLocalInertia(mass, inertia);

    btRigidBody::btRigidBodyConstructionInfo body_info = btRigidBody::btRigidBodyConstructionInfo(mass, motion_state, shape, inertia);
    body_info.m_restitution = 0.0f;
    // TODO(Matthew): figure this out, maybe we don't want any?
    body_info.m_friction = 10.0f;

    m_body = new btRigidBody(body_info);
    m_body->setAngularFactor(0.0f);

    m_world->addRigidBody(m_body);
}

void hphys::SimpleKinematicEntity::set_target_direction(const btQuaternion& direction) {
    // TODO(Matthew): I don't like this too much, should we not let the physics control constraints?
    //                Here we do something that manifestly can override those constraints requiring
    //                callers to manage some degree of those constraints (e.g. when anchored to a
    //                gravitational body we want to preserve "up").
    //                  But then, we want to allow for things like ragdolling. I think this is best
    //                  achieved though by turning off/changing constraints and applying an impulse.
    //                  Again, allowing for physics to handle constraints.
    btTransform trans = m_body->getWorldTransform();
    trans.setRotation(direction);
}

void hphys::SimpleKinematicEntity::set_target_velocity(const btVector3& velocity) {
    // TODO(Matthew): Do we like this function at all? We probably don't want to
    //                be setting velocity, or at least we want a simple API for
    //                the ideas of stepping forward/backward vs strafing vs
    //                turning.
    //                  This being a kinematic entity.
    //                      We could consider an IK entity, but not for now.
    set_step_speed(velocity.length());
}

void hphys::SimpleKinematicEntity::set_default_jump_strength(btScalar default_jump_strength) {
    m_default_jump_strength = default_jump_strength;
}

void hphys::SimpleKinematicEntity::set_max_step_up(btScalar max_step_up) {
    m_max_step_up = max_step_up;
}

void hphys::SimpleKinematicEntity::set_step_speed(btScalar step_speed) {
    m_step_speed = step_speed;
}

void hphys::SimpleKinematicEntity::step_forward() {
    // TODO(Matthew): What actually defines forward in their basis?
    //                  Just for sake of writing something assuming [0],
    //                  this is almost certainly incorrect.
    step_direction(m_body->getWorldTransform().getBasis()[0]);
}

void hphys::SimpleKinematicEntity::step_backward() {
    // TODO(Matthew): What actually defines forward in their basis?
    //                  Just for sake of writing something assuming [0],
    //                  this is almost certainly incorrect.
    step_direction(-1. * m_body->getWorldTransform().getBasis()[0]);
}

void hphys::SimpleKinematicEntity::strafe_left() {
    // TODO(Matthew): What actually defines forward in their basis?
    //                  Just for sake of writing something assuming [0],
    //                  this is almost certainly incorrect.
    btVector3 left = btVector3(0., 1., 0.).cross(m_body->getWorldTransform().getBasis()[0]);
    step_direction(left);
}

void hphys::SimpleKinematicEntity::strafe_right() {
    // TODO(Matthew): What actually defines forward in their basis?
    //                  Just for sake of writing something assuming [0],
    //                  this is almost certainly incorrect.
    btVector3 left = btVector3(0., 1., 0.).cross(m_body->getWorldTransform().getBasis()[0]);
    step_direction(-1. * left);
}

void hphys::SimpleKinematicEntity::step_direction(const btVector3&) {

}

void hphys::SimpleKinematicEntity::jump(btScalar) {

}

void hphys::SimpleKinematicEntity::apply_impulse(const btVector3&) {

}

void hphys::SimpleKinematicEntity::updateAction(btCollisionWorld*, btScalar) {

}

void hphys::SimpleKinematicEntity::debugDraw(btIDebugDraw*) {

}
