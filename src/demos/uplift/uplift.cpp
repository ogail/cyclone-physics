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
class ParticleUplift : public ParticleForceGenerator
{
public:
	void updateForce(Particle* particle, real duration)
    {
        // Update the particle.
    }
};

class Ball : public CollisionSphere
{
public:
	unsigned startTime;

	Ball(Vector3 position)
	{
		body = new RigidBody;
		body->setMass(200.0f); // 200.0kg
		body->setVelocity(0.0f, 0.0f, 0.0f); // 50m/s
		body->setAcceleration(0.0f, 0.0f, 0.0f);
		body->setDamping(0.99f, 0.8f);
		radius = 10.0f;
		
		body->setCanSleep(false);
		body->setAwake();

		Matrix3 tensor;
		real coeff = radius * body->getMass() * radius * radius;
		tensor.setInertiaTensorCoeffs(coeff, coeff, coeff);
		body->setInertiaTensor(tensor);

		// Set the data common to all particle types
		body->setPosition(position);
		startTime = TimingData::get().lastFrameTimestamp;

		// Clear the force accumulators
		body->calculateDerivedData();
		calculateInternals();
	}

	~Ball()
	{
		delete body;
	}

	/** Draws the box, excluding its shadow. */
	void render()
	{
		// Get the OpenGL transformation
		GLfloat mat[16];
		body->getGLTransform(mat);

		glPushMatrix();
		glMultMatrixf(mat);
		glutSolidSphere(radius, 20, 20);
		glPopMatrix();
	}

	/** Moves the ball. */
	void move(Direction dir)
	{
		
	}
};

/**
 * The main demo class definition.
 */
class UpliftDemo : public Application
{
	static const int PlaneWidth = 300;

	static const int PlaneHight = 1000;

	Ball ball;

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
	: ball(Vector3(PlaneWidth / 2, 50, 50))
{
    
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
    // Move the ball.
}

void UpliftDemo::update()
{
    // Find the duration of the last frame in seconds
    float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
    if (duration <= 0.0f) return;

	// Write update code here
	ball.body->integrate(duration);
	ball.calculateInternals();

    Application::update();
}

void UpliftDemo::display()
{
    const static real size = 0.1f;

    // Clear the viewport and set the camera direction
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(PlaneWidth / 2.0, 100.0, -100.0,  PlaneWidth / 2.0, 0.0, 50.0,  0.0, 1.0, 0.0);

    glBegin(GL_QUADS);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3d(0, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, PlaneHight);
	glVertex3d(PlaneWidth, 0, 0);
	glVertex3d(0, 0, 0);
    glEnd();

	glColor3f(1, 0, 0);
	ball.render();
}

void UpliftDemo::key(unsigned char key)
{
    switch (key)
    {
	case 'a': move(Direction::Left); break;
    case 'd': move(Direction::Right); break;
    case 'w': move(Direction::Forward); break;
    case 's': move(Direction::Backward); break;
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