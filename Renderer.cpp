#include "Renderer.h"
#include <windows.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <functional>
#include "FSData.h"
#include "FlightSimulator.h"

extern CFlightSimulator gFlightSimulator;
extern CFSData gFSData;

CTexture CRenderer::mTexture;
std::function<void(void)> CRenderer::mRenderFunction = nullptr;

void CRenderer::Init(int argc, char *argv[])
{
    glutInit(&argc, argv);                   // Initialize GLUT
    glutCreateWindow("Sinop Script Tester"); // Create a window with the given title
    glutInitWindowSize(1024, 768);           // Set the window's initial width & height
    glutInitWindowPosition(50, 50);          // Position the window's initial top-left corner
    glutDisplayFunc(CRenderer::Render);      // Register display callback handler for window re-paint
    glutReshapeFunc(CRenderer::Reshape);

    png_to_gl_texture(&mTexture, "spriteTest.png");
}

void CRenderer::Render()
{
    gFlightSimulator.Update();

    glClearColor(0.0f, 0.2f, 0.2f, 1.0f);               // Set background color to black and opaque
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the color buffer (background)

    glMatrixMode(GL_MODELVIEW);
    // glPushMatrix();
    glLoadIdentity();

    // Draw a Red 1x1 Square centered at origin
    /*glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
        glColor3f(0.0f, 0.0f, 1.0f); // Red
        glVertex2f( 400.f, 400.f);    // x, y
        glVertex2f( 500.f, 400.f);
        glVertex2f( 500.f,  500.f);
        glVertex2f( 400.f,  500.f);
    glEnd();

    glTranslatef(100,100,0);
    glRotatef(45.f, 0,0,1.0);
    Rect r{0,0,128,64};
    DrawImage(&mTexture, &r, 0xFFFFFFFF);*/

    if (mRenderFunction != nullptr)
        mRenderFunction();

    glPopMatrix();

    glFlush(); // Render now

    glutPostRedisplay();
}

void CRenderer::Reshape(GLsizei width, GLsizei height)
{ // GLsizei for non-negative integer
    // Compute aspect ratio of the new window
    if (height == 0)
        height = 1; // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping area to match the viewport
    glMatrixMode(GL_PROJECTION); // To operate on the Projection matrix
    glLoadIdentity();
    if (width >= height)
    {
        // aspect >= 1, set the height from -1 to 1, with larger width
        gluOrtho2D(0, 768, 512, 0); //-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
        // gluOrtho2D(0, 1024, 768, 0); //-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
    }
    else
    {
        // aspect < 1, set the width to -1 to 1, with larger height
        gluOrtho2D(0, 768, 512, 0);
        // gluOrtho2D(0, 1024, 768, 0);
    }
}

void CRenderer::DrawImage(CTexture *tex, Rect *sourcerect, Color color)
{
    glColor4ub((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, color >> 24);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex->min_filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex->mag_filter);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw a textured quad
    glBegin(GL_QUADS);
    if (sourcerect == nullptr)
    {
        glTexCoord2f(0, 0);
        glVertex2f(0, tex->h);
        glTexCoord2f(0, 1);
        glVertex2f(0, 0);
        glTexCoord2f(1, 1);
        glVertex2f(tex->w, 0);
        glTexCoord2f(1, 0);
        glVertex2f(tex->w, tex->h);
    }
    else
    {
        float w = RectWidth(*sourcerect);
        float h = RectHeight(*sourcerect);
        float leftpct = sourcerect->left / tex->w;
        float toppct = 1 - sourcerect->top / tex->h;
        float rightpct = sourcerect->right / tex->w;
        float bottompct = 1 - sourcerect->bottom / tex->h;
        //adf
        glTexCoord2f(leftpct, toppct);
        glVertex2f(-w/2, -h/2);
        glTexCoord2f(leftpct, bottompct);
        glVertex2f(-w/2, h/2);
        glTexCoord2f(rightpct, bottompct);
        glVertex2f(w/2, h/2);
        glTexCoord2f(rightpct, toppct);
        glVertex2f(w/2, -h/2);
        // glTexCoord2f(leftpct, toppct);
        // glVertex2f(0, 0);
        // glTexCoord2f(leftpct, bottompct);
        // glVertex2f(0, h);
        // glTexCoord2f(rightpct, bottompct);
        // glVertex2f(w, h);
        // glTexCoord2f(rightpct, toppct);
        // glVertex2f(w, 0);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 'q' || key == 27)
    { // 'q' or Escape key
        gFSData.CloseSocket();
        glutLeaveMainLoop();
    }
}

void CRenderer::InitSetStart(int argc, char *argv[],
        const std::function<void(std::vector<std::string>)> &init_f,
        const std::vector<std::string> &script,
        const std::function<void(void)>& render_f)
{
    Init(argc, argv);
    init_f(script);

    glutKeyboardFunc(keyboard); // does not stop dialog thread! enter "0" instead in terminal
    SetRenderFunction(render_f);
    glutMainLoop();
}
