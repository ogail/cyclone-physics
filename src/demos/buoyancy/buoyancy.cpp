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

#include <cstdarg>
#include <cstdlib>
#include <stdio.h>

using namespace cyclone;

enum Direction { Right, Left, Forward, Backward };

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

class Box
{
public:
	Particle particle;

	real side;

	Box(Vector3 position)
	{
		particle.setMass(10.0f); // 30 kg
		particle.setVelocity(Vector3());
		particle.setAcceleration(Vector3());
		particle.setDamping(0.9f);
		side = 1.0f;

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
		glutSolidCube(side);
		glPopMatrix();
	}

	real volume()
	{
		return (side * side * side);
	}
};

/**
 * The main demo class definition.
 */
class BuoyancyDemo : public Application
{
	static const int PlaneWidth = 10;

	static const int PlaneHight = 10;

	ParticleWorld world;

	Box ball;

	Vector3 upliftPosition;

	real side;

	PushForceGenerator pushForceGenerator;

	ParticleBuoyancy buoyancyForceGenerator;

	bool pause;

    /** Moves the particle. */
    void move(Direction dir);

public:
    /** Creates a new demo object. */
    BuoyancyDemo();
    ~BuoyancyDemo();

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
BuoyancyDemo::BuoyancyDemo()
	:
	ball(Vector3(PlaneWidth / 2, 5, PlaneHight / 2)),
	world(1),
	upliftPosition(PlaneWidth / 2, 0, PlaneHight / 2),
	pushForceGenerator(20, 0.5f),
	buoyancyForceGenerator(ball.side, ball.volume(), 0),
	pause(false)
{
    world.getParticles().push_back(&ball.particle);
	world.getForceRegistry().add(&ball.particle, new ParticleGravity(Vector3::GRAVITY));
	world.getForceRegistry().add(&ball.particle, &pushForceGenerator);
	world.getForceRegistry().add(&ball.particle, &buoyancyForceGenerator);
}

BuoyancyDemo::~BuoyancyDemo()
{
}

void BuoyancyDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* BuoyancyDemo::getTitle()
{
    return "Cyclone > Buoyancy Demo";
}

void BuoyancyDemo::move(Direction dir)
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

void BuoyancyDemo::update()
{
	if (pause)
	{
		return;
	}

	// Clear accumulators
	world.startFrame();

    // Find the duration of the last frame in seconds
    float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
    if (duration <= 0.0f) return;

	// Run the simulation
	world.runPhysics(duration);

    Application::update();
}

std::string format(char* format, ...)
{
	char szBuffer1[1024] = { 0 };

    va_list args;
    va_start(args, format);
    vsprintf_s(szBuffer1, format, args);
    va_end(args);

	return szBuffer1;
}

void BuoyancyDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(PlaneWidth + 10, 2, PlaneHight / 2.0,  PlaneWidth / 2.0, 0.0, PlaneHight / 2.0,  0.0, 1.0, 0.0);

    glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3d(0, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, 0);
	glVertex3d(0, 0, 0);
    glEnd();

	ball.render();

	glColor3f(1.0f, 1.0f, 0.0f);
	renderText(10.0f, 10.0f, format("acceleration: %s", ball.particle.getAcceleration().toString().c_str()).c_str());
	renderText(10.0f, 20.0f, format("velocity: %s", ball.particle.getVelocity().toString().c_str()).c_str());
	renderText(10.0f, 30.0f, format("position: %s", ball.particle.getPosition().toString().c_str()).c_str());
}

void BuoyancyDemo::key(unsigned char key)
{
    switch (key)
    {
	case 'a': move(Direction::Backward); break;
    case 'd': move(Direction::Forward); break;
    case 'w': move(Direction::Left); break;
    case 's': move(Direction::Right); break;
    case 'p': pause = !pause; break;
    }
}

/**
 * Called by the common demo uplift to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new BuoyancyDemo();
}