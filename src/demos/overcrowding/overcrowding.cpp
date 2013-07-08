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
class OvercrowdingForceGenerator : public ParticleForceGenerator
{
private:
	real crowdingRadius;
	ParticleWorld *world;
	ParticleAnchoredSpring spring;

public:
	OvercrowdingForceGenerator (ParticleWorld *world, real crowdingRadius, real springConstant):
		world(world),
		crowdingRadius(crowdingRadius),
		spring(nullptr, springConstant, crowdingRadius * 2.0)
	{
	}

	bool IsOvercrownding(Particle *p1, Particle* p2)
	{
		real dist = (p1->getPosition() - p2->getPosition()).magnitude();

		if (dist < crowdingRadius)
			return true;
		else
			return false;
	}

	void updateForce(Particle* particle, real duration)
    {
		for (unsigned i = 0; i < world->getParticles().size(); ++i)
		{
			if (particle->getPosition() != world->getParticles()[i]->getPosition() &&
				IsOvercrownding(particle,  world->getParticles()[i]))
			{
				Vector3 anchorPos = world->getParticles()[i]->getPosition();
				spring.setAnchor(&anchorPos);
				spring.updateForce(particle, duration);
			}
		}
    }
};

class Ball
{
public:
	static const unsigned DefaultParticleRadius = 20;

	Particle particle;

	real raduis;

	Ball()
	{
		particle.setMass(1.0f);
		particle.setPosition(Vector3());
		particle.setVelocity(Vector3());
		particle.setAcceleration(Vector3());
		particle.setDamping(1.0);
		raduis = real(DefaultParticleRadius);


		// Clear the force accumulators
		particle.clearAccumulator();
	}

	Ball(Vector3 position)
	{
		particle.setMass(1.0f);
		particle.setVelocity(Vector3());
		particle.setAcceleration(Vector3());
		particle.setDamping(0.5);
		raduis = 5.0f;

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
		glutSolidSphere(raduis, 20, 20);
		glPopMatrix();
	}
};

/**
 * The main demo class definition.
 */
class OvercrowdingDemo : public Application
{
	static const int WorldRadius = 250;

	static const unsigned ParticleCount = 2;
	static const unsigned ParticleVelocityMagnitudeMax = 1;
	static const unsigned ParticleVelocityMagnitudeMin = 1;

	ParticleWorld world;

	Ball particles[ParticleCount];
	OvercrowdingForceGenerator overcrowdingFgn;

    /** Moves the particle. */
    void move(Direction dir);

public:
    /** Creates a new demo object. */
    OvercrowdingDemo();
    ~OvercrowdingDemo();

	void setView()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, (double)width/(double)height, 1.0, 3000.0f);
		glMatrixMode(GL_MODELVIEW);
	}

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
OvercrowdingDemo::OvercrowdingDemo()
	:
	world(ParticleCount),
	overcrowdingFgn(&world, 25.0f, 200.0) 
{
	srand(time(NULL));

	for (int i = 0; i < ParticleCount; ++i)
	{
		Vector3 initialVelocity = Vector3::randomWithMagnitudeRange(ParticleVelocityMagnitudeMin, ParticleVelocityMagnitudeMax);
		particles[i].particle.setVelocity(initialVelocity.x, initialVelocity.y, initialVelocity.z);
		particles[i].particle.setPosition(Vector3(0.0f, 0.0f, 0.0f));
		world.getParticles().push_back(&particles[i].particle);
		world.getForceRegistry().add(&particles[i].particle, &overcrowdingFgn);
	}
}

OvercrowdingDemo::~OvercrowdingDemo()
{
}

void OvercrowdingDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* OvercrowdingDemo::getTitle()
{
    return "Cyclone > Overcrowding Demo";
}

void OvercrowdingDemo::move(Direction dir)
{

}

void OvercrowdingDemo::update()
{
	// Clear accumulators
	world.startFrame();

    // Find the duration of the last frame in seconds
    float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
    if (duration <= 0.0f) return;

	// Run the simulation
	world.runPhysics(duration);

    Application::update();

	for (unsigned i = 0; i < ParticleCount; ++i)
	{
		Particle& curParticle = particles[i].particle;
		real distFromWorldCenter = curParticle.getPosition().magnitude();

		if (distFromWorldCenter >= real(WorldRadius))
		{
			Vector3 oppositeVelocity = curParticle.getVelocity();
			oppositeVelocity.invert();
			curParticle.setVelocity(oppositeVelocity);
		}
	}
}

void OvercrowdingDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(WorldRadius * 1.5f, WorldRadius * 1.5f, 0, 0, 0, 0.0, 0.0, 1.0, 0.0);

	glColor3f(1.0f, 0.0, 0.0);
	glutWireSphere(WorldRadius, 20, 20);

	for (int i = 0; i < ParticleCount; ++i)
	{
		particles[i].render();
	}
}

void OvercrowdingDemo::key(unsigned char key)
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
    return new OvercrowdingDemo();
}