//
// By Benjamin Bassett (benonymity) on 4.20.24
//
// For CMP-201 with Dr. Seiffert at Hillsdale College
// https://github.com/benonymity/CMP-201
//


#include "tigr.h"
#include <algorithm>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>

// Define the vector class with nice math
class Vector {
  public:
    // Constructor
    Vector(float x = 0.0f, float y = 0.0f)
        : x(x), y(y) {
    }

    // Vector variables
    float x;
    float y;

    // Overloaded operators
    Vector operator+(Vector v) {
        return Vector(x + v.x, y + v.y);
    }

    Vector operator+=(Vector v) {
        x += v.x;
        y += v.y;
        return Vector(x, y);
    }

    Vector operator-(Vector v) {
        return Vector(x - v.x, y - v.y);
    }

    Vector operator-=(Vector v) {
        x -= v.x;
        y -= v.y;
        return Vector(x, y);
    }

    Vector operator*(float b) {
        return Vector(x * b, y * b);
    }

    Vector operator*=(float b) {
        x *= b;
        y *= b;
        return Vector(x, y);
    }

    Vector operator/(float b) {
        return Vector(x / b, y / b);
    }

    Vector operator/=(float b) {
        x /= b;
        y /= b;
        return Vector(x, y);
    }

    // Get the dot product
    float dot(const Vector& other) const {
        return x * other.x + y * other.y;
    }

    // Get the cross product
    float cross(const Vector& other) const {
        return x * other.y - y * other.x;
    }

    // Get the length of the vector
    float length() {
        return sqrt(x * x + y * y);
    }

    // Get the normal of the vector
    Vector normal() {
        float len = length();
        if (len > 0) {
            return Vector(x / len, y / len);
        }
        return Vector(0, 0);
    }
};


class Display {
  public:
    // Constructor
    Display(int w, int h, std::string t) {
        width = w;
        height = h;
        screen = tigrWindow(w, h, t.c_str(), 0);
    }

    // Destructor
    ~Display() {
        tigrFree(screen);
    }

    // Draw a point
    void draw_point(Vector p, TPixel color) {
        tigrPlot(screen, int(p.x), int(p.y), color);
    }

    // Draw a line
    void draw_line(Vector a, Vector b, TPixel color) {
        tigrLine(screen, (int)a.x, (int)a.y, (int)b.x, (int)b.y, color);
    }

    // Draw a circle
    void draw_circle(Vector p, float r, TPixel color) {
        tigrFillCircle(screen, (int)p.x, (int)p.y, r, color);
    }

    // Reutrn the screen
    Tigr* get_screen() {
        return screen;
    }

    // Window size getters, but these are unreliable with padding
    int get_width() {
        return width;
    }

    int get_height() {
        return height;
    }

  private:
    // Display variables
    int width;
    int height;
    std::string title;
    Tigr* screen;
};

// Base shape class
class Shape {
  public:
    // Constructor
    virtual ~Shape() {
    }

    // Shape variables
    Vector center;
    Vector acceleration;
    Vector velocity;
    TPixel color;
    bool intersecting = false;

    // Universal shape functions
    virtual void draw(Display& d) = 0;
    virtual void update(float dt, const std::vector<Shape*>& shapes) = 0;
    virtual bool intersects(Shape& other) = 0;
};

// Circle class extending shape
class Circle : public Shape {
  public:
    // Constructor
    Circle(Vector p, float r, TPixel c = tigrRGB(0xFF, 0xFF, 0xFF),
           Vector a = Vector(0, -200.0f), Vector v = Vector(0, 0))
        : radius(r) {
        center = p;
        color = c;
        acceleration = a;
        velocity = v;
    }

    // Circle variables
    float radius;

    // Intersection delegator (causes issues with random spawning because it
    // can't run a square collision check and forward declaration doesn't work
    // with big classes for some reason)
    bool intersects(Shape& other) override {
        if (auto circle = dynamic_cast<Circle*>(&other)) {
            return intersectCircle(*circle);
        }
        return false;
    }

    // Ensure the distance check for collision is correct
    bool intersectCircle(const Circle& other) {
        float distance = (center - other.center).length();
        return distance < (radius + other.radius);
    }

