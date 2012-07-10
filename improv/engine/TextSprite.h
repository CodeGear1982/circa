// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

// TextSprite
//
// A piece of text that appears on the screen. Wraps a TextTexture and a TextVbo
// into one convenient object.

#include "circa/circa.h"

#include "Common.h"
#include "RenderEntity.h"

struct FontBitmap;
struct TextTexture;
struct TextVbo;

struct TextSprite : RenderEntity
{
    TextTexture* textTexture;
    TextVbo* textVbo;
    circa::Value _text;
    int _font;

    static TextSprite* create(RenderTarget* target);
    virtual void destroy();
    virtual bool destroyed();

    void setFont(int font);
    void setText(const char* str);
    void setPosition(int x, int y);
    void setColor(Color color);
    FontBitmap* getMetrics();
};
