class Cube {

    float min;
    float max;

    glm::vec3 xcol = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 ycol = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 zcol = glm::vec3(0.0f, 0.0f, 1.0f);

public:

    Cube(float min, float max) : min(min+0.1), max(max+0.1) {}

    void draw() {
        float axisExtension = 0.5f;
        float arrowSize = 0.25f;

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glLineWidth(5.0f);
        glBegin(GL_LINES);

        glColor4f(1.0, 0.0f, 0.0f, 1.0f);
        glVertex3f(min, min, min);
        glVertex3f(max + axisExtension, min, min);

        glColor4f(0.0, 1.0f, 0.0f, 1.0f);
        glVertex3f(min, min, min);
        glVertex3f(min, max + axisExtension, min);

        glColor4f(0.0, 0.0f, 1.0f, 1.0f);
        glVertex3f(min, min, min);
        glVertex3f(min, min, max + axisExtension);

        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        glVertex3f(min, max, min);
        glVertex3f(max, max, min);

        glVertex3f(max, min, min);
        glVertex3f(max, max, min);

        glVertex3f(max, min, min);
        glVertex3f(max, min, max);

        glVertex3f(max, min, max);
        glVertex3f(max, max, max);

        glVertex3f(max, max, min);
        glVertex3f(max, max, max);

        glVertex3f(min, max, min);
        glVertex3f(min, max, max);

        glVertex3f(min, max, max);
        glVertex3f(min, min, max);

        glVertex3f(min, min, max);
        glVertex3f(max, min, max);

        glVertex3f(min, max, max);
        glVertex3f(max, max, max);

        glEnd();

        // Draw arrowheads (triangles)
        glBegin(GL_TRIANGLES);

        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertex3f(max + axisExtension, min, min);
        glVertex3f(max + axisExtension - arrowSize, min + arrowSize/2, min);
        glVertex3f(max + axisExtension - arrowSize, min - arrowSize/2, min);

        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glVertex3f(min, max + axisExtension, min);
        glVertex3f(min + arrowSize/2, max + axisExtension - arrowSize, min);
        glVertex3f(min - arrowSize/2, max + axisExtension - arrowSize, min);

        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glVertex3f(min, min, max + axisExtension);
        glVertex3f(min, min + arrowSize/2, max + axisExtension - arrowSize);
        glVertex3f(min, min - arrowSize/2, max + axisExtension - arrowSize);

        glEnd();

        glPopMatrix();
    }

};