    // Draw the circle (handled by display class)
    void draw(Display& d) override {
        d.draw_circle(center, radius, color);
    }

    // Update the circle
    void update(float dt, const std::vector<Shape*>& shapes) override {
        velocity -= acceleration * dt; // Apply acceleration
        center += velocity * dt;       // Apply velocity

        // Bounce off walls
        if (center.x - radius < 0 || center.x + radius > 1000) {
            velocity.x = -velocity.x * 0.9; // Reflect and dampen velocity
            center.x = velocity.x < 0 ? 1000 - radius : 0 + radius;
        }
        if (center.y - radius - 40 < 0 || center.y + radius + 40 > 1000) {
            velocity.y = -velocity.y * 0.9; // Reflect and dampen velocity
            center.y = velocity.y < 0 ? 1000 - radius - 40 : radius + 40;
        }

        // Update intersect flag
        intersecting = false;

        // Check for collisions with other circles
        // (I can't do rectangle checks because the class hasn't been declared
        // yet, and I'm too lazy to declare everything then define it later)
        for (auto shape : shapes) {
            if (shape == this)
                continue;
            Circle* other = dynamic_cast<Circle*>(shape);
            if (other && intersects(*other)) {
                intersecting = true;
                handleCollision(*other);
            }
        }
    }

    // Handle circle collisions
    void handleCollision(Circle& other) {
        // Calculate the distance between the circles and the normal vector
        Vector normal = (other.center - center).normal();
        float distance = (center - other.center).length();
        float penetration = (radius + other.radius) - distance;

        // Calculate their relative velocity
        Vector relativeVelocity = velocity - other.velocity;
        float velocityAlongNormal = relativeVelocity.dot(normal);

        // Make sure the circles are moving towards each other
        if (velocityAlongNormal < 0)
            return;

        float e = 0.9; // Resistution coefficient
        float impulseScalar = -(1 + e) * velocityAlongNormal;
        impulseScalar /=
            (1 / (radius * radius) + 1 / (other.radius * other.radius));
        // Calculate the impulse
        Vector impulse = normal * impulseScalar;

        // Apply impulse to the circles' velocities
        velocity += impulse / (radius * radius) * 0.9;
        other.velocity -= impulse / (other.radius * other.radius);

        // Actually move the circles
        center -= (1 / (radius * radius));
        other.center += (1 / (other.radius * other.radius));
    }
};

// Inheritance!
class Rectangle : public Shape {
  public:
    // Constructors
    Rectangle() {
    }

    Rectangle(Vector pos, Vector e, TPixel c = tigrRGB(0xFF, 0xFF, 0xFF),
              Vector a = Vector(0, -200.0f), Vector v = Vector(0, 0))
        : size(e) {
        center = pos;
        color = c;
        acceleration = a;
        velocity = v;
    }

    Vector size;

    // Get the center of the rectangle (don't really use this)
    Vector get_center() {
        return center + size * 0.5f;
    }

    // Polymorphism!
    // Delegate the intersection checks depending on shape type
    bool intersects(Shape& other) override {
        if (auto circle = dynamic_cast<Circle*>(&other)) {
            return intersectCircle(*circle);
        } else if (auto rect = dynamic_cast<Rectangle*>(&other)) {
            return intersectRect(*rect);
        }
        return false;
    }

    // Check for intersection with a rectangle
    bool intersectRect(const Rectangle& other) const {
        return !(center.x + size.x / 2 < other.center.x - other.size.x / 2 ||
                 center.x - size.x / 2 > other.center.x + other.size.x / 2 ||
                 center.y + size.y / 2 < other.center.y - other.size.y / 2 ||
                 center.y - size.y / 2 > other.center.y + other.size.y / 2);
    }

    // Check for intersection with a circle
    bool intersectCircle(const Circle& circle) const {
        // Find the closest point to the circle's center on the rectangle
        float closestX = std::clamp(circle.center.x, center.x - size.x / 2,
                                    center.x + size.x / 2);
        float closestY = std::clamp(circle.center.y, center.y - size.y / 2,
                                    center.y + size.y / 2);

        float distanceX = circle.center.x - closestX;
        float distanceY = circle.center.y - closestY;

        return (distanceX * distanceX + distanceY * distanceY) <
               (circle.radius * circle.radius);
    }

