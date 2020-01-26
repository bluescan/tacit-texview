// Crop.cpp
//
// GL and ImGui code that implements cropping images.
//
// Copyright (c) 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <Math/tColour.h>
#include <Math/tGeometry.h>
#include <System/tPrint.h>
#include "Crop.h"
#include "TacitTexView.h"
using namespace tMath;


void TexView::CropWidget::MouseButton
(
	CropLine& line, bool down, float mouse
)
{
	if (down)
	{
		if (line.Hovered && !line.Pressed)
		{
			line.PressedAnchor = mouse;
			line.Pressed = true;
		}
	}
	else
	{
		if (line.Pressed)
		{
			line.V += line.PressedDelta;
			line.PressedDelta = 0.0f;
			line.Pressed = false;
		}
	}
}


void TexView::CropWidget::MouseHovered
(
	CropLine& line, const tMath::tVector2& mouse, const tVector2& ends, bool horizontal
)
{
	tARect2 hitBox;
	if (horizontal)
		hitBox.Set(tVector2(ends.x, line.V+line.PressedDelta), tVector2(ends.y, line.V+line.PressedDelta));
	else
		hitBox.Set(tVector2(line.V+line.PressedDelta, ends.x), tVector2(line.V+line.PressedDelta, ends.y));
	hitBox.Expand(4);
	line.Hovered = hitBox.IsPointInside(mouse);
}


void TexView::CropWidget::MouseButton(bool down, const tVector2& mouse)
{
	MouseButton(LineB, down, mouse.y);
	MouseButton(LineT, down, mouse.y);
	MouseButton(LineL, down, mouse.x);
	MouseButton(LineR, down, mouse.x);
}


void TexView::CropWidget::SetLines(const tVector4& lines)
{
	LineL.V = lines.L;
	LineR.V = lines.R;
	LineT.V = lines.T;
	LineB.V = lines.B;
}


void TexView::CropWidget::UpdateDraw(const tVector4& imgext, const tVector2& mouse)
{
	if (LineB.Pressed)
		LineB.PressedDelta = mouse.y - LineB.PressedAnchor;
	if (LineT.Pressed)
		LineT.PressedDelta = mouse.y - LineT.PressedAnchor;
	if (LineL.Pressed)
		LineL.PressedDelta = mouse.x - LineL.PressedAnchor;
	if (LineR.Pressed)
		LineR.PressedDelta = mouse.x - LineR.PressedAnchor;

	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	//tPrintf("L V %f  LD %f\n", LineL.V, LineL.PressedDelta);
	MouseHovered(LineB, mouse, tVector2(l, r), true);

	MouseHovered(LineT, mouse, tVector2(l, r), true);
	MouseHovered(LineL, mouse, tVector2(b, t), false);
	MouseHovered(LineR, mouse, tVector2(b, t), false);

	ConstrainCropLines(imgext);
	DrawMatt(imgext);
	DrawLines();
	DrawHandles();
}


void TexView::CropWidget::ConstrainCropLines(const tVector4& imgext)
{
	tiClamp(LineL.V, imgext.L, imgext.R);
	tiClamp(LineR.V, imgext.L, imgext.R);
	tiClamp(LineT.V, imgext.B, imgext.T);
	tiClamp(LineB.V, imgext.B, imgext.T);
	
	tiClampMax(LineL.V, LineR.V);
	tiClampMax(LineB.V, LineT.V);
}


void TexView::CropWidget::DrawMatt(const tVector4& imext)
{
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	glColor4f(ColourClear.x, ColourClear.y, ColourClear.z, 0.75f);
	glBegin(GL_QUAD_STRIP);
	glVertex2f(imext.L, imext.B);
	glVertex2f(l, b);
	glVertex2f(imext.R, imext.B);
	glVertex2f(r, b);
	glVertex2f(imext.R, imext.T);
	glVertex2f(r, t);
	glVertex2f(imext.L, imext.T);
	glVertex2f(l, t);
	glVertex2f(imext.L, imext.B);
	glVertex2f(l, b);
	glEnd();
}


void TexView::CropWidget::DrawLines()
{
	glColor4fv(tColourf::white.E);
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	glBegin(GL_LINES);

	glColor4fv(LineB.Hovered ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(l,	b-1);
	glVertex2f(r+1,	b-1);

	glColor4fv(LineR.Hovered ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(r+1,	b-1);
	glVertex2f(r+1,	t);

	glColor4fv(LineT.Hovered ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(r+1,	t);
	glVertex2f(l,	t);

	glColor4fv(LineL.Hovered ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(l,	t);
	glVertex2f(l,	b-1);

	glColor4fv(tColourf::white.E);
	glEnd();
}


void TexView::CropWidget::DrawHandles()
{
	float l = LineL.V + LineL.PressedDelta;
	float r = LineR.V + LineR.PressedDelta;
	float b = LineB.V + LineB.PressedDelta;
	float t = LineT.V + LineT.PressedDelta;

	glBegin(GL_QUADS);

	glColor4fv((LineL.Hovered && LineB.Hovered) ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(l-4,		b-4);
	glVertex2f(l+3,		b-4);
	glVertex2f(l+3,		b+3);
	glVertex2f(l-4,		b+3);

	glColor4fv((LineR.Hovered && LineB.Hovered) ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(r-3,		b-4);
	glVertex2f(r+4,		b-4);
	glVertex2f(r+4,		b+3);
	glVertex2f(r-3,		b+3);

	glColor4fv((LineR.Hovered && LineT.Hovered) ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(r-3,		t-3);
	glVertex2f(r+4,		t-3);
	glVertex2f(r+4,		t+4);
	glVertex2f(r-3,		t+4);

	glColor4fv((LineL.Hovered && LineT.Hovered) ? tColourf::blue.E : tColourf::white.E);
	glVertex2f(l-4,		t-3);
	glVertex2f(l+3,		t-3);
	glVertex2f(l+3,		t+4);
	glVertex2f(l-4,		t+4);

	glColor4fv(tColourf::white.E);
	glEnd();
}
