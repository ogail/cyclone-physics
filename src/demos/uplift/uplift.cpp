/*
 * The uplifts demo.
 *
 * Part of the Cyclone physics system.
 *
 * Copyright (c) Icosagon 2003. All Rights Reserved.
 *
 * This software is distributed under licence. Use of this software
 * implies agreement with all terms and conditions of the accompanying
 * software licence.
 */

#include <gl/glut.h>
#include <cyclone/cyclone.h>
#include "../app.h"
#include "../timing.h"

#include <stdio.h>

using namespace cyclone;

enum Direction { Right, Left, Forward, Backward };

/**
 * Uplifts are particles, with additional data for rendering and
 * evolution.
 */
class UpliftForceGenerator : public ParticleForceGenerator
{
private:
	unsigned forceAmount;

	Vector3 position;

	real radius;

public:
	UpliftForceGenerator(unsigned amount, Vector3 position, real radius):
		forceAmount(amount),
		position(position),
		radius(radius)
	{
		printf("force generated");
	}

	void updateForce(Particle* particle, real duration)
    {
		Vector3 temp = particle->getPosition();
		temp.y = position.y;
		Vector3 distance = temp - position;

		if (distance.magnitude() <= radius)
		{
			particle->addForce(Vector3(0.0f, forceAmount, 0.0f));
		}
    }
};

/**
 * pushes particles.
 */
class PushForceGenerator : public ParticleForceGenerator
{
private:
	unsigned forceAmount;

	Vector3 direction;

	real totalDuration;

	real remaining;

public:
	PushForceGenerator(unsigned amount, real totalDuration):
		forceAmount(amount),
		direction(Vector3()),
		totalDuration(totalDuration),
		remaining(0.0f)
	{
		printf("force generated");
	}

	void push(Vector3 direction)
	{
		this->direction = direction;
		remaining = totalDuration;
	}

	void updateForce(Particle* particle, real duration)
    {
		if (remaining > 0.0f)
		{
			particle->addForce(direction * forceAmount);
			remaining -= duration;
		}
    }
};

class Ball
{
public:
	Particle particle;

	Ball(Vector3 position)
	{
		particle.setMass(1.0f);
		particle.setVelocity(Vector3());
		particle.setAcceleration(Vector3());
		particle.setDamping(0.5f);

		particle.setPosition(position);
	
		// Clear the force accumulators
		particle.clearAccumulator();
	}

	/** Draws the box, excluding its shadow. */
	void render()
	{
		Vector3 position;
		particle.getPosition(&position);

		glColor3f(0.75, 0.75, 0.75);
		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);
		glutSolidSphere(10.0f, 20, 20);
		glPopMatrix();

		glColor3f(0.5, 0.5, 0.5);
		glPushMatrix();
		glTranslatef(position.x, 2.0f, position.z);
		glScalef(1.0f, 0.1f, 1.0f);
		glutSolidSphere(10.0f, 20, 20);
		glPopMatrix();
	}
};

/**
 * The main demo class definition.
 */
class UpliftDemo : public Application
{
	static const int PlaneWidth = 300;

	static const int PlaneHight = 300;

	ParticleWorld world;

	Ball ball;

	Vector3 upliftPosition;

	real radius;

	PushForceGenerator pushForceGenerator;

    /** Moves the particle. */
    void move(Direction dir);

public:
    /** Creates a new demo object. */
    UpliftDemo();
    ~UpliftDemo();

    /** Sets up the graphic rendering. */
    virtual void initGraphics();

    /** Returns the window title for the demo. */
    virtual const char* getTitle();

    /** Update the particle positions. */
    virtual void update();

    /** Display the particle positions. */
    virtual void display();

    /** Handle a keypress. */
    virtual void key(unsigned char key);
};

// Method definitions
UpliftDemo::UpliftDemo()
	:
	ball(Vector3(PlaneWidth / 2, 75, 50)),
	world(1),
	upliftPosition(PlaneWidth / 2, 0, PlaneHight / 2),
	radius(40.0f),
	pushForceGenerator(50, 0.5f)
{
    world.getParticles().push_back(&ball.particle);
	world.getForceRegistry().add(&ball.particle, new ParticleGravity(Vector3::GRAVITY));
	world.getForceRegistry().add(&ball.particle, new UpliftForceGenerator(50.0f, upliftPosition, radius));
	world.getForceRegistry().add(&ball.particle, &pushForceGenerator);
}

UpliftDemo::~UpliftDemo()
{
}

void UpliftDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* UpliftDemo::getTitle()
{
    return "Cyclone > Uplift Demo";
}

void UpliftDemo::move(Direction dir)
{
    switch (dir)
    {
	case Direction::Backward:
		pushForceGenerator.push(Vector3(0.0f, 0.0f, 1.0f));
    	break;
	case Direction::Forward:
		pushForceGenerator.push(Vector3(0.0f, 0.0f, -1.0f));
		break;
	case Direction::Left:
		pushForceGenerator.push(Vector3(-1.0f, 0.0f, 0.0f));
		break;
	case Direction::Right:
		pushForceGenerator.push(Vector3(1.0f, 0.0f, 0.0f));
		break;
    }
    
}

void UpliftDemo::update()
{
	// Clear accumulators
	world.startFrame();

    // Find the duration of the last frame in seconds
    float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
    if (duration <= 0.0f) return;

	// Run the simulation
	world.runPhysics(duration);

    Application::update();
}

void UpliftDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(PlaneWidth, 150, PlaneHight / 2.0,  PlaneWidth / 2.0, 0.0, PlaneHight / 2.0,  0.0, 1.0, 0.0);

    glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3d(0, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, 0);
	glVertex3d(0, 0, 0);
    glEnd();

	// Draw the ball
	glColor3f(1.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(upliftPosition.x, upliftPosition.y, upliftPosition.z);
	glScalef(1.0f, 0.1f, 1.0f);
	glutSolidSphere(radius, 20, 20);
	glPopMatrix();

	ball.render();
}

void UpliftDemo::key(unsigned char key)
{
    switch (key)
    {
	case 'a': move(Direction::Backward); break;
    case 'd': move(Direction::Forward); break;
    case 'w': move(Direction::Left); break;
    case 's': move(Direction::Right); break;
    }
}

/**
 * Called by the common demo uplift to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new UpliftDemo();
}