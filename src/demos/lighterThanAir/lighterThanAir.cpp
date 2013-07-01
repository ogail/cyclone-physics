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

real calculateGasDenisty(real h, real M)
{
	// sea level standard atmospheric pressure p0 = 101.325 kPa
	const real p0 = 101.325;

	// sea level standard temperature T0 = 288.15 K
	const real T0 = 288.15; 

	// Earth-surface gravitational acceleration g = 9.80665 m/s2.
	const real g = real_abs(Vector3::GRAVITY.y);

	// temperature lapse rate L = 0.0065 K/m
	const real L = 0.0065;

	// ideal (universal) gas constant R = 8.31447 J/(mol·K)
	const real R = 8.31447;

	// Temperature at altitude h meters above sea level is approximated by the following formula (only valid inside the troposphere):
	real T = T0 - (L * h);

	// The pressure at altitude h is given by:
	real p = p0 * real_pow((1.0f - ((L * h) / T0)), ((g * M) / (R * L))); 

	// Density can then be calculated according to a molar form of the ideal gas law:
	real roh = (p * M) / (R * T);

	return roh;
}

real scale = 10;

real toWorldAltitude(real gameAltitude)
{
	return gameAltitude * scale;
}

real toGameAltitude(real worldAltitude)
{
	return worldAltitude / scale;
}

real calculateHeliumDenisty(real h)
{
	// molar mass of helium M = 0.004002602 kg/mol
	const real M = 0.004002602;

	return calculateGasDenisty(h, M);
}

real calculateAirDenisty(real h)
{
	// molar mass of dry air M = 0.0289644 kg/mol
	const real M = 0.0289644;

	return calculateGasDenisty(h, M);
}

/**
 * pushes particles.
 */
class HeliumGenerator : public ParticleForceGenerator
{
private:
	real volume;

public:
	real worldPosition;

	real airDensity;

	real airForce;

	HeliumGenerator(real volume) : volume(volume) {  }

	void updateForce(Particle* particle, real duration)
    {
		worldPosition = toWorldAltitude(particle->getPosition().y);
		airDensity = calculateAirDenisty(worldPosition);
		airForce = airDensity * volume;

		particle->addForce(Vector3(0, airForce, 0));
    }
};

class Box
{
public:
	Particle particle;

	real side;

	Box(Vector3 position)
	{
		side = 1.0f;
		particle.setVelocity(Vector3());
		particle.setAcceleration(Vector3());
		particle.setDamping(0.9f);
		real heliumDensity = calculateHeliumDenisty(0);
		real mass = heliumDensity * volume();
		particle.setMass(mass);

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
class HeliumDemo : public Application
{
	static const int PlaneWidth = 100;

	static const int PlaneHight = 100;

	static const int MaxGameAltitude = 100;

	static const int MaxWorldAltitude = 1000;

	ParticleWorld world;

	Box ball;

	Vector3 upliftPosition;

	real side;

	PushForceGenerator pushForceGenerator;

	HeliumGenerator heliumForceGenerator;

	bool pause;

    /** Moves the particle. */
    void move(Direction dir);

public:
    /** Creates a new demo object. */
    HeliumDemo();
    ~HeliumDemo();

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
HeliumDemo::HeliumDemo()
	:
	ball(Vector3(0, 10, 0)),
	world(1),
	upliftPosition(PlaneWidth / 2, 0, PlaneHight / 2),
	pushForceGenerator(20, 0.5f),
	heliumForceGenerator(ball.volume()),
	pause(false)
{
    world.getParticles().push_back(&ball.particle);
	world.getForceRegistry().add(&ball.particle, new ParticleGravity(Vector3::GRAVITY));
	world.getForceRegistry().add(&ball.particle, &pushForceGenerator);
	world.getForceRegistry().add(&ball.particle, &heliumForceGenerator);

	real density = calculateAirDenisty(0);
}

HeliumDemo::~HeliumDemo()
{
}

void HeliumDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* HeliumDemo::getTitle()
{
    return "Cyclone > Helium Demo";
}

void HeliumDemo::move(Direction dir)
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

void HeliumDemo::update()
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

void HeliumDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(PlaneWidth + 100, MaxGameAltitude + 50, PlaneHight / 2.0,  PlaneWidth / 2.0, 0.0, PlaneHight / 2.0,  0.0, 1.0, 0.0);

    glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3d(0, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, 0);
	glVertex3d(0, 0, 0);
    glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3d(0, MaxGameAltitude, PlaneHight);
	glVertex3d(PlaneWidth, MaxGameAltitude, PlaneHight);
	glVertex3d(PlaneWidth, MaxGameAltitude, 0);
	glVertex3d(0, MaxGameAltitude, 0);
	glEnd();

	ball.render();

	glColor3f(1.0f, 1.0f, 0.0f);
	renderText(10.0f, 10.0f, format("acceleration: %s", ball.particle.getAcceleration().toString().c_str()).c_str());
	renderText(10.0f, 20.0f, format("velocity: %s", ball.particle.getVelocity().toString().c_str()).c_str());
	renderText(10.0f, 30.0f, format("position: %s", ball.particle.getPosition().toString().c_str()).c_str());
	renderText(10.0f, 40.0f, format("world position: %f", heliumForceGenerator.worldPosition).c_str());
	renderText(10.0f, 50.0f, format("air density: %f", heliumForceGenerator.airDensity).c_str());
	renderText(10.0f, 60.0f, format("air force: %f", heliumForceGenerator.airForce).c_str());
	renderText(10.0f, 70.0f, format("mass: %f", ball.particle.getMass()).c_str());
	renderText(10.0f, 80.0f, format("heluim desnity: %f", calculateHeliumDenisty(0)).c_str());
}

void HeliumDemo::key(unsigned char key)
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
    return new HeliumDemo();
}