    void update(float dt, const std::vector<Shape*>& shapes) override {
        velocity -= acceleration * dt; // Apply acceleration
        center += velocity * dt;       // Apply velocity

        // Bounce off walls
        if (center.x - size.x / 2 < 0 || center.x + size.x / 2 > 1000) {
            velocity.x = -velocity.x * 0.9; // Reflect and dampen velocity
            center.x = velocity.x < 0 ? 1000 - size.x / 2 : 0 + size.x / 2;
        }
        // (these y adjustments of 40 I think are caused by the header of the
        // window of MacOS. Not sure if this is true on other OSes, but this
        // minimal library doesn't provide a nice way to handle screen size.)
        if (center.y - size.y / 2 - 40 < 0 ||
            center.y + size.y / 2 + 40 > 1000) {
            velocity.y = -velocity.y * 0.9; // Reflect and dampen velocity
            center.y =
                velocity.y < 0 ? 1000 - size.y / 2 - 40 : 0 + size.y / 2 + 40;
        }

        // Update the intersect flag

        intersecting = false;
        // Collision detection and response
        for (auto shape : shapes) {
            if (shape == this)
                continue;
            if (auto rect = dynamic_cast<Rectangle*>(shape)) {
                if (this->intersects(*rect)) {
                    intersecting = true;
                    handleCollision(*rect);
                }
            } else if (auto circ = dynamic_cast<Circle*>(shape)) {
                if (this->intersects(*circ)) {
                    intersecting = true;
                    handleCollision(*circ);
                }
            }
        }
    }

    // Handle rectangular collisions
    void handleCollision(Rectangle& other) {
        // Calculate overlap along each axis
        float overlapX =
            0.5 * (size.x + other.size.x) - abs(center.x - other.center.x);
        float overlapY =
            0.5 * (size.y + other.size.y) - abs(center.y - other.center.y);

        // Calculate the "mass" of each object
        float mass = size.x * size.y;
        float otherMass = other.size.x * other.size.y;

        // Make sure the rectangles are colliding
        if (overlapX > 0 && overlapY > 0) {
            // Determine which axis has the minimum overlap
            if (overlapX < overlapY) {
                // Collision is horizontal
                // Check the direction of the overlap
                if (center.x < other.center.x) {
                    center.x -= overlapX;
                } else {
                    center.x += overlapX;
                }
                // Calculate the impulse
                float impulse = velocity.x - other.velocity.x;
                velocity.x = -impulse * 0.5;
                other.velocity.x = impulse * 0.5;
            } else {
                // Collision is vertical
                if (center.y < other.center.y) {
                    center.y -= overlapY;
                } else {
                    center.y += overlapY;
                }
                // Calculate the impulse
                float impulse = velocity.y - other.velocity.y;
                velocity.y = -impulse * 0.5;
                other.velocity.y = impulse * 0.5;
            }

            // Apply a small correction to prevent the shapes sticking together
            // (this seems necessary for the rectangles, but doesn't seem to
            // work as a good strategy with the circles.)
            float massTotal = mass + otherMass;
            Vector correction = (overlapX < overlapY ? Vector(overlapX, 0)
                                                     : Vector(0, overlapY)) *
                                0.5;
            if (center.x < other.center.x) {
                center -= correction * (otherMass / massTotal);
                other.center += correction * (mass / massTotal);
            } else {
                center += correction * (otherMass / massTotal);
                other.center -= correction * (mass / massTotal);
            }
            if (center.y < other.center.y) {
                center -= correction * (otherMass / massTotal);
                other.center += correction * (mass / massTotal);
            } else {
                center += correction * (otherMass / massTotal);
                other.center -= correction * (mass / massTotal);
            }
        }
    }

