#pragma once

class Body {
public:
    float x, v, m, k, f;

    Body(float posizione = 0.f, float velocita = 0.f, float massa = 1.f, float attrito = 0.1f, float force = 0.f)
        : x(posizione), v(velocita), m(massa), k(attrito), f(force) {
    }

    float updateElasticPosition(
        float p1,
        float p2,
        float k,
        float dt = 0.1f,
        float damping = 0.5f // nuovo parametro
    ) {
        float midpoint = (p1 + p2) * 0.5f;

        // Forza elastica + smorzamento
        float force = -2.0f * k * (x - midpoint) - damping * v;

        float a = force / m;

        v += a * dt;
        x += v * dt;

        return x;
    }



    // SET
    inline void setPosition(float posizione) { x = posizione; }
    inline void setVelocity(float velocita) { v = velocita; }
    inline void setMass(float massa) { m = massa; }
    inline void setFriction(float attrito) { k = attrito; }
    inline void setForce(float force) { f = force; }

    // GET
    inline float getPosition() const { return x; }
    float getMass() const { return m; }
};
