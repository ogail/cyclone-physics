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

/**
     * A force generator that applies a Spring force, where
     * one end is attached to a fixed point in space.
     */
    class ParticleDeformedAnchoredSpring : public ParticleAnchoredSpring
    {
    private:
        /** The maximum elasticity of the spring. */
        real springMaxDistance;

    public:
        ParticleDeformedAnchoredSpring();

        /** Creates a new spring with the given parameters. */
        ParticleDeformedAnchoredSpring(Vector3 *anchor,
                               real springConstant,
                               real restLength,
							   real maxDistance) :
		ParticleAnchoredSpring(anchor, springConstant, restLength)
		{
			springMaxDistance = maxDistance;
		}

		/** Retrieve the max distance. */
        real getMaxDistance() const { return springMaxDistance; }

        /** Applies the spring force to the given particle. */
        virtual void updateForce(Particle *particle, real duration)
		{
			// Calculate the vector of the spring
			Vector3 force;
			particle->getPosition(&force);
			force -= *anchor;

			// Calculate the magnitude of the force
			real magnitude = force.magnitude();

			// If the spring is stretched more than the max distance, deform.
			if (magnitude >= springMaxDistance)
			{
				springConstant *= 0.999;
			}

			magnitude = (restLength - magnitude) * springConstant;

			// Calculate the final force and apply it
			force.normalise();
			force *= magnitude;
			particle->addForce(force);
		}
    };

class AnchorSpring
{
private:
	ParticleDeformedAnchoredSpring springForGenerator;

	Particle *springParticle;

	real springRestLength;

public:
	AnchorSpring(Vector3* anchor, real springConstant, real restLength, Particle* particle, ParticleWorld *world)
		:
		springForGenerator(anchor, springConstant, restLength, 200.0f)
	{
		world->getForceRegistry().add(particle, &springForGenerator);
		springParticle = particle;
		springRestLength = restLength;
	}

	/** Draws the spring */
	void render()
	{
		glBegin(GL_LINES);
		const Vector3* anchor = springForGenerator.getAnchor();
		Vector3 end = springParticle->getPosition();
		real magnitude = (end - *anchor).magnitude();
		real maxDistance = springForGenerator.getMaxDistance();

		if (magnitude < springRestLength)
		{
			glColor3f(0.0f, 0.0f, 1.0f);
		}
		else if (magnitude > maxDistance)
		{
			glColor3f(1.0f, 0.0f, 0.0f);
		}
		else
		{
			glColor3f(0.0f, 1.0f, 0.0f);
		}

		glVertex3f(springForGenerator.getAnchor()->x,
			       springForGenerator.getAnchor()->y,
				   springForGenerator.getAnchor()->z);

		glVertex3f(springParticle->getPosition().x,
				   springParticle->getPosition().y,
				   springParticle->getPosition().z);
		glEnd();
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
class SpringDemo : public Application
{
	static const int PlaneWidth = 300;

	static const int PlaneHight = 300;

	ParticleWorld world;

	Ball ball;

	Vector3 upliftPosition;

	Vector3 springAnchor;

	real radius;

	PushForceGenerator pushForceGenerator;

	Vector3 anchor;

	AnchorSpring spring;

    /** Moves the particle. */
    void move(Direction dir);

public:
    /** Creates a new demo object. */
    SpringDemo();
    ~SpringDemo();

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
SpringDemo::SpringDemo()
	:
	ball(Vector3(PlaneWidth / 2.0, 0.0, PlaneHight - 100)),
	world(1),
	upliftPosition(PlaneWidth / 2, 0, PlaneHight / 2),
	radius(40.0f),
	pushForceGenerator(200, 0.5f),
	anchor(Vector3(PlaneWidth / 2.0, 0.0, PlaneHight)),
	spring(&anchor, 2.0f, 100.0f, &ball.particle, &world)
{
    world.getParticles().push_back(&ball.particle);
	world.getForceRegistry().add(&ball.particle, &pushForceGenerator);
}

SpringDemo::~SpringDemo()
{
}

void SpringDemo::initGraphics()
{
    // Call the superclass
    Application::initGraphics();

    // But override the clear color
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
}

const char* SpringDemo::getTitle()
{
    return "Cyclone > Uplift Demo";
}

void SpringDemo::move(Direction dir)
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

void SpringDemo::update()
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

void SpringDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(PlaneWidth, 150, PlaneHight / 2.0, PlaneWidth / 2.0, 0.0, PlaneHight / 2.0,  0.0, 1.0, 0.0);

	ball.render();
	spring.render();
}

void SpringDemo::key(unsigned char key)
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
    return new SpringDemo();
}