    // Handle circular collisions
    void handleCollision(Circle& circle) {
        // Find the closest point on the circle to the rectangle
        float closestX = std::clamp(circle.center.x, center.x - size.x / 2,
                                    center.x + size.x / 2);
        float closestY = std::clamp(circle.center.y, center.y - size.y / 2,
                                    center.y + size.y / 2);

        // Find the point of contact
        Vector collisionPoint(closestX, closestY);
        Vector collisionNormal = (circle.center - collisionPoint).normal();
        float overlap =
            circle.radius - (circle.center - collisionPoint).length();

        // Check if the circles are colliding
        if (overlap > 0) {
            // Calculate impulse to resolve collision
            Vector relativeVelocity = circle.velocity - velocity;
            float velocityAlongNormal = relativeVelocity.dot(collisionNormal);

            // Make sure they're moving towards each other
            if (velocityAlongNormal > 0)
                return;

            // Calculate the "mass" of the rectangle
            float mass = size.x * size.y;

            // Restitution coefficient
            float e = 0.9;

            float combinedMass =
                (1 / mass) + (1 / (circle.radius * circle.radius));

            float impulseScalar = -(1 + e) * velocityAlongNormal / combinedMass;
            // Calculate the impulse
            Vector impulse = collisionNormal * impulseScalar;

            // Apply the impulse
            velocity -= impulse / mass;
            circle.velocity += impulse / (circle.radius * circle.radius);

            // Apply a position correction
            center -= (1 / mass);
            circle.center += (1 / (circle.radius * circle.radius));
        }
    }

    // Draw the shape with the built-in rectangle function
    void draw(Display& d) override {
        Vector topLeft = center - Vector(size.x / 2, size.y / 2);
        tigrFillRect(d.get_screen(), topLeft.x, topLeft.y, size.x, size.y,
                     color);
    }
};

// Create a random shape
Shape* createRandomShape(const std::vector<Shape*>& shapes, Vector a,
                         int a_const) {
    // Randomize the shape type
    int type = rand() % 2;
    // type = 0;
    // Randomize the parameters of the shape
    float radius = 20 + rand() % 30;
    TPixel color = tigrRGB(rand() % 256, rand() % 256, rand() % 256);
    Vector size(40 + rand() % 100, 40 + rand() % 60);

    // Randomize the position
    Vector position(rand() % 940 + 30, rand() % 940 + 30);

    // Set the velocity to 0
    Vector v = Vector(0, 0);

    // Run the collision check 5 times—if it fails them all, there are probably
    // too many shapes
    for (int i = 0; i < 5; i++) {
        // Check for overlap with existing shapes
        for (auto existingShape : shapes) {
            if (type == 0) { // Circle
                Circle tempCircle(position, radius, color, a * a_const, v);
                if (std::any_of(shapes.begin(), shapes.end(),
                                [&tempCircle](Shape* s) {
                                    return s->intersects(tempCircle);
                                })) {
                }
            } else { // Rectangle
                Rectangle tempRectangle(position, size, color, a * a_const, v);
                if (std::any_of(shapes.begin(), shapes.end(),
                                [&tempRectangle](Shape* s) {
                                    return s->intersects(tempRectangle);
                                })) {
                }
            }
        }

        // If there's no overlap, make the shape!
        if (type == 0) {
            return new Circle(position, radius, color, a * a_const, v);
        } else {
            return new Rectangle(position, size, color, a * a_const, v);
        }
    }
    return nullptr;
}

