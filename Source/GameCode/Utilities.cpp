
#define UTILITIES_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "glut.h"
#include "..\\Framework\\baseTypes.h"

#include "Utilities.h"

namespace GameUtil
{
    float Utilities::squareDistance(const float x1, const float x2, const float y1, const float y2)
    {
        float diffX = x2 - x1;
        float diffY = y2 - y1;

        return (diffX * diffX) + (diffY * diffY);
    }

//-----------------------------------------------------------------------------------------

    void Utilities::drawSprite(const Coord2D& vertTR, const Coord2D& vertBL, const Coord2D& textureCoord, const Coord2D& textureSize)
    {
        // this function draws the textures, but it is up to the caller to provide the data

        glTexCoord2f(textureCoord.x, textureCoord.y);                                   // top left
        glVertex3f(vertBL.x, vertTR.y, 0.0);

        glTexCoord2f(textureCoord.x + textureSize.x, textureCoord.y);                   // top right
        glVertex3f(vertTR.x, vertTR.y, 0.0);

        glTexCoord2f(textureCoord.x + textureSize.x, textureCoord.y + textureSize.y);   // bottom right
        glVertex3f(vertTR.x, vertBL.y, 0.0);

        glTexCoord2f(textureCoord.x, textureCoord.y + textureSize.y);                   // bottom left
        glVertex3f(vertBL.x, vertBL.y, 0.0);
    }
}