// Handle keyboard input
void handleKeyboard(Display& d, std::vector<Shape*>& shapes, Vector& a,
                    int& a_const) {
    if (tigrKeyDown(d.get_screen(), TK_SPACE)) {
        // if (shapes.size() < 50) { // Prevent too many shapes for getting
                                  // spawned, otherwise performance issues
            // Create a random shape
            Shape* newShape = createRandomShape(shapes, a, a_const);
            if (newShape) {
                shapes.push_back(newShape);
            }
        // }
    }

    // Clear all the shapes
    if (tigrKeyDown(d.get_screen(), TK_BACKSPACE)) {
        shapes.clear();
    }

    // Handle directional keys
    if (tigrKeyDown(d.get_screen(), TK_UP)) {
        a = Vector(0, 200.0f);
        for (auto shape : shapes) {
            if (a_const == 0) {
                shape->velocity -= a;
            } else {
                shape->acceleration = a * a_const;
            }
        }
    }
    if (tigrKeyDown(d.get_screen(), TK_DOWN)) {
        a = Vector(0, -200.0f);
        for (auto shape : shapes) {
            if (a_const == 0) {
                shape->velocity -= a;
            } else {
                shape->acceleration = a * a_const;
            }
        }
    }
    if (tigrKeyDown(d.get_screen(), TK_LEFT)) {
        a = Vector(200.0f, 0);
        for (auto shape : shapes) {
            if (a_const == 0) {
                shape->velocity -= a;
            } else {
                shape->acceleration = a * a_const;
            }
        }
    }
    if (tigrKeyDown(d.get_screen(), TK_RIGHT)) {
        a = Vector(-200.0f, 0);
        for (auto shape : shapes) {
            if (a_const == 0) {
                shape->velocity -= a;
            } else {
                shape->acceleration = a * a_const;
            }
        }
    }

    // Handle gravity keys
    if (tigrKeyDown(d.get_screen(), TK_MINUS)) {
        if (a_const > 0) {
            for (auto shape : shapes) {
                shape->acceleration /= a_const;
            }
            a_const -= 1;
        }
        for (auto shape : shapes) {
            shape->acceleration *= a_const;
        }
    }
    if (tigrKeyDown(d.get_screen(), TK_EQUALS)) {
        if (a_const > 0) {
            for (auto shape : shapes) {
                shape->acceleration /= a_const;
            }
        }
        a_const += 1;
        for (auto shape : shapes) {
            shape->acceleration *= a_const;
        }
    }
}

int main() {
    // Initialize the display
    Display d(1000, 1000, "Physics");

    // Initialize needed variables
    std::vector<Shape*> shapes;
    Vector a = Vector(0, -200.0f);
    int a_const = 1;
    float t = 0;

    bool lastIntersect = false;
    bool intersect = false;
    // Start the display loop
    while (!tigrClosed(d.get_screen()) &&
           !tigrKeyDown(d.get_screen(), TK_ESCAPE)) {
        // Reset the screen (looks funny without this)
        tigrClear(d.get_screen(), tigrRGB(0, 0, 0));

        // Get time since last frame
        t = tigrTime();

        // Handle keyboard input
        handleKeyboard(d, shapes, a, a_const);
        if (intersect) {
            lastIntersect = true;
        } else {
            lastIntersect = false;
        }
        intersect = false;
        // Update and draw each shapes
        for (auto shape : shapes) {
            shape->draw(d);
            shape->update(t, shapes);
            if (shape->intersecting) {
                intersect = true;
            }
        }

        if (intersect && !lastIntersect) {
            // Shape* newShape = createRandomShape(shapes, a, a_const);
            // if (newShape) {
            //     shapes.push_back(newShape);
            // }
        }

        // Print some instructions
        tigrPrint(
            d.get_screen(), tfont, 10, 50, tigrRGB(0xff, 0xff, 0xff),
            "Look at this fun physics engine! Built from scratch except for "
            "the super-minimal OpenGL interface.\nIt's extremely simple and "
            "extremely buggy, but it was a good learning experience.\nYou may "
            "notice circles phasing into other shapes, and some shapes "
            "spontaneously\nphasing through walls; this all seems to be due to "
            "the imprecision of pointer arithmetic,\na problem I do not know "
            "how to solve in an elegant manner.Just don't go too crazy\non the "
            "gravity and number of shapes and you should be "
            "good.\n\nCommands:\n   Space: Spawn new random shape\n   "
            "Backspace: Delete all shapes\n   Up/Down/Left/Right: Change "
            "direction of gravity/Apply impulse\n   -/+: Increase/decrease "
            "gravity\n   Esc: Quit");

        // Print some stats
        tigrPrint(d.get_screen(), tfont, 890, 50, tigrRGB(0xff, 0xff, 0xff),
                  ("Shapes: " + std::to_string(shapes.size()) +
                   "\nGravity: " + std::to_string(a_const) + "G" +
                   ((a.y == 0) > 0 ? ((a.x < 0) ? " Right" : " Left")
                                   : ((a.y < 0) ? " Down" : " Up")))
                      .c_str());

        // Update the screen
        tigrUpdate(d.get_screen());
    }
}