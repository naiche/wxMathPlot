/////////////////////////////////////////////////////////////////////////////
// Name:            mathplot.cpp
// Purpose:         Framework for plotting in wxWindows
// Original Author: David Schalig
// Maintainer:      Davide Rondini
// Contributors:    Jose Luis Blanco, Val Greene, R1kk3r, Naiche Barcelos
// Created:         21/07/2003
// Last edit:       26/11/2014
// Copyright:       (c) David Schalig, Davide Rondini
// Licence:         wxWindows licence
/////////////////////////////////////////////////////////////////////////////

//#pragma region includes & definitions
//#include "stdafx.h"		//used in Visual Studio

#ifdef __GNUG__
// #pragma implementation "plot.h"
#pragma implementation "mathplot.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include <wx/wx.h>
// #include <wx/window.h>
//#include <wx/wxprec.h>

// Comment out for release operation:
// (Added by J.L.Blanco, Aug 2007)
// #define MATHPLOT_DO_LOGGING


#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/object.h"
#include "wx/font.h"
#include "wx/colour.h"
#include "wx/settings.h"
#include "wx/sizer.h"
#include "wx/log.h"
#include "wx/intl.h"
#include "wx/dcclient.h"
#include "wx/cursor.h"
#endif

#include "mathplot.h"
#include <wx/bmpbuttn.h>
#include <wx/module.h>
#include <wx/msgdlg.h>
#include <wx/image.h>
#include <wx/tipwin.h>
#include <math.h>

#include <cmath>
#include <cstdio> // used only for debug
#include <ctime> // used for representation of x axes involving date
#include <cstdint>

// #include "pixel.xpm"

// Memory leak debugging		(not used in Visual Studio)
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

// Legend margins
#define mpLEGEND_MARGIN 5
#define mpLEGEND_LINEWIDTH 10

// Minimum axis label separation
#define mpMIN_X_AXIS_LABEL_SEPARATION 64
#define mpMIN_Y_AXIS_LABEL_SEPARATION 32

// Number of pixels to scroll when scrolling by a line
#define mpSCROLL_NUM_PIXELS_PER_LINE  10

// See doxygen comments.
double mpWindow::zoomIncrementalFactor = 1.1;
//#pragma endregion

//#pragma region mpLayer
//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpLayer, wxObject)

mpLayer::mpLayer() : m_type(mpLAYER_UNDEF)
{
	SetPen((wxPen&) *wxBLACK_PEN);
	SetFont((wxFont&) *wxNORMAL_FONT);
	m_continuous = FALSE; // Default
	m_contpoints = FALSE; // Default
	m_showName   = TRUE;  // Default
	m_drawOutsideMargins = TRUE;
	m_visible = true;
}

void mpLayer::SetVisible(bool show) { 
	m_visible = show; 
	parent->UpdateBarChartCoordinates();
}

wxBitmap mpLayer::GetColourSquare(int side)
{
	wxBitmap square(side, side, -1);
	wxColour filler = m_pen.GetColour();
	wxBrush brush(filler,  wxBRUSHSTYLE_SOLID);
	wxMemoryDC dc;
	dc.SelectObject(square);
	dc.SetBackground(brush);
	dc.Clear();
	dc.SelectObject(wxNullBitmap);
	return square;
}
//#pragma endregion

//#pragma region mpInfoLayer
//-----------------------------------------------------------------------------
// mpInfoLayer
//-----------------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(mpInfoLayer, mpLayer)

mpInfoLayer::mpInfoLayer()
{
	m_dim = wxRect(0,0,1,1);
	m_brush = *wxTRANSPARENT_BRUSH;
	m_reference.x = 0; m_reference.y = 0;
	m_winX = 1; //parent->GetScrX();
	m_winY = 1; //parent->GetScrY();
	m_type = mpLAYER_INFO;
}

mpInfoLayer::mpInfoLayer(wxRect rect, const wxBrush* brush) : m_dim(rect)
{
	m_brush = *brush;
	m_reference.x = rect.x;
	m_reference.y = rect.y;
	m_winX = 1; //parent->GetScrX();
	m_winY = 1; //parent->GetScrY();
	m_type = mpLAYER_INFO;
}

mpInfoLayer::~mpInfoLayer()
{

}

#ifdef _WINDOWS
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

#ifdef _WINDOWS
#pragma warning( pop )
#endif

bool mpInfoLayer::Inside(wxPoint& point)
{
#if wxCHECK_VERSION(2, 8, 0)
	return m_dim.Contains(point);
#else
	return m_dim.Inside(point);
#endif
}

void mpInfoLayer::Move(wxPoint delta)
{
	m_dim.SetX(m_reference.x + delta.x);
	m_dim.SetY(m_reference.y + delta.y);
}

void mpInfoLayer::UpdateReference()
{
	m_reference.x = m_dim.x;
	m_reference.y = m_dim.y;
}


void mpInfoLayer::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		// Adjust relative position inside the window
		int scrx = w.GetScrX();
		int scry = w.GetScrY();
		// Avoid dividing by 0
		if(scrx == 0) scrx=1;
		if(scry == 0) scry=1;

		if ((m_winX != scrx) || (m_winY != scry)) {
#ifdef MATHPLOT_DO_LOGGING
		// wxLogMessage(_("mpInfoLayer::Plot() screen size has changed from %d x %d to %d x %d"), m_winX, m_winY, scrx, scry);
#endif
			if (m_winX != 1) 
				m_dim.x = floor((double)m_dim.x*scrx/m_winX);	//(int) floor((double)(m_dim.x*scrx/m_winX));
			if (m_winY != 1) {
				m_dim.y = floor((double)m_dim.y*scry/m_winY);
				UpdateReference();
			}
			// Finally update window size
			m_winX = scrx;
			m_winY = scry;
		}
		dc.SetPen(m_pen);
//     wxImage image0(wxT("pixel.png"), wxBITMAP_TYPE_PNG);
//     wxBitmap image1(image0);
//     wxBrush semiWhite(image1);
		dc.SetBrush(m_brush);
		dc.DrawRectangle(m_dim.x, m_dim.y, m_dim.width, m_dim.height);
	}
}

wxPoint mpInfoLayer::GetPosition()
{
	return m_dim.GetPosition();
}

wxSize mpInfoLayer::GetSize()
{
	return m_dim.GetSize();
}
//#pragma endregion

//#pragma region mpInfoCoords
mpInfoCoords::mpInfoCoords() : mpInfoLayer()
{

}

mpInfoCoords::mpInfoCoords(wxRect rect, const wxBrush* brush) : mpInfoLayer(rect, brush)
{

}

mpInfoCoords::~mpInfoCoords()
{

}

void mpInfoCoords::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible)
	{
		// Adjust relative position inside the window
		int scrx = w.GetScrX();
		int scry = w.GetScrY();
		if ((m_winX != scrx) || (m_winY != scry)) {
#ifdef MATHPLOT_DO_LOGGING
			// wxLogMessage(_("mpInfoLayer::Plot() screen size has changed from %d x %d to %d x %d"), m_winX, m_winY, scrx, scry);
#endif
			if (m_winX != 1) 
				m_dim.x = floor((double)m_dim.x*scrx/m_winX);
			if (m_winY != 1) {
				m_dim.y = floor((double)m_dim.y*scry/m_winY);
				UpdateReference();
			}
			// Finally update window size
			m_winX = scrx;
			m_winY = scry;
		}
		dc.SetPen(m_pen);
//     wxImage image0(wxT("pixel.png"), wxBITMAP_TYPE_PNG);
//     wxBitmap image1(image0);
//     wxBrush semiWhite(image1);
		dc.SetBrush(m_brush);
		dc.SetFont(m_font);

		/*if(m_labelType == mpX_DATETIME) {
		  fmt = (wxT("%04.0f-%02.0f-%02.0fT%02.0f:%02.0f:%02.0f"));
		  }
		else if (m_labelType == mpX_DATE) {
			 fmt = (wxT("%04.0f-%02.0f-%02.0f"));
		}
		else if (m_labelType == mpX_TIME) {
			 fmt = (wxT("%02.0f:%02.3f"));
		}
		else if (m_labelType == mpX_TIMEOFDAY) {
				  fmt = (wxT("%02.0f:%02.0f:%02.3f"));*/

		wxString line1 = wxT("x = ") + std::to_string(w.p2x(w.GetMouseX()));
		wxString line2 = wxT("y = ") + std::to_string(w.p2y(w.GetMouseY()));
		int textX1, textY1, textX2, textY2, textX, textY;
		dc.GetTextExtent(line1, &textX1, &textY1);
		dc.GetTextExtent(line2, &textX2, &textY2);
		if (textX1 > textX2) textX = textX1; else textX = textX2;
		textY = textY1 + textY2;

		if (m_dim.width < textX + 18) m_dim.width = textX + 19;
		if (m_dim.height < textY + 18) m_dim.height = textY + 18;
		dc.DrawRectangle(m_dim.x, m_dim.y, m_dim.width, m_dim.height);
		dc.DrawText(line1, m_dim.x + 10, m_dim.y + 7);
		dc.DrawText(line2, m_dim.x + 10, m_dim.y + textY1 + 10);
	}
}
//#pragma endregion

//#pragma region mpInfoLegend
mpInfoLegend::mpInfoLegend() : mpInfoLayer()
{

}

mpInfoLegend::mpInfoLegend(wxRect rect, const wxBrush* brush) : mpInfoLayer(rect, brush)
{

}

mpInfoLegend::~mpInfoLegend()
{

}

#ifdef _WINDOWS
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

#ifdef _WINDOWS
#pragma warning( pop )
#endif

void mpInfoLegend::Plot(wxDC & dc, mpWindow & w)
{
	if (!m_visible) return;

	// Adjust relative position inside the window
	int scrx = w.GetScrX();
	int scry = w.GetScrY();
	if ((m_winX != scrx) || (m_winY != scry)) {
#ifdef MATHPLOT_DO_LOGGING
	    // wxLogMessage(_("mpInfoLayer::Plot() screen size has changed from %d x %d to %d x %d"), m_winX, m_winY, scrx, scry);
#endif
		if (m_winX != 1) m_dim.x = floor((double)m_dim.x*scrx/m_winX);
		if (m_winY != 1) {
			//m_dim.y = (int) floor((double)(m_dim.y*scry/m_winY));
			UpdateReference();
		}
		// Finally update window size
		m_winX = scrx;
		m_winY = scry;
	}
//     wxImage image0(wxT("pixel.png"), wxBITMAP_TYPE_PNG);
//     wxBitmap image1(image0);
//     wxBrush semiWhite(image1);
	dc.SetBrush(m_brush);
	dc.SetFont(m_font);
	const int baseWidth = (mpLEGEND_MARGIN*2 + mpLEGEND_LINEWIDTH);
	int textX = baseWidth, textY = mpLEGEND_MARGIN;
	int plotCount = 0;
	int posY = 0;
	int tmpX = 0, tmpY = 0;
	mpLayer* ly = NULL;
	wxPen lpen;
	wxString label;
	for (unsigned int p = 0; p < w.CountAllLayers(); p++) {
		ly = w.GetLayer(p);
		if ((ly->GetLayerType() == mpLAYER_PLOT) && (ly->IsVisible())) {
			label = ly->GetName();
			dc.GetTextExtent(label, &tmpX, &tmpY);
			textX = (textX > (tmpX + baseWidth)) ? textX : (tmpX + baseWidth + mpLEGEND_MARGIN);
			textY += (tmpY);
#ifdef MATHPLOT_DO_LOGGING
			// wxLogMessage(_("mpInfoLegend::Plot() Adding layer %d: %s"), p, label.c_str());
#endif
		}
	}
	dc.SetPen(m_pen);
	m_dim.width = textX;
	if (textY != mpLEGEND_MARGIN) { // Don't draw anything if there are no visible layers
        textY += mpLEGEND_MARGIN;
		m_dim.height = textY;
		dc.DrawRectangle(m_dim.x, m_dim.y, m_dim.width, m_dim.height);
		for (unsigned int p2 = 0; p2 < w.CountAllLayers(); p2++) {
			ly = w.GetLayer(p2);
			if ((ly->GetLayerType() == mpLAYER_PLOT) && (ly->IsVisible())) {
				label = ly->GetName();
				lpen = ly->GetPen();
				dc.GetTextExtent(label, &tmpX, &tmpY);
				dc.SetPen(lpen);
				//textX = (textX > (tmpX + baseWidth)) ? textX : (tmpX + baseWidth);
				//textY += (tmpY + mpLEGEND_MARGIN);
				posY = m_dim.y + mpLEGEND_MARGIN + plotCount*tmpY + (tmpY>>1);
				dc.DrawLine(m_dim.x + mpLEGEND_MARGIN,   // X start coord
						    posY,                        // Y start coord
						    m_dim.x + mpLEGEND_LINEWIDTH + mpLEGEND_MARGIN, // X end coord
						    posY);
				//dc.DrawRectangle(m_dim.x + 5, m_dim.y + 5 + plotCount*tmpY, 5, 5);
				dc.DrawText(label, m_dim.x + baseWidth, m_dim.y + mpLEGEND_MARGIN + plotCount*tmpY);
				plotCount++;
			}
		}
	}
}
//#pragma endregion

//#pragma region mpFX, mpFY, mpFXY
//-----------------------------------------------------------------------------
// mpLayer implementations - functions
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpFX, mpLayer)

mpFX::mpFX(wxString name, int flags)
{
	SetName(name);
	m_flags = flags;
	m_type = mpLAYER_PLOT;
}

void mpFX::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);

		wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft();
		wxCoord endPx   = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
		wxCoord minYpx  = m_drawOutsideMargins ? 0 : w.GetMarginTop();
		wxCoord maxYpx  = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

		wxCoord iy = 0;
	
		for (wxCoord i = startPx; i < endPx; ++i)
		{
			iy = w.y2p( GetY(w.p2x(i)));
			if (m_drawOutsideMargins || ((iy >= minYpx) && (iy <= maxYpx))) {   // Draw the point only if you can draw outside margins or if the point is inside margins
				if (m_pen.GetWidth() <= 1)
					dc.DrawPoint(i, iy);
				else
					dc.DrawLine(i, iy, i, iy);
			}
		}

		if (!m_name.IsEmpty() && m_showName)
		{
			dc.SetFont(m_font);

			wxCoord tx, ty;
			dc.GetTextExtent(m_name, &tx, &ty);

			/*if ((m_flags & mpALIGNMASK) == mpALIGN_RIGHT)
				tx = (w.GetScrX()>>1) - tx - 8;
			else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
				tx = -tx/2;
			else
				tx = -(w.GetScrX()>>1) + 8;
			*/
			if ((m_flags & mpALIGNMASK) == mpALIGN_RIGHT)
				tx = (w.GetScrX() - tx) - w.GetMarginRight() - 8;
			else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
				tx = ((w.GetScrX() - w.GetMarginRight() - w.GetMarginLeft() - tx) / 2) + w.GetMarginLeft();
			else
				tx = w.GetMarginLeft() + 8;
			dc.DrawText( m_name, tx, w.y2p(GetY(w.p2x(tx))) ); // (wxCoord) ((w.GetPosY() - GetY( (double)tx / w.GetScaleX() + w.GetPosX())) * w.GetScaleY()) );
		}
	}
}

IMPLEMENT_ABSTRACT_CLASS(mpFY, mpLayer)

mpFY::mpFY(wxString name, int flags)
{
	SetName(name);
	m_flags = flags;
	m_type = mpLAYER_PLOT;
}

void mpFY::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);

		wxCoord i, ix;

		wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft();
		wxCoord endPx   = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
		wxCoord minYpx  = m_drawOutsideMargins ? 0 : w.GetMarginTop();
		wxCoord maxYpx  = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

		for (i = minYpx; i < maxYpx; ++i)
		{
			ix = w.x2p(GetX(w.p2y(i)));
			if (m_drawOutsideMargins || ((ix >= startPx) && (ix <= endPx))) {
				if (m_pen.GetWidth() <= 1)
					dc.DrawPoint(ix, i);
				else
					dc.DrawLine(ix, i, ix, i);
			}
		}

		if (!m_name.IsEmpty() && m_showName)
		{
			dc.SetFont(m_font);

			wxCoord tx, ty;
			dc.GetTextExtent(m_name, &tx, &ty);

			if ((m_flags & mpALIGNMASK) == mpALIGN_TOP)
				ty = w.GetMarginTop() + 8;
			else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
				ty = ((w.GetScrY() - w.GetMarginTop() - w.GetMarginBottom() - ty) / 2) + w.GetMarginTop();
			else
				ty = w.GetScrY() - 8 - ty - w.GetMarginBottom();

			dc.DrawText( m_name, w.x2p(GetX(w.p2y(ty))), ty ); // (wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX()), -ty);
		}
	}
}

IMPLEMENT_ABSTRACT_CLASS(mpFXY, mpLayer)

mpFXY::mpFXY(wxString name, int flags)
{
	SetName(name);
	m_flags = flags;
	m_type = mpLAYER_PLOT;
}

ViewBoundary mpFXY::UpdateViewBoundary(wxCoord xnew, wxCoord ynew)
{
	// Keep track of how many points have been drawn and the bouding box
	maxDrawX = (xnew > maxDrawX) ? xnew : maxDrawX;
	minDrawX = (xnew < minDrawX) ? xnew : minDrawX;
	maxDrawY = (maxDrawY > ynew) ? maxDrawY : ynew;
	minDrawY = (minDrawY < ynew) ? minDrawY : ynew;
	//drawnPoints++;
	
	ViewBoundary vb;
	vb.maxDrawX = maxDrawX;
	vb.minDrawX = minDrawX;
	vb.maxDrawY = maxDrawY;
	vb.minDrawY = minDrawY;
	
	return vb;
}

//wxRealPoint mpFXY::GetClosestXY(double x, double y, double scaleX, double scaleY){
std::tuple<double, double, double>  mpFXY::GetClosestXY(double x, double y, double scaleX, double scaleY) {
	double delta, closestX, closestY;
	double xValue, yValue;
	Rewind();
	GetNextXY(xValue, yValue);
	delta = sqrt(pow((x - xValue)*scaleX, 2) + pow((y - yValue)*scaleY, 2));
	closestX = xValue;
	closestY = yValue;

	while(GetNextXY(xValue, yValue)) {            //runs through function looking for the closest X and Y value
		double thisPointDelta = sqrt(pow((x - xValue)*scaleX, 2) + pow((y - yValue)*scaleY, 2));
		if (thisPointDelta < delta) {
			closestX = xValue;
			closestY = yValue;
			delta = thisPointDelta;
		}
	}
	return {closestX, closestY, closestY};//wxRealPoint(closestX, closestY);
}

void mpFXY::Plot(wxDC & dc, mpWindow & w)
{
	if (!m_visible) return;

	dc.SetPen( m_pen);

	double x, y;
	// Do this to reset the counters to evaluate bounding box for label positioning
	Rewind(); GetNextXY(x, y);
	maxDrawX = x; minDrawX = x; maxDrawY = y; minDrawY = y;
	//drawnPoints = 0;
	Rewind();

	wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft();
	wxCoord endPx   = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
	wxCoord minYpx  = m_drawOutsideMargins ? 0 : w.GetMarginTop();
	wxCoord maxYpx  = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

	wxCoord ix = 0, iy = 0;

	if (!m_continuous)
	{
		while (GetNextXY(x, y))
		{
			ix = w.x2p(x);
			iy = w.y2p(y);
			if (m_drawOutsideMargins || ((ix >= startPx) && (ix <= endPx) && (iy >= minYpx) && (iy <= maxYpx))) {
				// for some reason DrawPoint does not use the current pen, so we use DrawLine for fat pens
				if (m_pen.GetWidth() <= 1)
					dc.DrawPoint(ix, iy);
				else
					dc.DrawLine(ix, iy, ix, iy);
				UpdateViewBoundary(ix, iy);
			};
		}
	}
	else
	{
		// Old code
		wxCoord x0=0,c0=0;
		bool    first = TRUE;
		while (GetNextXY(x, y))
		{
			wxCoord x1 = w.x2p(x); // (wxCoord) ((x - w.GetPosX()) * w.GetScaleX());
			wxCoord c1 = w.y2p(y); // (wxCoord) ((w.GetPosY() - y) * w.GetScaleY());
			if (first)
			{
				first = FALSE;
				x0=x1; c0=c1;
			}
			bool outUp, outDown;
			// These coordinates are used when part of the line is
			//   outside the view space and will be clipped.
			wxCoord xclip=-1,cclip=-1;
			bool clip_from=false, clip_to=false;
			// if the current and next points are in the x-view space
			if((x1 >= startPx)&&(x0 <= endPx)) {
				// both current and next are above the y-view space
				outDown = (c0 > maxYpx) && (c1 > maxYpx);
				// both current and next are below the y-view space
				outUp = (c0 < minYpx) && (c1 < minYpx);
				// if at least one point is visible in the y-view space
				if (!outUp && !outDown) {
					// Do any recalculation due to clipping.
					// Don't alter (x0,c0) or (x1,c1). We need these
					//   to properly handle clipping.
					if (c1 != c0) {
					// current point is below view
						if (c0 < minYpx) {
							xclip = (int)(((float)(minYpx - c0))/((float)(c1 - c0))*(x1-x0)) + x0;
							cclip = minYpx;
							clip_from = true;
						}
						// current point is above view
						if (c0 > maxYpx) {
							xclip = (int)(((float)(maxYpx - c0))/((float)(c1 - c0))*(x1-x0)) + x0;
	//wxLogDebug(wxT("old x0 = %d, new x0 = %d"), x0, newX0);
							//x0 = newX0;
							cclip = maxYpx;
							clip_from = true;
						}
						if (c1 < minYpx) {
							xclip = (int)(((float)(minYpx - c0))/((float)(c1 - c0))*(x1-x0)) + x0;
							cclip = minYpx;
							clip_to = true;
						}
						if (c1 > maxYpx) {
							xclip = (int)(((float)(maxYpx - c0))/((float)(c1 - c0))*(x1-x0)) + x0;
	//wxLogDebug(wxT("old x0 = %d, old x1 = %d, new x1 = %d, c0 = %d, c1 = %d, maxYpx = %d"), x0, x1, newX1, c0, c1, maxYpx);
							//x1 = newX1;
							cclip = maxYpx;
							clip_to = true;
						}
					}
					if (x1 != x0) {
						if (x0 < startPx) {
							cclip = (int)(((float)(startPx - x0))/((float)(x1 -x0))*(c1 -c0)) + c0;
							xclip = startPx;
							clip_from = true;
						}
						if (x1 > endPx) {
							cclip = (int)(((float)(endPx - x0))/((float)(x1 -x0))*(c1 -c0)) + c0;
							xclip = endPx;
							clip_to = true;
						}
					}
					if (clip_from)
						dc.DrawLine(xclip, cclip, x1, c1);
					else if (clip_to)
						dc.DrawLine(x0, c0, xclip, cclip);
					else
						dc.DrawLine(x0, c0, x1, c1);
					if (m_contpoints) {
						// Don't draw the point if it's outside the view space.
						if (!(clip_from || clip_to))
							dc.DrawCircle(x1,c1,m_pen.GetWidth()*1.1);
					}
					UpdateViewBoundary(x1, c1);
				}
			}
			x0=x1; c0=c1;
		}
	}

	if (!m_name.IsEmpty() && m_showName)
	{
		dc.SetFont(m_font);

		wxCoord tx, ty;
		dc.GetTextExtent(m_name, &tx, &ty);

		// xxx implement else ... if (!HasBBox())
		{
			// const int sx = w.GetScrX();
			// const int sy = w.GetScrY();

			if ((m_flags & mpALIGNMASK) == mpALIGN_NW)
			{
				tx = minDrawX + 8;
				ty = maxDrawY + 8;
			}
			else if ((m_flags & mpALIGNMASK) == mpALIGN_NE)
			{
				tx = maxDrawX - tx - 8;
				ty = maxDrawY + 8;
			}
			else if ((m_flags & mpALIGNMASK) == mpALIGN_SE)
			{
				tx = maxDrawX - tx - 8;
				ty = minDrawY - ty - 8;
			}
			else
			{ // mpALIGN_SW
				tx = minDrawX + 8;
				ty = minDrawY - ty - 8;
			}
		}

		dc.DrawText( m_name, tx, ty);
	}
}
//#pragma endregion

//#pragma region mpProfile
//-----------------------------------------------------------------------------
// mpProfile implementation
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpProfile, mpLayer)

mpProfile::mpProfile(wxString name, int flags)
{
	SetName(name);
	m_flags = flags;
	m_type = mpLAYER_PLOT;
}

void mpProfile::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);

		wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft();
		wxCoord endPx   = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
		wxCoord minYpx  = m_drawOutsideMargins ? 0 : w.GetMarginTop();
		wxCoord maxYpx  = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

	// Plot profile linking subsequent point of the profile, instead of mpFY, which plots simple points.
		for (wxCoord i = startPx; i < endPx; ++i) {
			wxCoord c0 = w.y2p( GetY(w.p2x(i)) ); // (wxCoord) ((w.GetYpos() - GetY( (double)i / w.GetXscl() + w.GetXpos()) ) * w.GetYscl());
			wxCoord c1 = w.y2p( GetY(w.p2x(i+1)) );//(wxCoord) ((w.GetYpos() - GetY( (double)(i+1) / w.GetXscl() + (w.GetXpos() ) ) ) * w.GetYscl());
		        // c0 = (c0 <= maxYpx) ? ((c0 >= minYpx) ? c0 : minYpx) : maxYpx;
		        // c1 = (c1 <= maxYpx) ? ((c1 >= minYpx) ? c1 : minYpx) : maxYpx;
			if (!m_drawOutsideMargins) {
				c0 = (c0 <= maxYpx) ? ((c0 >= minYpx) ? c0 : minYpx) : maxYpx;
				c1 = (c1 <= maxYpx) ? ((c1 >= minYpx) ? c1 : minYpx) : maxYpx;
			}
			dc.DrawLine(i, c0, i+1, c1);
		};
		if (!m_name.IsEmpty()) {
			dc.SetFont(m_font);

			wxCoord tx, ty;
			dc.GetTextExtent(m_name, &tx, &ty);

			if ((m_flags & mpALIGNMASK) == mpALIGN_RIGHT)
				tx = (w.GetScrX() - tx) - w.GetMarginRight() - 8;
			else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
				tx = ((w.GetScrX() - w.GetMarginRight() - w.GetMarginLeft() - tx) / 2) + w.GetMarginLeft();
			else
				tx = w.GetMarginLeft() + 8;

			dc.DrawText( m_name, tx, w.y2p( GetY( w.p2x(tx) ) ) );//(wxCoord) ((w.GetPosY() - GetY( (double)tx / w.GetScaleX() + w.GetPosX())) * w.GetScaleY()) );
		}
	}
}
//#pragma endregion

//#pragma region mpScaleX
//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

#define mpLN10 2.3025850929940456840179914546844

IMPLEMENT_DYNAMIC_CLASS(mpScaleX, mpLayer)

mpScaleX::mpScaleX(wxString name, int flags, bool ticks, unsigned int type)
{
	SetName(name);
	SetFont( (wxFont&) *wxSMALL_FONT);
	SetPen( (wxPen&) *wxGREY_PEN);
	m_flags = flags;
	m_ticks = ticks;
	m_grid = !ticks;
	m_labelType = type;
	m_type = mpLAYER_AXIS;
	m_labelFormat = wxT("");
}

void mpScaleX::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);
		dc.SetFont( m_font);
		int orgy=0;
			wxCoord tx, ty;
			wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft();
			wxCoord endPx = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
			int labelH = 0; // Control labels heigth to decide where to put axis name (below labels or on top of axis)

		const int extend = w.GetScrX(); //  /2;
		if (m_flags == mpALIGN_CENTER)
			orgy   = w.y2p(0); //(int)(w.GetPosY() * w.GetScaleY());
		if (m_flags == mpALIGN_TOP) {
			if (m_drawOutsideMargins)
				orgy = X_BORDER_SEPARATION;
			else
				orgy = w.GetMarginTop();
		}
		if (m_flags == mpALIGN_BOTTOM) {
			if (m_drawOutsideMargins)
				orgy = X_BORDER_SEPARATION;
			else
				orgy = w.GetScrY() - w.GetMarginBottom();
		}
		if (m_flags == mpALIGN_BORDER_BOTTOM )
		  orgy = w.GetScrY() - 1;//dc.LogicalToDeviceY(0) - 1;
		if (m_flags == mpALIGN_BORDER_TOP )
		  orgy = 1;//-dc.LogicalToDeviceY(0);

		dc.DrawLine( 0, orgy, w.GetScrX(), orgy);

		// To cut the axis line when draw outside margin is false, use this code
		/*if (m_drawOutsideMargins == true)
			dc.DrawLine( 0, orgy, w.GetScrX(), orgy);
		else
			dc.DrawLine( w.GetMarginLeft(), orgy, w.GetScrX() - w.GetMarginRight(), orgy); */

#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(wxT("mpScaleX::Plot: ScaleX: %f, ScreenX %d"), w.GetScaleX(), w.GetScrX());
#endif
		if (m_labelType != mpX_NORMAL) { // Date and/or time axis representation
			labelH = DatePlot(dc, w, orgy, startPx, endPx);
		}
		else{							//mpX_NORMAL
			const double dig  = floor( log( 128.0 / w.GetScaleX() ) / mpLN10 );
			const double step = exp( mpLN10 * dig);
			const double end  = w.GetPosX() + (double)extend / w.GetScaleX();
			double n0 = floor( (w.GetPosX() /* - (double)(extend - w.GetMarginLeft() - w.GetMarginRight())/ w.GetScaleX() */) / step ) * step ;

			wxString s, fmt;

		int tmp = (int)dig;
		if (!m_labelFormat.IsEmpty()) {
		  fmt = m_labelFormat;
		} else {
			if (tmp>=1) {
				fmt = wxT("%.f");
			} else {
				tmp=8-tmp;
				fmt.Printf(wxT("%%.%df"), tmp >= -1 ? 2 : -tmp);
			}
		}
		double n = 0;

		wxCoord minYpx = m_drawOutsideMargins ? 0 : w.GetMarginTop();
		wxCoord maxYpx = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();
#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(wxT("mpScaleX::Plot: dig: %f , step: %f, end: %f, n: %f"), dig, step, end, n0);
#endif
			int maxExtent = 0;
			for (n = n0; n < end; n += step) {
				const int p = (int)((n - w.GetPosX()) * w.GetScaleX());
#ifdef MATHPLOT_DO_LOGGING
				wxLogMessage(wxT("mpScaleX::Plot: n: %f -> p = %d"), n, p);
				wxLogMessage(wxT("mpScaleX::Plot: Px range: %d,%d"), startPx, endPx);
	#endif
				if ((p >= startPx) && (p <= endPx)) {
					if (m_ticks) { // draw axis ticks
						wxPen tickPen(w.GetAxesColour(), 2, wxPENSTYLE_DOT);
						dc.SetPen(tickPen);
						if (m_flags == mpALIGN_BORDER_BOTTOM)
							dc.DrawLine(p, orgy, p, orgy - 4);
						else
							dc.DrawLine(p, orgy, p, orgy + 4);
					}
					if (m_grid) { // draw grid dotted lines
						wxPen gridPen(w.GetGridColour(), 2, wxPENSTYLE_DOT);
						dc.SetPen(gridPen);
						if ((m_flags == mpALIGN_BOTTOM) && !m_drawOutsideMargins) {
							dc.DrawLine(p, orgy + 4, p, minYpx);
						}
						else {
							if ((m_flags == mpALIGN_TOP) && !m_drawOutsideMargins) {
								dc.DrawLine(p, orgy - 4, p, maxYpx);
							}
							else {
								dc.DrawLine(p, 0/*-w.GetScrY()*/, p, w.GetScrY());
							}
						}
						dc.SetPen(m_pen);
					}
					// Write ticks labels in s string

					s.Printf(fmt, n);

					dc.GetTextExtent(s, &tx, &ty);
					labelH = (labelH <= ty) ? ty : labelH;

					maxExtent = (tx > maxExtent) ? tx : maxExtent; // Keep in mind max label width
				}
			}
			// Actually draw labels, taking care of not overlapping them, and distributing them regularly
			double labelStep = ceil((maxExtent + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()*step))*step;
			for (n = n0; n < end; n += labelStep) {
				const int p = (int)((n - w.GetPosX()) * w.GetScaleX());
#ifdef MATHPLOT_DO_LOGGING
				wxLogMessage(wxT("mpScaleX::Plot: n_label = %f -> p_label = %d"), n, p);
#endif
				if ((p >= startPx) && (p <= endPx)) {
					// Write ticks labels in s string
					s.Printf(fmt, n);

					dc.GetTextExtent(s, &tx, &ty);
					if ((m_flags == mpALIGN_BORDER_BOTTOM) || (m_flags == mpALIGN_TOP)) {
						dc.DrawText(s, p - tx / 2, orgy - 4 - ty);
					}
					else {
						dc.DrawText(s, p - tx / 2, orgy + 4);
					}
				}
			}
		}

	// Draw axis name
		dc.GetTextExtent(m_name, &tx, &ty);
		switch (m_flags) {
			case mpALIGN_BORDER_BOTTOM:
				dc.DrawText( m_name, extend - tx - 4, orgy - 8 - ty - labelH);
			break;
			case mpALIGN_BOTTOM: {
				if ((!m_drawOutsideMargins) && (w.GetMarginBottom() > (ty + labelH + 8))) {
					dc.DrawText( m_name, (endPx - startPx - tx)>>1, orgy + 6 + labelH);
				} else {
					dc.DrawText( m_name, extend - tx - 4, orgy - 4 - ty);
				}
			} break;
			case mpALIGN_CENTER:
				dc.DrawText( m_name, extend - tx - 4, orgy - 4 - ty);
			break;
			case mpALIGN_TOP: {
				if ((!m_drawOutsideMargins) && (w.GetMarginTop() > (ty + labelH + 8))) {
					dc.DrawText( m_name, (endPx - startPx - tx)>>1, orgy - 6 - ty - labelH);
				} else {
					dc.DrawText( m_name, extend - tx - 4, orgy + 4);
				}
			} break;
			case mpALIGN_BORDER_TOP:
				dc.DrawText( m_name, extend - tx - 4, orgy + 6 + labelH);
			break;
			default:
			break;
		}
	}
}
//#pragma endregion

//#pragma region mpScaleY
IMPLEMENT_DYNAMIC_CLASS(mpScaleY, mpLayer)

int mpScaleX::DatePlot(wxDC & dc, mpWindow & w, int orgy, wxCoord startPx, wxCoord endPx) {
	wxCoord tx, ty;
	wxString fmt = "", s;
	long long int n0 = floor(w.GetPosX());
	long long int end = w.GetPosX() + (double)w.GetScrX() / w.GetScaleX();
	wxCoord minYpx = m_drawOutsideMargins ? 0 : w.GetMarginTop();
	wxCoord maxYpx = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

	wxDateTime current((wxLongLong)n0*1000);
	if (current == wxInvalidDateTime) return 0;
	if (n0 < -pow(2, 37)) return 0;
	if (end > (pow(2, 63) - 2)) return 0;

	wxDateTime whenEnd((wxLongLong)end*1000);

	bool isTimeStep = false;
	wxTimeSpan timeStep = 0;
	wxDateSpan dateStep = 0;
	bool isMiliSec = false;

	long long int interval = end - n0;

	if (interval > 3600 * 24 * 8000) {  //Year intervals
		fmt = (wxT("%Y"));
		dc.GetTextExtent("9999", &tx, &ty);
		double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()* 3600 * 24 * 365));
		dateStep.SetYears(labelStep);
	}
	else if (interval > 3600 * 24 * 1000) {
		fmt = "%Y";
		dateStep.SetYears(1);
	}
	else if (interval > 3600 * 24 * 300) {
		fmt = "%b-%Y";
	dc.GetTextExtent("aaa-99", &tx, &ty);
	double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()* 3600 * 24 * 30));
		dateStep.SetMonths(labelStep);
	}
	else if (interval > 3600 * 24 * 80) {
		fmt = "%b-%Y";
		dateStep.SetMonths(1);
	}
	else if (interval > 3600 * 24) {
		fmt = (wxT("%d %b %Y"));
		dc.GetTextExtent("99 mmm 99", &tx, &ty);
		double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()* 3600 * 24));
		dateStep.SetDays(labelStep);
  }
	else if(m_labelType == mpX_DATE){
		fmt = (wxT("%d %b %Y"));
		dateStep.SetDays(1);
	}
	else {//if (m_labelType == mpX_DATETIME || m_labelType == mpX_HOURS || m_labelType == mpX_TIME || m_labelType == mpX_TIMEOFDAY) {
		isTimeStep = true;
		if ((m_labelType == mpX_TIME) || (m_labelType == mpX_TIMEOFDAY))
			fmt = (wxT("%H:%M:%S"));
		else
			fmt = (wxT("%Y-%m-%dT%H:%M:%S"));
		dc.GetTextExtent("9999-19-99T99:99", &tx, &ty);
		if (interval > 3600 * 3) {
			double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()*3600));
			timeStep = wxTimeSpan(labelStep,0,0,0);//   .Set(labelStep);
		}
		else if (interval > 60 * 6) {
			double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()*60));
			timeStep = wxTimeSpan(0,labelStep,0,0);//   .Set(labelStep);
		}
		else if (interval > 4) {
			double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()));
			timeStep = wxTimeSpan(0,0,labelStep,0);//   .Set(labelStep);
		}
		else {
			dc.GetTextExtent("9999-19-99T99:99.9", &tx, &ty);
			double labelStep = ceil((tx + mpMIN_X_AXIS_LABEL_SEPARATION) / (w.GetScaleX()/1000));
			timeStep = wxTimeSpan(0, 0, 0, labelStep);//   .Set(labelStep);
			isMiliSec = true;
		}
	}

	if(!isTimeStep){
		current.SetDay(1);
		current.SetMonth(wxDateTime::Month(0));
	}

	while (current < whenEnd) {
	if (isTimeStep)
		current.Add(timeStep);
	else
		current.Add(dateStep);

	if (current == wxInvalidDateTime) break;
		const double p = (((double)current.GetValue().GetValue()/1000 - w.GetPosX()) * w.GetScaleX());
      //wxLogMessage(wxT("ScaleX: %f, PosX %f, P: %f, %ld"), w.GetScaleX(), w.GetPosX(), p, current.GetValue().GetValue());

	if (m_labelType == mpX_HOURS) {
		long hour = (long)current.GetValue().GetValue()/1000/3600;
		int remainder = abs((long)(current.GetValue().GetValue()/1000) % 3600);
		if (hour == 0 && current.GetTicks() < 0) {
			s.Printf(wxT("-%02ld:%02d:%02d"), hour, remainder/60, remainder % 60);
		} else {
			s.Printf(wxT("%02ld:%02d:%02d"), hour, remainder/60, remainder % 60);
		}
	}
	else if (isMiliSec)
		if (m_labelType == mpX_TIMEOFDAY)
			s.Printf("%s.%03d", current.Format(fmt), current.GetMillisecond());
		else
		s.Printf("%s.%03d", current.Format(fmt, wxDateTime::GMT0), current.GetMillisecond());
	else
		if (m_labelType == mpX_TIMEOFDAY)
			s.Printf("%s", current.Format(fmt));
		else
			s.Printf("%s", current.Format(fmt, wxDateTime::GMT0));

	dc.GetTextExtent(s, &tx, &ty);
		if ((p >= startPx) && (p <= endPx)) {
			//m_pen.SetColour(w.m_axColour);
			dc.DrawText(s, p - tx / 2, orgy + 4);

			if (m_grid)
			{ // draw grid dotted lines
				wxPen gridPen(w.GetGridColour(), 1, wxPENSTYLE_DOT);
				dc.SetPen(gridPen);
				if ((m_flags == mpALIGN_BOTTOM) && !m_drawOutsideMargins) {
					dc.DrawLine(p, orgy, p, minYpx);
				}
				else {
					if ((m_flags == mpALIGN_TOP) && !m_drawOutsideMargins) {
						dc.DrawLine(p, orgy - 4, p, maxYpx);
					}
					else {
						dc.DrawLine(p, 0/*-w.GetScrY()*/, p, w.GetScrY());
					}
				}
				dc.SetPen(m_pen);
			}
			if (m_ticks){ // draw axis ticks
				wxPen tickPen(w.GetAxesColour(), 1, wxPENSTYLE_SOLID);
				dc.SetPen(tickPen);
				if (m_flags == mpALIGN_BORDER_BOTTOM)
					dc.DrawLine(p, orgy, p, orgy - 4);
				else
					dc.DrawLine(p, orgy, p, orgy + 4);
			}
		}
	}

	dc.GetTextExtent(s, &tx, &ty);
	int labelH = ty;//(labelH <= ty) ? ty : labelH;
	return labelH;
}


mpScaleY::mpScaleY(wxString name, int flags, bool ticks)
{
	SetName(name);
	SetFont( (wxFont&) *wxSMALL_FONT);
	SetPen( (wxPen&) *wxGREY_PEN);
	m_flags = flags;
	m_ticks = ticks;
	m_grid = !ticks;
	m_type = mpLAYER_AXIS;
	m_labelFormat = wxT("");
}

void mpScaleY::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);
		dc.SetFont( m_font);

		int orgx=0;
		const int extend = w.GetScrY(); // /2;
		if (m_flags == mpALIGN_CENTER)
			orgx   = w.x2p(0); //(int)(w.GetPosX() * w.GetScaleX());
		if (m_flags == mpALIGN_LEFT) {
			if (m_drawOutsideMargins)
				orgx = Y_BORDER_SEPARATION;
			else
				orgx = w.GetMarginLeft();
		}
		if (m_flags == mpALIGN_RIGHT) {
			if (m_drawOutsideMargins)
				orgx = w.GetScrX() - Y_BORDER_SEPARATION;
			else
				orgx = w.GetScrX() - w.GetMarginRight();
		}
		if (m_flags == mpALIGN_BORDER_RIGHT )
			orgx = w.GetScrX() - 1; //dc.LogicalToDeviceX(0) - 1;
		if (m_flags == mpALIGN_BORDER_LEFT )
			orgx = 1; //-dc.LogicalToDeviceX(0);

		// Draw line
		dc.DrawLine( orgx, 0, orgx, extend);

		// To cut the axis line when draw outside margin is false, use this code
		/* if (m_drawOutsideMargins == true)
			dc.DrawLine( orgx, 0, orgx, extend);
		else
			dc.DrawLine( orgx, w.GetMarginTop(), orgx, w.GetScrY() - w.GetMarginBottom()); */

		const double dig  = floor( log( 128.0 / w.GetScaleY() ) / mpLN10 );
		const double step = exp( mpLN10 * dig);
		const double end  = w.GetPosY() + (double)extend / w.GetScaleY();

		wxCoord tx, ty;
		wxString s;
		wxString fmt;
		int tmp = (int)dig;
		double maxScaleAbs = fabs(w.GetDesiredYmax());
		double minScaleAbs = fabs(w.GetDesiredYmin());
		double endscale = (maxScaleAbs > minScaleAbs) ? maxScaleAbs : minScaleAbs;
		if (m_labelFormat.IsEmpty()) {
			if ((endscale < 1e4) && (endscale > 1e-3))
				fmt = wxT("%.2f"); //????????????
			else
				fmt = wxT("%.1e");
		} else {
			fmt = m_labelFormat;
		}

		double n = floor( (w.GetPosY() - (double)(extend - w.GetMarginTop() - w.GetMarginBottom())/ w.GetScaleY()) / step ) * step ;

		/* wxCoord startPx = m_drawOutsideMargins ? 0 : w.GetMarginLeft(); */
		wxCoord endPx   = m_drawOutsideMargins ? w.GetScrX() : w.GetScrX() - w.GetMarginRight();
		wxCoord minYpx  = m_drawOutsideMargins ? 0 : w.GetMarginTop();
		wxCoord maxYpx  = m_drawOutsideMargins ? w.GetScrY() : w.GetScrY() - w.GetMarginBottom();

		tmp=65536;
		int labelW = 0;
		// Before starting loop, calculate label height
		int labelHeigth = 0;
		s.Printf(fmt,n);
		dc.GetTextExtent(s, &tx, &labelHeigth);
		for (;n < end; n += step) {
			const int p = (int)((w.GetPosY() - n) * w.GetScaleY());
			if ((p >= minYpx) && (p <= maxYpx)) {
				if (m_grid) { //Draw grid lines
            		wxPen gridPen(w.GetGridColour(), 1, wxPENSTYLE_DOT);
					dc.SetPen(gridPen);
					if ((m_flags == mpALIGN_LEFT) && !m_drawOutsideMargins) {
						dc.DrawLine( orgx-4, p, endPx, p);
					} else {
						if ((m_flags == mpALIGN_RIGHT) && !m_drawOutsideMargins) {
						dc.DrawLine( minYpx, p, orgx+4, p);
									} else {
						dc.DrawLine( 0/*-w.GetScrX()*/, p, w.GetScrX(), p);
							}

					}
					dc.SetPen( m_pen);
				}
				if (m_ticks) {// Draw axis ticks
  				if (m_flags == mpALIGN_BORDER_LEFT) {
  					dc.DrawLine(orgx, p, orgx + 4, p);
  				}
  				else {
  					dc.DrawLine(orgx - 4, p, orgx+1, p); //( orgx, p, orgx+4, p);
  				}
        }

				// Print ticks labels
				s.Printf(fmt, n);
				dc.GetTextExtent(s, &tx, &ty);
	#ifdef MATHPLOT_DO_LOGGING
				if (ty != labelHeigth) wxLogMessage(wxT("mpScaleY::Plot: ty(%f) and labelHeigth(%f) differ!"), ty, labelHeigth);
	#endif
				labelW = (labelW <= tx) ? tx : labelW;
				if ((tmp-p+labelHeigth/2) > mpMIN_Y_AXIS_LABEL_SEPARATION) {
					if ((m_flags == mpALIGN_BORDER_LEFT) || (m_flags == mpALIGN_RIGHT))
						dc.DrawText( s, orgx+4, p-ty/2);
					else
						dc.DrawText( s, orgx-4-tx, p-ty/2); //( s, orgx+4, p-ty/2);
					tmp=p-labelHeigth/2;
				}
			}
		}
		// Draw axis name
		dc.GetTextExtent(m_name, &tx, &ty);
		switch (m_flags) {
			case mpALIGN_BORDER_LEFT:
				dc.DrawText( m_name, labelW + 8, 4);
			break;
			case mpALIGN_LEFT: {
				if ((!m_drawOutsideMargins) && (w.GetMarginLeft() > (ty + labelW + 8))) {
					dc.DrawRotatedText( m_name, orgx - 6 - labelW - ty, (maxYpx - minYpx + tx)>>1, 90);
				} else {
					dc.DrawText( m_name, orgx + 4, 4);
				}
			} break;
			case mpALIGN_CENTER:
				dc.DrawText( m_name, orgx + 4, 4);
			break;
			case mpALIGN_RIGHT: {
				if ((!m_drawOutsideMargins) && (w.GetMarginRight() > (ty + labelW + 8))) {
					dc.DrawRotatedText( m_name, orgx + 6 + labelW, (maxYpx - minYpx + tx)>>1, 90);
				} else {
					dc.DrawText( m_name, orgx - tx - 4, 4);
				}
			} break;
			case mpALIGN_BORDER_RIGHT:
				dc.DrawText( m_name, orgx - 6 - tx -labelW, 4);
			break;
			default:
			break;
		}
	}

/*    if (m_flags != mpALIGN_RIGHT) {
    dc.GetTextExtent(m_name, &tx, &ty);
    if (m_flags == mpALIGN_BORDER_LEFT) {
            dc.DrawText( m_name, orgx-tx-4, -extend + ty + 4);
        } else {
            if (m_flags == mpALIGN_BORDER_RIGHT )
                dc.DrawText( m_name, orgx-(tx*2)-4, -extend + ty + 4);
            else
                dc.DrawText( m_name, orgx + 4, -extend + 4);
        }
    }; */
}
//#pragma endregion

//#pragma region mpWindow
//-----------------------------------------------------------------------------
// mpWindow
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpWindow, wxWindow)

BEGIN_EVENT_TABLE(mpWindow, wxWindow)
	EVT_PAINT    ( mpWindow::OnPaint)
	EVT_SIZE     ( mpWindow::OnSize)
	EVT_SCROLLWIN_THUMBTRACK(mpWindow::OnScrollThumbTrack)
	EVT_SCROLLWIN_PAGEUP(mpWindow::OnScrollPageUp)
	EVT_SCROLLWIN_PAGEDOWN(mpWindow::OnScrollPageDown)
	EVT_SCROLLWIN_LINEUP(mpWindow::OnScrollLineUp)
	EVT_SCROLLWIN_LINEDOWN(mpWindow::OnScrollLineDown)
	EVT_SCROLLWIN_TOP(mpWindow::OnScrollTop)
	EVT_SCROLLWIN_BOTTOM(mpWindow::OnScrollBottom)

	EVT_LEFT_DCLICK(mpWindow::OnMouseLeftDClick)
	EVT_MIDDLE_UP( mpWindow::OnMouseMiddleUp)//OnShowPopupMenu)
	EVT_MIDDLE_DOWN(mpWindow::OnMouseMiddleDown)
	EVT_RIGHT_DOWN( mpWindow::OnMouseRightDown) // JLB
	EVT_RIGHT_UP ( mpWindow::OnMouseRightUp)
	EVT_MOUSEWHEEL( mpWindow::OnMouseWheel )   // JLB
	EVT_MOTION( mpWindow::OnMouseMove )   // JLB
	EVT_LEFT_DOWN( mpWindow::OnMouseLeftDown)
	EVT_LEFT_UP( mpWindow::OnMouseLeftRelease)

	EVT_MENU( mpID_CENTER,    mpWindow::OnCenter)
	EVT_MENU( mpID_FIT,       mpWindow::OnFit)
	EVT_MENU( mpID_ZOOM_IN,   mpWindow::OnZoomIn)
	EVT_MENU( mpID_ZOOM_OUT,  mpWindow::OnZoomOut)
	EVT_MENU( mpID_LOCKASPECT,mpWindow::OnLockAspect)
	EVT_MENU( mpID_HELP_MOUSE,mpWindow::OnMouseHelp)
END_EVENT_TABLE()

mpWindow::mpWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long flag )
	: wxWindow( parent, id, pos, size, flag, wxT("mathplot") )
{
	m_scaleX = m_scaleY = 1.0;
	m_posX   = m_posY   = 0;
	m_desiredXmin=m_desiredYmin=0;
	m_desiredXmax=m_desiredYmax=1;
	m_scrX   = m_scrY   = 64; // Fixed from m_scrX = m_scrX = 64;
	m_minX   = m_minY   = 0;
	m_maxX   = m_maxY   = 0;
	m_last_lx= m_last_ly= 0;
	m_buff_bmp = NULL;
	m_enableDoubleBuffer        = FALSE;
	m_enableMouseNavigation     = TRUE;
	m_mouseMovedAfterRightClick = FALSE;
	m_movingInfoLayer = NULL;
	// Set margins to 0
	m_marginTop = 0; m_marginRight = 0; m_marginBottom = 0; m_marginLeft = 0;


	m_lockaspect = FALSE;

	m_popmenu.Append( mpID_CENTER,     _("Center"),      _("Center plot view to this position"));
	m_popmenu.Append( mpID_FIT,        _("Fit"),         _("Set plot view to show all items"));
	m_popmenu.Append( mpID_ZOOM_IN,    _("Zoom in"),     _("Zoom in plot view."));
	m_popmenu.Append( mpID_ZOOM_OUT,   _("Zoom out"),    _("Zoom out plot view."));
	m_popmenu.AppendCheckItem( mpID_LOCKASPECT, _("Lock aspect"), _("Lock horizontal and vertical zoom aspect."));
	m_popmenu.Append( mpID_HELP_MOUSE,   _("Show mouse commands..."),    _("Show help about the mouse commands."));

	m_layers.clear();
	SetBackgroundColour( *wxWHITE );
	 m_bgColour = *wxWHITE;
	 m_fgColour = *wxBLACK;
	 m_axColour = *wxBLACK;
	 m_grColour = wxColour(200, 200, 200);

	m_enableScrollBars = false;
	SetSizeHints(128, 128);

	// J.L.Blanco: Eliminates the "flick" with the double buffer.
	SetBackgroundStyle( wxBG_STYLE_CUSTOM );

	UpdateAll();
}

mpWindow::~mpWindow()
{
	// Free all the layers:
	DelAllLayers( true, false );

	if (m_buff_bmp)
	{
	    delete m_buff_bmp;
	    m_buff_bmp = NULL;
	}
}

//#pragma region WindowEvents
// Mouse handler, for detecting when the user drag with the right button or just "clicks" for the menu
// JLB
// Mouse handler, for detecting when the user drag with the right button or just "clicks" for the menu
	// JLB

	// Process mouse wheel events
	// JLB
	void mpWindow::OnMouseWheel( wxMouseEvent &event )
	{
		if (!m_enableMouseNavigation)
		{
			event.Skip();
			return;
		}
	//     GetClientSize( &m_scrX,&m_scrY);

		mpMouseWheelCommand mwCommand = mpWHEEL_NO_ACTION;
		if (event.m_controlDown)
			mwCommand = ctrlWheellCommand;
		else
			if (event.m_shiftDown)
				mwCommand = shiftWheellCommand;
			else
				mwCommand = wheelCommand;

		if (mwCommand == mpZOOM) {
			wxPoint clickPt(event.GetX(), event.GetY());
			// CTRL key hold: Zoom in/out:
			if (event.GetWheelRotation() > 0)
				ZoomIn(clickPt);
			else    ZoomOut(clickPt);
		}
		else
		{
			// Scroll vertically or horizontally (this is SHIFT is hold down).
			int change = - event.GetWheelRotation(); // Opposite direction (More intuitive)!
			double changeUnitsX = change / m_scaleX;
			double changeUnitsY = change / m_scaleY;

			if (mwCommand == mpHORIZONTAL_PAN)
			{
				m_posX 		+= changeUnitsX;
				m_desiredXmax 	+= changeUnitsX;
				m_desiredXmin 	+= changeUnitsX;
			}
			else if (mwCommand == mpVERTICAL_PAN)
			{
				m_posY 		-= changeUnitsY;
				m_desiredYmax	-= changeUnitsY;
				m_desiredYmax	-= changeUnitsY;
			}

			UpdateAll();
		}
	}

	// If the user "drags" with the pan buttom pressed, do "pan"
	// JLB
	void mpWindow::OnMouseMove(wxMouseEvent &event)
	{
		m_cursorX = event.GetX();
		m_cursorY = event.GetY();

    if (!m_enableMouseNavigation) {
      event.Skip();
      return;
    }

		if (m_panDown)	//Pan
		{
			m_mouseMovedAfterRightClick = TRUE;  // Hides the popup menu after releasing the button!

			// The change:
			int  Ax= m_mouseRClick_X - event.GetX();
			int  Ay= m_mouseRClick_Y - event.GetY();

			// For the next event, use relative to this coordinates.
			m_mouseRClick_X = event.GetX();
			m_mouseRClick_Y = event.GetY();

			double   Ax_units = Ax / m_scaleX;
			double   Ay_units = -Ay / m_scaleY;

			m_posX += Ax_units;
			m_posY += Ay_units;
      m_desiredXmax 	+= Ax_units;
      m_desiredXmin 	+= Ax_units;
      m_desiredYmax 	+= Ay_units;
      m_desiredYmin 	+= Ay_units;

      UpdateAll();

	#ifdef MATHPLOT_DO_LOGGING
			wxLogMessage(_("[mpWindow::OnMouseMove] Ax:%i Ay:%i m_posX:%f m_posY:%f"),Ax,Ay,m_posX,m_posY);
	#endif
		} else {
			UpdateAll();
			if (event.LeftIsDown()) {
				if (m_movingInfoLayer != NULL) {
          m_zoomRectDown = false;
					wxPoint moveVector(event.GetX() - m_mouseLClick_X, event.GetY() - m_mouseLClick_Y);
					m_movingInfoLayer->Move(moveVector);
				}
			}
		}

    event.Skip();
	}

	void mpWindow::OnMouseLeftDown (wxMouseEvent &event)
	{
		ExecuteMouseCommand(leftDownCommand);
		event.Skip();
	}
	void mpWindow::OnMouseLeftRelease(wxMouseEvent &event)
	{
		if (leftDownCommand == mpTRACK) {
			m_trackDown = false;
			UpdateAll();
		}
		else if (leftDownCommand == mpZOOM_RECTANGLE) {
			ZoomRectRelease(event.GetX(), event.GetY());
		}
    else if (leftDownCommand == mpPAN) {
      m_panDown = false;
    }

		event.Skip();
	}

	void mpWindow::OnMouseLeftDClick(wxMouseEvent &event)
	{
		ExecuteMouseCommand(doubleClickCommand);
	}

	void mpWindow::OnMouseMiddleUp(wxMouseEvent &event) {
		if (middleDownCommand == mpTRACK) {
			m_trackDown = false;
			UpdateAll();
		}
		else if (middleDownCommand == mpZOOM_RECTANGLE) {
			ZoomRectRelease(event.GetX(), event.GetY());
		}
    else if (middleDownCommand == mpPAN) {
      m_panDown = false;
    }
	}
	void mpWindow::OnMouseMiddleDown(wxMouseEvent &event) {
		ExecuteMouseCommand(middleDownCommand);
	}

	void mpWindow::OnMouseRightDown(wxMouseEvent &event)
	{
		ExecuteMouseCommand(rightDownCommand);
	}
	void mpWindow::OnMouseRightUp(wxMouseEvent &event) {
		if (rightDownCommand == mpTRACK) {
			m_trackDown = false;
			UpdateAll();
		}
		else if (rightDownCommand == mpZOOM_RECTANGLE) {
			ZoomRectRelease(event.GetX(), event.GetY());
		}
    else if (rightDownCommand == mpPAN) {
			m_panDown = false;
		}
		ExecuteMouseCommand(rightUpCommand);
	}

	void mpWindow::ExecuteMouseCommand(mpMouseButtonCommand cmd) {
		switch (cmd) {
		case mpFIT:
			Fit();
			break;
		case mpCONTEXT_MENU:
			ShowPopupMenu(GetMouseX(), GetMouseY());//event.GetX(), event.GetY());
			break;
		case mpPAN:
			PanPlot();
			break;
		case mpZOOM_RECTANGLE:
			ZoomRectEnter(GetMouseX(),GetMouseY());
			break;
		case mpTRACK:
			m_trackDown = true;
			DrawTrackBox();
			break;
		}
	}

	void mpWindow::PanPlot()
	{
		m_panDown = true;
		m_mouseMovedAfterRightClick = FALSE;
		m_mouseRClick_X = GetMouseX();//event.GetX();
		m_mouseRClick_Y = GetMouseY();//event.GetY();
	}

  	void mpWindow::DrawTrackBox() {
		//std::pair<mpLayer*, wxRealPoint> pointInfo = GetClosestPoint(p2x(GetMouseX()), p2y(GetMouseY()), this);
		std::pair<mpLayer*, std::tuple<double, double, double>> pointInfo = GetClosestPoint(p2x(GetMouseX()), p2y(GetMouseY()), this);
		//tuple<X value, Y value, y coordinate,>
		//std::get<0>(pointInfo.second)

		wxString label, valueX, valueY;
		label.Printf(wxT("%s"), pointInfo.first->GetName());

		for (wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)//while(node)
		{
			if ((*li)->IsScaleX()) {
				mpScaleX *scaleX = (mpScaleX*)(*li);

				switch(scaleX->GetLabelMode()){
					case mpX_NORMAL:
						valueX.Printf(wxT("%s: %.4f"), (*li)->GetName(), std::get<0>(pointInfo.second));
						break;
					case mpX_HOURS:
					case mpX_TIME:
				    {
						int remainder = abs((long)std::get<0>(pointInfo.second) % 3600);
						int miliseconds = abs((long)(std::get<0>(pointInfo.second)*1000) % 1000);
						long hour = (long)std::get<0>(pointInfo.second)/3600;
						if (hour == 0 && std::get<0>(pointInfo.second) < 0) {
							valueX.Printf(wxT("%s: -%02ld:%02d:%02d.%03d"), scaleX->GetName(), hour, remainder/60, remainder % 60, miliseconds);
						}else{
							valueX.Printf(wxT("%s: %02ld:%02d:%02d.%03d"), scaleX->GetName(), hour, remainder/60, remainder % 60, miliseconds);
						}
						break;
					}
					case mpX_USERDEFINED:
						valueX.Printf(GetTrackBoxXvalueFormat(), (*li)->GetName(), std::get<0>(pointInfo.second));
						break;
					default:
						wxDateTime xTime((wxLongLong)(std::get<0>(pointInfo.second) * 1000));
						switch(scaleX->GetLabelMode()) {
							case mpX_DATE:
								valueX.Printf(wxT("%s: %s"), scaleX->GetName(), xTime.Format("%d %b %Y", wxDateTime::GMT0));
								break;
							case mpX_DATETIME:
								//wxLogMessage(_("x: %ld"), ticks);
								valueX.Printf(wxT("%s: %s"), scaleX->GetName(), xTime.Format("%d %b %Y - %H:%M:%S", wxDateTime::GMT0));
								//%2.d %s %d - %02d:%02d:%02d"), scaleX->GetName(), xTime.GetDay(), wxDateTime::GetMonthName(xTime.GetMonth(), wxDateTime::Name_Abbr), xTime.GetYear(), xTime.GetHour(), xTime.GetMinute(), xTime.GetSecond());
								break;
							case mpX_TIMEOFDAY:
								valueX.Printf(wxT("%s: %s.%003d"), scaleX->GetName(), xTime.Format("%H:%M:%S"), abs((int)(std::get<0>(pointInfo.second)*1000) % 1000));//wxDateTime::GMT0));
								break;
							case mpX_USERDEFINEDDATE:
								valueX.Printf("%s: %s", scaleX->GetName(), xTime.Format(GetTrackBoxXvalueFormat()), abs((int)(std::get<0>(pointInfo.second)*1000) % 1000));//wxDateTime::GMT0));
								break;
						}
				}
			}
			else if ((*li)->IsScaleY()) {
				//valueY.Printf(m_trackbox_y_fmt, (*li)->GetName(), pointInfo.second.y);
				if (pointInfo.first->GetTrackBoxYvalueFormat().length() >0)
					valueY.Printf(pointInfo.first->GetTrackBoxYvalueFormat(), (*li)->GetName(), std::get<1>(pointInfo.second));
				else
					valueY.Printf(GetTrackBoxYvalueFormat(), (*li)->GetName(), std::get<1>(pointInfo.second));
			}
		}

		wxClientDC dc(this);
		wxPen pen(m_fgColour, 1, wxPENSTYLE_SOLID);		//wxDOT);    *wxBLACK
		dc.SetPen(pen);
		dc.SetBrush(m_bgColour);//SetBrush(*wxTRANSPARENT_BRUSH);

		int textX, textY;
		dc.GetTextExtent(valueX, &textX, &textY);
		if (dc.GetTextExtent(label).GetX() > textX)
			textX = dc.GetTextExtent(label).GetX();
		if (dc.GetTextExtent(valueY).GetX() > textX)
			textX = dc.GetTextExtent(valueY).GetX();

		wxRect m_dim;
		#ifdef __linux__
			m_dim.width = textX + 37;
			m_dim.height = 3 * textY + 29;
		#else
			m_dim.width = textX + 35;
			m_dim.height = 3 * textY + 33;
		#endif

		if(x2p(std::get<0>(pointInfo.second)) < GetScreenRect().width-m_dim.width)
			m_dim.x = x2p(std::get<0>(pointInfo.second));
		else
			m_dim.x = x2p(std::get<0>(pointInfo.second)) - m_dim.width;

		if (y2p( std::get<2>(pointInfo.second) ) < GetScreenRect().height - m_dim.height)
			m_dim.y = y2p(std::get<2>(pointInfo.second));
		else
			m_dim.y = y2p(std::get<2>(pointInfo.second)) - m_dim.height;

		int labelMargin = (m_dim.width - dc.GetTextExtent(label).GetX()) / 2.2;
		int dateMargin = (m_dim.width - dc.GetTextExtent(valueX).GetX()) / 2.1;
		int valueMargin = (m_dim.width - dc.GetTextExtent(valueY).GetX()) / 2.1;

		dc.DrawRectangle(m_dim.x, m_dim.y, m_dim.width, m_dim.height);
		dc.DrawText(label, m_dim.x + labelMargin, m_dim.y + 7);
		dc.DrawText(valueX, m_dim.x + dateMargin, m_dim.y + 32);
		dc.DrawText(valueY, m_dim.x + valueMargin, m_dim.y + 56);
	}

	//std::pair<mpLayer*, wxRealPoint> mpWindow::GetClosestPoint(double x, double y, mpWindow * w) {
						//tuple<X value, Y value, y coordinate,>
	std::pair<mpLayer*,std::tuple<double, double, double>> mpWindow::GetClosestPoint(double x, double y, mpWindow * w) {
		mpLayer* layer;
		double coordX, coordY, valueY;
		double previousDelta = 999999999999;

		for (wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)
		{
			if (!(*li)->IsVisible() || !(*li)->IsTrackable()) continue;
			if ((*li)->IsVector()) {
				mpFXYVector *vect = (mpFXYVector*)(*li);

				int closestI=0;     				//runs through vector looking for the closest X value
				double delta = sqrt(pow((x - vect->m_xs[0])*GetScaleX(), 2) + pow((y - vect->m_ys[0])*GetScaleY(), 2));
				for (int i = 1; i < vect->m_ys.size(); i++) {
					double currDelta = sqrt(pow((x - vect->m_xs[i])*GetScaleX(), 2) + pow((y - vect->m_ys[i])*GetScaleY(), 2));
					if (currDelta < delta) {
						delta = currDelta;
						closestI = i;
					}
				}

				if (delta < previousDelta) {   //compares delta of this function to previous ones, to determine the nearest point
					layer = *li;
					previousDelta = delta;
					coordX = vect->m_xs[closestI];
					coordY = vect->m_ys[closestI];
				}
				valueY = coordY;
			}
			else if ((*li)->IsFX()) {
				mpFX *fx = (mpFX*)(*li);
				
				wxCoord minXpx = fx->GetDrawOutsideMargins() ? 0 : w->GetMarginLeft();
				wxCoord maxXpx = fx->GetDrawOutsideMargins() ? w->GetScrX() : w->GetScrX() - w->GetMarginRight();

				wxCoord currX = minXpx; 
				double xDouble = w->p2x(currX);
				double closestX = xDouble;
				double delta = sqrt(pow((y - fx->GetY(xDouble))*GetScaleY(), 2) + pow((x - xDouble)*GetScaleX(), 2));
				currX++;
				for (currX; currX < maxXpx; ++currX)			//runs through function looking for the closest point
				{
					xDouble = w->p2x(currX);
					double currDelta = sqrt(pow((y - fx->GetY(xDouble))*GetScaleY(), 2) + pow((x - xDouble)*GetScaleX(), 2));
					if (currDelta < delta) {
						delta = currDelta;
						closestX = xDouble;
					}
				}
				if (delta < previousDelta) {		//compares delta of this function to previous ones, to determine the nearest point
					layer = *li;
					previousDelta = delta;
					coordX = closestX;
					coordY = fx->GetY(closestX);
				}
				valueY = coordY;
			}
			else if ((*li)->IsFY()) {
				mpFY *fy = (mpFY*)(*li);

				wxCoord minYpx = fy->GetDrawOutsideMargins() ? 0 : w->GetMarginTop();
				wxCoord maxYpx = fy->GetDrawOutsideMargins() ? w->GetScrY() : w->GetScrY() - w->GetMarginBottom();

				wxCoord currY = minYpx;
				double yDouble = w->p2y(currY);
				double closestY = yDouble;
				double delta = sqrt(pow((x - fy->GetX(yDouble))*GetScaleX(), 2) + pow((y - yDouble)*GetScaleY(), 2));
				currY++;
				for (currY; currY < maxYpx; ++currY)			//runs through function looking for the closest point
				{
					yDouble = w->p2y(currY);
					double currDelta = sqrt(pow((x - fy->GetX(yDouble))*GetScaleX(), 2) + pow((y - yDouble)*GetScaleY(), 2));
					if (currDelta < delta) {
						delta = currDelta;
						closestY = yDouble;
					}
				}
				if (delta < previousDelta) {   //compares delta of this function to previous ones, to determine the nearest point
					layer = *li;
					previousDelta = delta;
					coordX = fy->GetX(closestY);
					coordY = closestY;
				}
				valueY = coordY;
		    } 
			else if ((*li)->IsFXY()) {
				mpFXY *fxy = (mpFXY*)(*li);

				//wxRealPoint closestPoint = fxy->GetClosestXY(x, y, GetScaleX(), GetScaleY());
				//double thisFunctionDelta = sqrt(pow((x - closestPoint.x)*GetScaleX(), 2) + pow((y - closestPoint.y)*GetScaleY(), 2));
				std::tuple<double, double, double> closestPoint = fxy->GetClosestXY(x, y, GetScaleX(), GetScaleY());
				double thisFunctionDelta = sqrt(pow((x - std::get<0>(closestPoint))*GetScaleX(), 2) + pow((y - std::get<2>(closestPoint))*GetScaleY(), 2));

				if (thisFunctionDelta < previousDelta) {   //compares delta of this function to previous ones, to determine the nearest point
					//LayerName = fxy->GetName();
					layer = *li;
					previousDelta = thisFunctionDelta;
					coordX = std::get<0>(closestPoint);
					coordY = std::get<2>(closestPoint);
					valueY = std::get<1>(closestPoint);
				}
			}
		}
		std::tuple<double, double, double> t { coordX, valueY, coordY };
		return std::make_pair(layer, t);//wxRealPoint(coordX, coordY));
	}

	void mpWindow::ShowPopupMenu(int x, int y)//(wxMouseEvent &event)
	{
		// Only display menu if the user has not "dragged" the figure
		if (m_enableMouseNavigation)
		{
			SetCursor(*wxSTANDARD_CURSOR);
		}

		if (!m_mouseMovedAfterRightClick)   // JLB
		{
			m_clickedX = x;//event.GetX();
			m_clickedY = y;//event.GetY();
			PopupMenu(&m_popmenu, x, y);//event.GetX(), event.GetY());
		}
	}

	void mpWindow::ZoomRectEnter(int x, int y) {
	  m_zoomRectDown = true;
	  m_mouseLClick_X = x;
		m_mouseLClick_Y = y;
	#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(_("mpWindow::OnMouseLeftDown() X = %d , Y = %d"), event.GetX(), event.GetY());/*m_mouseLClick_X, m_mouseLClick_Y);*/
	#endif
		wxPoint pointClicked = wxPoint(x, y);//event.GetPosition();
		m_movingInfoLayer = IsInsideInfoLayer(pointClicked);
		if (m_movingInfoLayer != NULL) {
	#ifdef MATHPLOT_DO_LOGGING
			wxLogMessage(_("mpWindow::OnMouseLeftDown() started moving layer %lx"), (long int)m_movingInfoLayer);/*m_mouseLClick_X, m_mouseLClick_Y);*/
	#endif
		}
	}
	
	void mpWindow::ZoomRectRelease(int x, int y) {
	  m_zoomRectDown = false;
		wxPoint release(x, y);
		wxPoint press(m_mouseLClick_X, m_mouseLClick_Y);
		if (m_movingInfoLayer != NULL) {
			m_movingInfoLayer->UpdateReference();
			m_movingInfoLayer = NULL;
		}
		else {
			if (release != press) {
		  if(abs(m_mouseLClick_X - x)>10)   //Avoids accidentally zooming when double clicking
				   ZoomRect(press, release);
			}
		}
	}
//#pragma endregion

void mpWindow::Fit()
{
    if (UpdateBBox())
        Fit(m_minX,m_maxX,m_minY,m_maxY );
}

// JL
void mpWindow::Fit(double xMin, double xMax, double yMin, double yMax, wxCoord *printSizeX, wxCoord *printSizeY)
{
	// Save desired borders:
	m_desiredXmin=xMin; m_desiredXmax=xMax;
	m_desiredYmin=yMin; m_desiredYmax=yMax;

	if (printSizeX!=NULL && printSizeY!=NULL)
	{
		// Printer:
		m_scrX = *printSizeX;
		m_scrY = *printSizeY;
	}
	else
	{
		// Normal case (screen):
		GetClientSize( &m_scrX,&m_scrY);
	}

	double Ax,Ay;

	Ax = xMax - xMin;
	Ay = yMax - yMin;

	m_scaleX = (Ax!=0) ? (m_scrX - m_marginLeft - m_marginRight)/Ax : 1; //m_scaleX = (Ax!=0) ? m_scrX/Ax : 1;
	m_scaleY = (Ay!=0) ? (m_scrY - m_marginTop - m_marginBottom)/Ay : 1; //m_scaleY = (Ay!=0) ? m_scrY/Ay : 1;

	if (m_lockaspect)
	{
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::Fit()(lock) m_scaleX=%f,m_scaleY=%f"), m_scaleX,m_scaleY);
#endif
		// Keep the lowest "scale" to fit the whole range required by that axis (to actually "fit"!):
		double s = m_scaleX < m_scaleY ? m_scaleX : m_scaleY;
		m_scaleX = s;
		m_scaleY = s;
	}

	// Adjusts corner coordinates: This should be simply:
	//   m_posX = m_minX;
	//   m_posY = m_maxY;
	// But account for centering if we have lock aspect:
	m_posX = (xMin+xMax)/2 - ((m_scrX - m_marginLeft - m_marginRight)/2 + m_marginLeft)/m_scaleX ; // m_posX = (xMin+xMax)/2 - (m_scrX/2)/m_scaleX;
//	m_posY = (yMin+yMax)/2 + ((m_scrY - m_marginTop - m_marginBottom)/2 - m_marginTop)/m_scaleY;  // m_posY = (yMin+yMax)/2 + (m_scrY/2)/m_scaleY;
	m_posY = (yMin+yMax)/2 + ((m_scrY - m_marginTop - m_marginBottom)/2 + m_marginTop)/m_scaleY;  // m_posY = (yMin+yMax)/2 + (m_scrY/2)/m_scaleY;

#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::Fit() m_desiredXmin=%f m_desiredXmax=%f  m_desiredYmin=%f m_desiredYmax=%f"), xMin,xMax,yMin,yMax);
	wxLogMessage(_("mpWindow::Fit() m_scaleX = %f , m_scrX = %d,m_scrY=%d, Ax=%f, Ay=%f, m_posX=%f, m_posY=%f"), m_scaleX, m_scrX,m_scrY, Ax,Ay,m_posX,m_posY);
#endif

	// It is VERY IMPORTANT to DO NOT call Refresh if we are drawing to the printer!!
	// Otherwise, the DC dimensions will be those of the window instead of the printer device
	if (printSizeX==NULL || printSizeY==NULL)
		UpdateAll();
}

// Patch ngpaton
void mpWindow::DoZoomInXCalc (const int staticXpixel)
{
	// Preserve the position of the clicked point:
	double staticX = p2x( staticXpixel );
	// Zoom in:
	m_scaleX = m_scaleX * zoomIncrementalFactor;
	// Adjust the new m_posx
	m_posX = staticX - (staticXpixel / m_scaleX);
	// Adjust desired
	m_desiredXmin = m_posX;
	m_desiredXmax = m_posX + (m_scrX - (m_marginLeft + m_marginRight)) / m_scaleX;
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::DoZoomInXCalc() prior X coord: (%f), new X coord: (%f) SHOULD BE EQUAL!!"), staticX, p2x(staticXpixel));
#endif
}

void mpWindow::DoZoomInYCalc (const int staticYpixel)
{
	// Preserve the position of the clicked point:
	double staticY = p2y( staticYpixel );
	// Zoom in:
	m_scaleY = m_scaleY * zoomIncrementalFactor;
	// Adjust the new m_posy:
	m_posY = staticY + (staticYpixel / m_scaleY);
	// Adjust desired
	m_desiredYmax = m_posY;
	m_desiredYmin = m_posY - (m_scrY - (m_marginTop + m_marginBottom)) / m_scaleY;
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::DoZoomInYCalc() prior Y coord: (%f), new Y coord: (%f) SHOULD BE EQUAL!!"), staticY, p2y(staticYpixel));
#endif
}

void mpWindow::DoZoomOutXCalc  (const int staticXpixel)
{
	// Preserve the position of the clicked point:
	double staticX = p2x( staticXpixel );
	// Zoom out:
	m_scaleX = m_scaleX / zoomIncrementalFactor;
	// Adjust the new m_posx/y:
	m_posX = staticX - (staticXpixel / m_scaleX);
	// Adjust desired
	m_desiredXmin = m_posX;
	m_desiredXmax = m_posX + (m_scrX - (m_marginLeft + m_marginRight)) / m_scaleX;
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::DoZoomOutXCalc() prior X coord: (%f), new X coord: (%f) SHOULD BE EQUAL!!"), staticX, p2x(staticXpixel));
#endif
}

void mpWindow::DoZoomOutYCalc  (const int staticYpixel)
{
	// Preserve the position of the clicked point:
	double staticY = p2y( staticYpixel );
	// Zoom out:
	m_scaleY = m_scaleY / zoomIncrementalFactor;
	// Adjust the new m_posx/y:
	m_posY = staticY + (staticYpixel / m_scaleY);
	// Adjust desired
	m_desiredYmax = m_posY;
	m_desiredYmin = m_posY - (m_scrY - (m_marginTop + m_marginBottom)) / m_scaleY;
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::DoZoomOutYCalc() prior Y coord: (%f), new Y coord: (%f) SHOULD BE EQUAL!!"), staticY, p2y(staticYpixel));
#endif
}


void mpWindow::ZoomIn(const wxPoint& centerPoint )
{
	wxPoint    c(centerPoint);
	if (c == wxDefaultPosition)
	{
		GetClientSize(&m_scrX, &m_scrY);
		c.x = (m_scrX - m_marginLeft - m_marginRight)/2 + m_marginLeft; // c.x = m_scrX/2;
		c.y = (m_scrY - m_marginTop - m_marginBottom)/2 - m_marginTop; // c.y = m_scrY/2;
	}

	// Preserve the position of the clicked point:
	double prior_layer_x = p2x( c.x );
	double prior_layer_y = p2y( c.y );

	// Zoom in:
	m_scaleX = m_scaleX * zoomIncrementalFactor;
	m_scaleY = m_scaleY * zoomIncrementalFactor;

	// Adjust the new m_posx/y:
	m_posX = prior_layer_x - c.x / m_scaleX;
	m_posY = prior_layer_y + c.y / m_scaleY;

	m_desiredXmin = m_posX;
	m_desiredXmax = m_posX + (m_scrX - m_marginLeft - m_marginRight) / m_scaleX; // m_desiredXmax = m_posX + m_scrX / m_scaleX;
	m_desiredYmax = m_posY;
	m_desiredYmin = m_posY - (m_scrY - m_marginTop - m_marginBottom) / m_scaleY; // m_desiredYmin = m_posY - m_scrY / m_scaleY;


#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::ZoomIn() prior coords: (%f,%f), new coords: (%f,%f) SHOULD BE EQUAL!!"), prior_layer_x,prior_layer_y, p2x(c.x),p2y(c.y));
#endif

	UpdateAll();
}

void mpWindow::ZoomOut(const wxPoint& centerPoint )
{
	wxPoint    c(centerPoint);
	if (c == wxDefaultPosition)
	{
		GetClientSize(&m_scrX, &m_scrY);
		c.x = (m_scrX - m_marginLeft - m_marginRight)/2 + m_marginLeft; // c.x = m_scrX/2;
		c.y = (m_scrY - m_marginTop - m_marginBottom)/2 - m_marginTop; // c.y = m_scrY/2;
	}

	// Preserve the position of the clicked point:
	double prior_layer_x = p2x( c.x );
	double prior_layer_y = p2y( c.y );

	// Zoom out:
	m_scaleX = m_scaleX / zoomIncrementalFactor;
	m_scaleY = m_scaleY / zoomIncrementalFactor;

	// Adjust the new m_posx/y:
	m_posX = prior_layer_x - c.x / m_scaleX;
	m_posY = prior_layer_y + c.y / m_scaleY;

	m_desiredXmin = m_posX;
	m_desiredXmax = m_posX + (m_scrX - m_marginLeft - m_marginRight) / m_scaleX; // m_desiredXmax = m_posX + m_scrX / m_scaleX;
	m_desiredYmax = m_posY;
	m_desiredYmin = m_posY - (m_scrY - m_marginTop - m_marginBottom) / m_scaleY; // m_desiredYmin = m_posY - m_scrY / m_scaleY;

#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::ZoomOut() prior coords: (%f,%f), new coords: (%f,%f) SHOULD BE EQUAL!!"), prior_layer_x,prior_layer_y, p2x(c.x),p2y(c.y));
#endif
	UpdateAll();
}

void mpWindow::ZoomInX()
{
	m_scaleX = m_scaleX * zoomIncrementalFactor;
	UpdateAll();
}

void mpWindow::ZoomOutX()
{
	m_scaleX = m_scaleX / zoomIncrementalFactor;
	UpdateAll();
}

void mpWindow::ZoomInY()
{
	m_scaleY = m_scaleY * zoomIncrementalFactor;
	UpdateAll();
}

void mpWindow::ZoomOutY()
{
	m_scaleY = m_scaleY / zoomIncrementalFactor;
	UpdateAll();
}

void mpWindow::ZoomRect(wxPoint p0, wxPoint p1)
{
	// Compute the 2 corners in graph coordinates:
	double p0x = p2x(p0.x);
	double p0y = p2y(p0.y);
	double p1x = p2x(p1.x);
	double p1y = p2y(p1.y);

	// Order them:
	double zoom_x_min = p0x<p1x ? p0x:p1x;
	double zoom_x_max = p0x>p1x ? p0x:p1x;
	double zoom_y_min = p0y<p1y ? p0y:p1y;
	double zoom_y_max = p0y>p1y ? p0y:p1y;

#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("Zoom: (%f,%f)-(%f,%f)"),zoom_x_min,zoom_y_min,zoom_x_max,zoom_y_max);
#endif

	Fit(zoom_x_min,zoom_x_max,zoom_y_min,zoom_y_max);
}

void mpWindow::LockAspect(bool enable)
{
	m_lockaspect = enable;
	m_popmenu.Check(mpID_LOCKASPECT, enable);

	// Try to fit again with the new config:
	Fit( m_desiredXmin, m_desiredXmax, m_desiredYmin, m_desiredYmax );
}

void mpWindow::OnLockAspect(wxCommandEvent& WXUNUSED(event))
{
	LockAspect( !m_lockaspect );
}

void mpWindow::OnMouseHelp(wxCommandEvent& WXUNUSED(event))
{
	wxMessageBox(_("Supported Mouse commands:\n \
		- Left button down + Mark area: Rectangular zoom\n \
		- Right button down + Move: Pan (Move)\n \
		- Wheel: Vertical scroll\n \
		- Wheel + SHIFT: Horizontal scroll\n \
		- Wheel + CTRL: Zoom in/out"),_("wxMathPlot help"),wxOK,this);
}

void mpWindow::OnFit(wxCommandEvent& WXUNUSED(event))
{
    Fit();
}

void mpWindow::OnCenter(wxCommandEvent& WXUNUSED(event))
{
	GetClientSize(&m_scrX, &m_scrY);
		int centerX = (m_scrX - m_marginLeft - m_marginRight)/2; // + m_marginLeft; // c.x = m_scrX/2;
	int centerY = (m_scrY - m_marginTop - m_marginBottom)/2; // - m_marginTop; // c.y = m_scrY/2;
		SetPos( p2x(m_clickedX - centerX), p2y(m_clickedY - centerY) );
	//SetPos( p2x(m_clickedX-m_scrX/2), p2y(m_clickedY-m_scrY/2) );  //SetPos( (double)(m_clickedX-m_scrX/2) / m_scaleX + m_posX, (double)(m_scrY/2-m_clickedY) / m_scaleY + m_posY);
}

void mpWindow::OnZoomIn(wxCommandEvent& WXUNUSED(event))
{
	ZoomIn( wxPoint(m_mouseRClick_X,m_mouseRClick_Y) );
}

void mpWindow::OnZoomOut(wxCommandEvent& WXUNUSED(event))
{
	ZoomOut();
}

void mpWindow::OnSize( wxSizeEvent& WXUNUSED(event) )
{
	// Try to fit again with the new window size:
	Fit( m_desiredXmin, m_desiredXmax, m_desiredYmin, m_desiredYmax );
#ifdef MATHPLOT_DO_LOGGING
	wxLogMessage(_("mpWindow::OnSize() m_scrX = %d, m_scrY = %d"), m_scrX, m_scrY);
#endif // MATHPLOT_DO_LOGGING
}

//std::map<double, std::tuple<double, double>> barCoordinates; //<x, <yBase, yTop>
void mpWindow::UpdateBarChartCoordinates(){

	std::map<double, double> baseMap; 
	wxLayerList::iterator li;
	for (li = m_layers.begin(); li != m_layers.end(); li++) //Plot BarChart
	{
		if (!(*li)->IsBarChart() || !(*li)->IsVisible()) continue;

		mpBAR *barChart = (mpBAR*)(*li);
		barChart->barCoordinates.clear();
		
		std::map<double, double>::iterator base;
		double x, y, baseY;
		barChart->Rewind(); 
		barChart->GetNextXY(x, y);
		barChart->Rewind();
		while (barChart->GetNextXY(x, y)) {

			base = baseMap.find(x);
			if (base != baseMap.end()){ 
				baseY = base->second;
				base->second = baseY + y;
			}
			else {
				baseY = 0;
				baseMap.insert({x,y});
			}

//wxLogMessage(_("X=%f, Y=%f, y0 = %f, y1 = %f"), x, y, baseY, baseY+y);
			barChart->barCoordinates.insert({x,{baseY, baseY+y}});
		}
	}
}

bool mpWindow::AddLayer( mpLayer* layer, bool refreshDisplay )
{
	if (layer != NULL) {
		m_layers.push_back( layer );
		layer->parent = this;

		if (layer->IsBarChart())
			UpdateBarChartCoordinates();

		if (refreshDisplay) UpdateAll();
			return true;
	};
	return false;
}

bool mpWindow::DelLayer(
	mpLayer*    layer,
	bool        alsoDeleteObject,
	bool        refreshDisplay )
{
	wxLayerList::iterator layIt;
	for (layIt = m_layers.begin(); layIt != m_layers.end(); layIt++)
	{
		if (*layIt == layer)
		{
			// Also delete the object?
			if (alsoDeleteObject)
				delete *layIt;
			m_layers.erase(layIt); // this deleted the reference only
			if (refreshDisplay)
				UpdateAll();
			return true;
		}
	}
	return false;
}

void mpWindow::DelAllLayers( bool alsoDeleteObject, bool refreshDisplay)
{
	while ( m_layers.size()>0 )
	{
		// Also delete the object?
		if (alsoDeleteObject) delete m_layers[0];
		m_layers.erase( m_layers.begin() ); // this deleted the reference only
	}
	if (refreshDisplay)  UpdateAll();
}

// void mpWindow::DoPrepareDC(wxDC& dc)
// {
//     dc.SetDeviceOrigin(x2p(m_minX), y2p(m_maxY));
// }

void mpWindow::OnPaint( wxPaintEvent& WXUNUSED(event) )
{
	wxPaintDC dc(this);
	dc.GetSize(&m_scrX, &m_scrY);   // This is the size of the visible area only!
//     DoPrepareDC(dc);

#ifdef MATHPLOT_DO_LOGGING
	{
		int px, py;
		//GetViewStart( &px, &py );
		px = -1; py = -1;
		wxLogMessage(_("[mpWindow::OnPaint] vis.area:%ix%i px=%i py=%i"),m_scrX,m_scrY,px,py);
	}
#endif

	// Selects direct or buffered draw:
	wxDC    *trgDc;

	// J.L.Blanco @ Aug 2007: Added double buffer support
	if (m_enableDoubleBuffer)
	{
		if (m_last_lx!=m_scrX || m_last_ly!=m_scrY)
		{
			if (m_buff_bmp) delete m_buff_bmp;
			m_buff_bmp = new wxBitmap(m_scrX,m_scrY);
			m_buff_dc.SelectObject(*m_buff_bmp);
			m_last_lx=m_scrX;
			m_last_ly=m_scrY;
		}
		trgDc = &m_buff_dc;
	}
	else
	{
		trgDc = &dc;
	}

	// Draw background:
	//trgDc->SetDeviceOrigin(0,0);
	trgDc->SetPen( *wxTRANSPARENT_PEN );
	wxBrush brush( GetBackgroundColour() );
	trgDc->SetBrush( brush );
	trgDc->SetTextForeground(m_fgColour);
	trgDc->DrawRectangle(0,0,m_scrX,m_scrY);

	// Draw all the layers:
	//trgDc->SetDeviceOrigin( m_scrX>>1, m_scrY>>1);  // Origin at the center
	wxLayerList::iterator li;
	for (li = m_layers.begin(); li != m_layers.end(); li++)
	{
		(*li)->Plot(*trgDc, *this);
	};

	//std::vector<double> baseX, baseY;
	std::map<double, double> baseMap; 
	for (li = m_layers.begin(); li != m_layers.end(); li++) //Plot BarChart
	{
		if (!(*li)->IsBarChart() || !(*li)->IsVisible()) continue;

		mpBAR *barChart = (mpBAR*)(*li);
		trgDc->SetPen(barChart->GetPen());

		double x, y;
		// Do this to reset the counters to evaluate bounding box for label positioning
		barChart->Rewind(); barChart->GetNextXY(x, y);
		///maxDrawX = x; minDrawX = x; maxDrawY = y; minDrawY = y;
		//drawnPoints = 0;
		barChart->Rewind();

		wxCoord startPx = barChart->GetDrawOutsideMargins() ? 0 : GetMarginLeft();
		wxCoord endPx   = barChart->GetDrawOutsideMargins() ? GetScrX() : GetScrX() - GetMarginRight();
		wxCoord minYpx  = barChart->GetDrawOutsideMargins() ? 0 : GetMarginTop();
		wxCoord maxYpx  = barChart->GetDrawOutsideMargins() ? GetScrY() : GetScrY() - GetMarginBottom();

		wxCoord ix = 0, iy = 0, iybase = 0;

		//ViewBoundary vb;
		std::map<double, std::tuple<double, double>>::iterator barCoords;
		double baseY;
		while (barChart->GetNextXY(x, y)) {

			/*base = baseMap.find(x);
			if (base != baseMap.end()){ 
				baseY = base->second;
				base->second = baseY + y;
			}
			else {
				baseY = 0;
				baseMap.insert({x,y});
			}*/
			
			barCoords = barChart->barCoordinates.find(x);
			ix = x2p(x);
			iybase = y2p(std::get<0>(barCoords->second));
			iy = y2p(std::get<1>(barCoords->second));
			
// #ifdef MATHPLOT_DO_LOGGING
     //wxLogMessage(_("X=%f, Y=%f, ix = %d, iy = %d, iybase = %d  ->  %f  ->  %f"), x, y, ix, iy, iybase, std::get<0>(barCoords->second), std::get<1>(barCoords->second));
// #endif
			if (barChart->GetDrawOutsideMargins() || ((ix >= startPx) && (ix <= endPx) && (iy >= minYpx) && (iy <= maxYpx))) {
				trgDc->DrawLine(ix, iy, ix, iybase);
				//vb = barChart->UpdateViewBoundary(ix, iy);
			}
		}

		/*if (!barChart->GetName().IsEmpty() && barChart->GetShowName()) {
			dc.SetFont(barChart->GetFont());//m_font);

			wxCoord tx, ty;
			dc.GetTextExtent(barChart->GetName(), &tx, &ty);
			{
			if ((barChart->GetLabelAlignment() & mpALIGNMASK) == mpALIGN_NW) {
					tx = vb.minDrawX + 8;
					ty = vb.maxDrawY + 8;
				}
				else if ((barChart->GetLabelAlignment() & mpALIGNMASK) == mpALIGN_NE) {
					tx = vb.maxDrawX - tx - 8;
					ty = vb.maxDrawY + 8;
				}
				else if ((barChart->GetLabelAlignment() & mpALIGNMASK) == mpALIGN_SE) {
					tx = vb.maxDrawX - tx - 8;
					ty = vb.minDrawY - ty - 8;
				}
				else { // mpALIGN_SW
					tx = vb.minDrawX + 8;
					ty = vb.minDrawY - ty - 8;
				}
			}
			dc.DrawText( barChart->GetName(), tx, ty);
		}*/
	}

	// If doublebuffer, draw now to the window:
	if (m_enableDoubleBuffer)
	{
		//trgDc->SetDeviceOrigin(0,0);
		//dc.SetDeviceOrigin(0,0);  // Origin at the center
		dc.Blit(0,0,m_scrX,m_scrY,trgDc,0,0);
    }

/*    if (m_coordTooltip) {
		wxString toolTipContent;
		wxPoint mousePoint =  wxGetMousePosition();
		toolTipContent.Printf(_("X = %f\nY = %f"), p2x(mousePoint.x), p2y(mousePoint.y));
		SetToolTip(toolTipContent);
    }*/
    // If scrollbars are enabled, refresh them
    if (m_enableScrollBars) {
/*       m_scrollX = (int) floor((m_posX - m_minX)*m_scaleX);
       m_scrollY = (int) floor((m_maxY - m_posY )*m_scaleY);
       Scroll(m_scrollX, m_scrollY);*/
       // Scroll(x2p(m_posX), y2p(m_posY));
//             SetVirtualSize((int) ((m_maxX - m_minX)*m_scaleX), (int) ((m_maxY - m_minY)*m_scaleY));
//         int centerX = (m_scrX - m_marginLeft - m_marginRight)/2; // + m_marginLeft; // c.x = m_scrX/2;
//     int centerY = (m_scrY - m_marginTop - m_marginBottom)/2; // - m_marginTop; // c.y = m_scrY/2;
        /*SetScrollbars(1, 1, (int) ((m_maxX - m_minX)*m_scaleX), (int) ((m_maxY - m_minY)*m_scaleY));*/ //, x2p(m_posX + centerX/m_scaleX), y2p(m_posY - centerY/m_scaleY), true);
    }
	if (m_zoomRectDown){
		wxPen pen(m_fgColour, 1, wxPENSTYLE_DOT);		//wxDOT);
		dc.SetPen(pen);
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(m_mouseLClick_X, m_mouseLClick_Y, GetMouseX() - m_mouseLClick_X, GetMouseY() - m_mouseLClick_Y);
	}
	if (m_trackDown) DrawTrackBox();
}

// void mpWindow::OnScroll2(wxScrollWinEvent &event)
// {
// #ifdef MATHPLOT_DO_LOGGING
//     wxLogMessage(_("[mpWindow::OnScroll2] Init: m_posX=%f m_posY=%f, sc_pos = %d"),m_posX,m_posY, event.GetPosition());
// #endif
//     // If scrollbars are not enabled, Skip operation
//     if (!m_enableScrollBars) {
//         event.Skip();
//         return;
//     }
// //     m_scrollX = (int) floor((m_posX - m_minX)*m_scaleX);
// //     m_scrollY = (int) floor((m_maxY - m_posY /*- m_minY*/)*m_scaleY);
// //     Scroll(m_scrollX, m_scrollY);
//
// //     GetClientSize( &m_scrX, &m_scrY);
//     //Scroll(x2p(m_desiredXmin), y2p(m_desiredYmin));
//     int pixelStep = 1;
//     if (event.GetOrientation() == wxHORIZONTAL) {
//         //m_desiredXmin -= (m_scrollX - event.GetPosition())/m_scaleX;
//         //m_desiredXmax -= (m_scrollX - event.GetPosition())/m_scaleX;
//         m_posX -= (m_scrollX - event.GetPosition())/m_scaleX;
//         m_scrollX = event.GetPosition();
//     }
//     Fit(m_desiredXmin, m_desiredXmax, m_desiredYmin, m_desiredYmax);
// // /*    int pixelStep = 1;
// //     if (event.GetOrientation() == wxHORIZONTAL) {
// //         m_posX         -= (px - event.GetPosition())/m_scaleX;//(pixelStep/m_scaleX);
// //     m_desiredXmax     -= (px - event.GetPosition())/m_scaleX;//(pixelStep/m_scaleX);
// //     m_desiredXmin     -= (px - event.GetPosition())/m_scaleX;//(pixelStep/m_scaleX);
// //         //SetPosX( (double)px / GetScaleX() + m_minX + (double)(width>>1)/GetScaleX());
// // //         m_posX = p2x(px); //m_minX + (double)(px /*+ (m_scrX)*/)/GetScaleX();
// //     } else {
// //         m_posY         += (py - event.GetPosition())/m_scaleY;//(pixelStep/m_scaleY);
// //     m_desiredYmax    += (py - event.GetPosition())/m_scaleY;//(pixelStep/m_scaleY);
// //     m_desiredYmax    += (py - event.GetPosition())/m_scaleY;//(pixelStep/m_scaleY);
// //         //SetPosY( m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY());
// //         //m_posY = m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY();
// // //         m_posY = p2y(py);//m_maxY - (double)(py /*+ (m_scrY)*/)/GetScaleY();
// //     }*/
// #ifdef MATHPLOT_DO_LOGGING
//     int px, py;
//     GetViewStart( &px, &py);
//     wxLogMessage(_("[mpWindow::OnScroll2] End:  m_posX = %f, m_posY = %f, px = %f, py = %f"),m_posX, m_posY, px, py);
// #endif
//
//     UpdateAll();
// //     event.Skip();
// }

void mpWindow::SetMPScrollbars(bool status)
{
	// Temporary behaviour: always disable scrollbars
	m_enableScrollBars = status; //false;
	if (status == false)
	{
		SetScrollbar(wxHORIZONTAL, 0, 0, 0);
		SetScrollbar(wxVERTICAL, 0, 0, 0);
	}
	// else the scroll bars will be updated in UpdateAll();
	UpdateAll();

//     EnableScrolling(false, false);
//     m_enableScrollBars = status;
//     EnableScrolling(status, status);
/*    m_scrollX = (int) floor((m_posX - m_minX)*m_scaleX);
    m_scrollY = (int) floor((m_posY - m_minY)*m_scaleY);*/
//     int scrollWidth = (int) floor((m_maxX - m_minX)*m_scaleX) - m_scrX;
//     int scrollHeight = (int) floor((m_minY - m_maxY)*m_scaleY) - m_scrY;

// /*    m_scrollX = (int) floor((m_posX - m_minX)*m_scaleX);
//     m_scrollY = (int) floor((m_maxY - m_posY /*- m_minY*/)*m_scaleY);
//     int scrollWidth = (int) floor(((m_maxX - m_minX) - (m_desiredXmax - m_desiredXmin))*m_scaleX);
//     int scrollHeight = (int) floor(((m_maxY - m_minY) - (m_desiredYmax - m_desiredYmin))*m_scaleY);
// #ifdef MATHPLOT_DO_LOGGING
//     wxLogMessage(_("mpWindow::SetMPScrollbars() scrollWidth = %d, scrollHeight = %d"), scrollWidth, scrollHeight);
// #endif
//     if(status) {
//         SetScrollbars(1,
//                       1,
//                       scrollWidth,
//                       scrollHeight,
//                       m_scrollX,
//                       m_scrollY);
// //         SetVirtualSize((int) (m_maxX - m_minX), (int) (m_maxY - m_minY));
//     }
//     Refresh(false);*/
};

bool mpWindow::UpdateBBox()
{
	bool FoundBBoxLayer = FALSE;

	for (wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)
	{
		mpLayer* f = *li;

		if (f->HasBBox())
		{
			if (!FoundBBoxLayer)
			{
				FoundBBoxLayer = TRUE;
				m_minX = f->GetMinX(); m_maxX=f->GetMaxX();
				m_minY = f->GetMinY(); m_maxY=f->GetMaxY();
			}
			else
			{
				if (f->GetMinX()<m_minX) 
					m_minX=f->GetMinX();
				if (f->GetMaxX()>m_maxX)
					m_maxX=f->GetMaxX();
				if (f->GetMinY()<m_minY) 
					m_minY=f->GetMinY();
				if (f->GetMaxY()>m_maxY)
					m_maxY=f->GetMaxY();
			}
		}
		//node = node->GetNext();
	}
#ifdef MATHPLOT_DO_LOGGING
	wxLogDebug(wxT("[mpWindow::UpdateBBox] Bounding box: Xmin = %f, Xmax = %f, Ymin = %f, YMax = %f"), m_minX, m_maxX, m_minY, m_maxY);
#endif // MATHPLOT_DO_LOGGING
	return FoundBBoxLayer;
}

void mpWindow::UpdateAll()
{
	if (UpdateBBox())
	{
		if (m_enableScrollBars)
		{
			int cx, cy;
			GetClientSize( &cx, &cy);
			// Do x scroll bar
			{
				// Convert margin sizes from pixels to coordinates
				double leftMargin  = m_marginLeft / m_scaleX;
				// Calculate the range in coords that we want to scroll over
				double maxX = (m_desiredXmax > m_maxX) ? m_desiredXmax : m_maxX;
				double minX = (m_desiredXmin < m_minX) ? m_desiredXmin : m_minX;
				if ((m_posX + leftMargin) < minX)
					minX = m_posX + leftMargin;
				// Calculate scroll bar size and thumb position
				int sizeX = (int) ((maxX - minX) * m_scaleX);
				int thumbX = (int)(((m_posX + leftMargin) - minX) * m_scaleX);
				SetScrollbar(wxHORIZONTAL, thumbX, cx - (m_marginRight + m_marginLeft), sizeX);
			}
			// Do y scroll bar
			{
				// Convert margin sizes from pixels to coordinates
				double topMargin = m_marginTop / m_scaleY;
				// Calculate the range in coords that we want to scroll over
				double maxY = (m_desiredYmax > m_maxY) ? m_desiredYmax : m_maxY;
				if ((m_posY - topMargin) > maxY)
					maxY = m_posY - topMargin;
				double minY = (m_desiredYmin < m_minY) ? m_desiredYmin : m_minY;
				// Calculate scroll bar size and thumb position
				int sizeY = (int)((maxY - minY) * m_scaleY);
				int thumbY = (int)((maxY - (m_posY - topMargin)) * m_scaleY);
				SetScrollbar(wxVERTICAL, thumbY, cy - (m_marginTop + m_marginBottom), sizeY);
			}
		}
	}

	Refresh( FALSE );
}

void mpWindow::DoScrollCalc(const int position, const int orientation)
{
	if (orientation == wxVERTICAL)
	{
		// Y axis
		// Get top margin in coord units
		double topMargin = m_marginTop / m_scaleY;
		// Calculate maximum Y coord to be shown in the graph
		double maxY = m_desiredYmax > m_maxY ? m_desiredYmax  : m_maxY;
		// Set new position
		SetPosY((maxY - (position / m_scaleY)) + topMargin);
	}
	else
	{
		// X Axis
		// Get left margin in coord units
		double leftMargin  = m_marginLeft / m_scaleX;
		// Calculate minimum X coord to be shown in the graph
		double minX = (m_desiredXmin < m_minX) ? m_desiredXmin : m_minX;
		// Set new position
		SetPosX((minX + (position / m_scaleX)) - leftMargin);
	}
}

void mpWindow::OnScrollThumbTrack (wxScrollWinEvent &event)
{
	DoScrollCalc(event.GetPosition(), event.GetOrientation());
}

void mpWindow::OnScrollPageUp (wxScrollWinEvent &event)
{
	int scrollOrientation = event.GetOrientation();
	// Get position before page up
	int position = GetScrollPos(scrollOrientation);
	// Get thumb size
	int thumbSize = GetScrollThumb(scrollOrientation);
	// Need to adjust position by a page
	position -= thumbSize;
	if (position < 0)
		position = 0;

	DoScrollCalc(position, scrollOrientation);
}

void mpWindow::OnScrollPageDown (wxScrollWinEvent &event)
{
	int scrollOrientation = event.GetOrientation();
	// Get position before page up
	int position = GetScrollPos(scrollOrientation);
	// Get thumb size
	int thumbSize = GetScrollThumb(scrollOrientation);
	// Get scroll range
	int scrollRange = GetScrollRange(scrollOrientation);
	// Need to adjust position by a page
	position += thumbSize;
	if (position > (scrollRange - thumbSize))
		position = scrollRange - thumbSize;

	DoScrollCalc(position, scrollOrientation);
}

void mpWindow::OnScrollLineUp     (wxScrollWinEvent &event)
{
	int scrollOrientation = event.GetOrientation();
	// Get position before page up
	int position = GetScrollPos(scrollOrientation);
	// Need to adjust position by a line
	position -= mpSCROLL_NUM_PIXELS_PER_LINE;
	if (position < 0)
		position = 0;

	DoScrollCalc(position, scrollOrientation);
}

void mpWindow::OnScrollLineDown   (wxScrollWinEvent &event)
{
	int scrollOrientation = event.GetOrientation();
	// Get position before page up
	int position = GetScrollPos(scrollOrientation);
	// Get thumb size
	int thumbSize = GetScrollThumb(scrollOrientation);
	// Get scroll range
	int scrollRange = GetScrollRange(scrollOrientation);
	// Need to adjust position by a page
	position += mpSCROLL_NUM_PIXELS_PER_LINE;
	if (position > (scrollRange - thumbSize))
		position = scrollRange - thumbSize;

	DoScrollCalc(position, scrollOrientation);
}

void mpWindow::OnScrollTop(wxScrollWinEvent &event)
{
	DoScrollCalc(0, event.GetOrientation());
}

void mpWindow::OnScrollBottom(wxScrollWinEvent &event)
{
	int scrollOrientation = event.GetOrientation();
	// Get thumb size
	int thumbSize = GetScrollThumb(scrollOrientation);
	// Get scroll range
	int scrollRange = GetScrollRange(scrollOrientation);

	DoScrollCalc(scrollRange - thumbSize, scrollOrientation);
}
// End patch ngpaton

void mpWindow::SetScaleX(double scaleX)
{
	if (scaleX!=0) m_scaleX=scaleX;
	UpdateAll();
}

// New methods implemented by Davide Rondini

unsigned int mpWindow::CountLayers()
{
	//wxNode *node = m_layers.GetFirst();
	unsigned int layerNo = 0;
	for(wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)//while(node)
		{
		if ((*li)->HasBBox()) layerNo++;
	// node = node->GetNext();
		};
	return layerNo;
}

mpLayer* mpWindow::GetLayer(int position)
{
	if ((position >= (int) m_layers.size()) || position < 0) return NULL;
	return m_layers[position];
}

mpLayer* mpWindow::GetLayerByName( const wxString &name)
{
	for (wxLayerList::iterator it=m_layers.begin();it!=m_layers.end();it++)
		if (! (*it)->GetName().Cmp( name ) )
			return *it;
	return NULL;    // Not found
}

void mpWindow::GetBoundingBox(double* bbox)
{
	bbox[0] = m_minX;
	bbox[1] = m_maxX;
	bbox[2] = m_minY;
	bbox[3] = m_maxY;
}

bool mpWindow::SaveScreenshot(const wxString& filename, int type, wxSize imageSize, bool fit)
{
	int sizeX, sizeY;
	int bk_scrX=-1, bk_scrY=-1;
	if (imageSize == wxDefaultSize) {
		sizeX = m_scrX;
		sizeY = m_scrY;
	} else {
		sizeX = imageSize.x;
		sizeY = imageSize.y;
		bk_scrX = m_scrX;
		bk_scrY = m_scrY;
		SetScr(sizeX, sizeY);
	}

	wxBitmap screenBuffer(sizeX,sizeY);
	wxMemoryDC screenDC;
	screenDC.SelectObject(screenBuffer);
	screenDC.SetPen( *wxTRANSPARENT_PEN );
	wxBrush brush( GetBackgroundColour() );
	screenDC.SetBrush( brush );
	screenDC.DrawRectangle(0,0,sizeX,sizeY);

	if (fit) {
		Fit(m_minX, m_maxX, m_minY, m_maxY, &sizeX, &sizeY);
	} else {
		Fit(m_desiredXmin, m_desiredXmax, m_desiredYmin, m_desiredYmax, &sizeX, &sizeY);
	}
	// Draw all the layers:
	wxLayerList::iterator li;
	for (li = m_layers.begin(); li != m_layers.end(); li++)
		(*li)->Plot(screenDC, *this);

	if (imageSize != wxDefaultSize) {
		// Restore dimensions
		SetScr(bk_scrX, bk_scrY);
		        Fit(m_desiredXmin, m_desiredXmax, m_desiredYmin, m_desiredYmax, &bk_scrX, &bk_scrY);
		UpdateAll();
	}
	// Once drawing is complete, actually save screen shot
	wxImage screenImage = screenBuffer.ConvertToImage();
	return screenImage.SaveFile(filename, (wxBitmapType) type);
}

void mpWindow::SetMargins(int top, int right, int bottom, int left)
{
	m_marginTop = top;
	m_marginRight = right;
	m_marginBottom = bottom;
	m_marginLeft = left;
}

mpInfoLayer* mpWindow::IsInsideInfoLayer(wxPoint& point)
{
	wxLayerList::iterator li;
	for (li = m_layers.begin(); li != m_layers.end(); li++) {
#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(_("mpWindow::IsInsideInfoLayer() examinining layer = %p"), (*li));
#endif // MATHPLOT_DO_LOGGING
		if ((*li)->IsInfo()) {
			mpInfoLayer* tmpLyr = (mpInfoLayer*) (*li);
#ifdef MATHPLOT_DO_LOGGING
			wxLogMessage(_("mpWindow::IsInsideInfoLayer() layer = %p"), (*li));
#endif // MATHPLOT_DO_LOGGING
			if (tmpLyr->Inside(point)) {
				return tmpLyr;
			}
		}
	}
	return NULL;
}

void mpWindow::SetLayerVisible(const wxString &name, bool viewable)
{
	mpLayer* lx = GetLayerByName(name);
	if ( lx ) {
		lx->SetVisible(viewable);
		UpdateAll();
	}
}

bool mpWindow::IsLayerVisible(const wxString &name )
{
	mpLayer* lx = GetLayerByName(name);
	return (lx) ? lx->IsVisible() : false;
}

void mpWindow::SetLayerVisible(const unsigned int position, bool viewable)
{
	mpLayer* lx = GetLayer(position);
	if ( lx ) {
		lx->SetVisible(viewable);
		UpdateAll();
	}
}

bool mpWindow::IsLayerVisible(const unsigned int position )
{
	mpLayer* lx = GetLayer(position);
	return (lx) ? lx->IsVisible() : false;
}

void mpWindow::SetColourTheme(const wxColour& bgColour, const wxColour& drawColour, const wxColour& axesColour, const wxColour& gridColour)
{
	SetBackgroundColour(bgColour);
	SetForegroundColour(drawColour);
	m_bgColour = bgColour;
	m_fgColour = drawColour;
	m_axColour = axesColour;
	m_grColour = gridColour;
	// cycle between layers to set colours and properties to them
	wxLayerList::iterator li;
	for (li = m_layers.begin(); li != m_layers.end(); li++) {
		if ((*li)->GetLayerType() == mpLAYER_AXIS) {
			wxPen axisPen = (*li)->GetPen(); // Get the old pen to modify only colour, not style or width
			axisPen.SetColour(axesColour);
			(*li)->SetPen(axisPen);
		}
		if ((*li)->GetLayerType() == mpLAYER_INFO) {
			wxPen infoPen = (*li)->GetPen(); // Get the old pen to modify only colour, not style or width
			infoPen.SetColour(drawColour);
			(*li)->SetPen(infoPen);
		}
	}
}

// void mpWindow::EnableCoordTooltip(bool value)
// {
//      m_coordTooltip = value;
// //      if (value) GetToolTip()->SetDelay(100);
// }

/*
double mpWindow::p2x(wxCoord pixelCoordX, bool drawOutside )
{
    if (drawOutside) {
        return m_posX + pixelCoordX/m_scaleX;
    }
    // Draw inside margins
    double marginScaleX = ((double)(m_scrX - m_marginLeft - m_marginRight))/m_scrX;
    return m_marginLeft + (m_posX + pixelCoordX/m_scaleX)/marginScaleX;
}

double mpWindow::p2y(wxCoord pixelCoordY, bool drawOutside )
{
    if (drawOutside) {
        return m_posY - pixelCoordY/m_scaleY;
    }
    // Draw inside margins
    double marginScaleY = ((double)(m_scrY - m_marginTop - m_marginBottom))/m_scrY;
    return m_marginTop + (m_posY - pixelCoordY/m_scaleY)/marginScaleY;
}

wxCoord mpWindow::x2p(double x, bool drawOutside)
{
    if (drawOutside) {
        return (wxCoord) ((x-m_posX) * m_scaleX);
    }
    // Draw inside margins
    double marginScaleX = ((double)(m_scrX - m_marginLeft - m_marginRight))/m_scrX;
#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(wxT("x2p ScrX = %d, marginRight = %d, marginLeft = %d, marginScaleX = %f"), m_scrX, m_marginRight, m_marginLeft,  marginScaleX);
#endif // MATHPLOT_DO_LOGGING
    return (wxCoord) (int)(((x-m_posX) * m_scaleX)*marginScaleX) - m_marginLeft;
}

wxCoord mpWindow::y2p(double y, bool drawOutside)
{
    if (drawOutside) {
        return (wxCoord) ( (m_posY-y) * m_scaleY);
    }
    // Draw inside margins
    double marginScaleY = ((double)(m_scrY - m_marginTop - m_marginBottom))/m_scrY;
#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(wxT("y2p ScrY = %d, marginTop = %d, marginBottom = %d, marginScaleY = %f"), m_scrY, m_marginTop, m_marginBottom, marginScaleY);
#endif // MATHPLOT_DO_LOGGING
    return (wxCoord) ((int)((m_posY-y) * m_scaleY)*marginScaleY) - m_marginTop;
}
*/

void mpWindow::BindMouseButton(mpMouseButton mouseButton, mpMouseButtonCommand command) {
	switch (mouseButton) {
	case mpDOUBLE_CLICK:
		doubleClickCommand = command;
		break;
	case mpLEFT_DOWN:
		leftDownCommand = command;
		break;
	case mpLEFT_UP:
		leftUpCommand = command;
		break;
	case mpMIDDLE_DOWN:
		middleDownCommand = command;
		break;
	case mpMIDDLE_UP:
		middleUpCommand = command;
		break;
	case mpRIGHT_DOWN:
		rightDownCommand = command;
		break;
	case mpRIGHT_UP:
		rightUpCommand = command;
		break;
	}
}

void mpWindow::BindMouseWheel(mpMouseWheel mouseWheel, mpMouseWheelCommand command) {
	switch (mouseWheel) {
	case mpWHEEL:
		wheelCommand = command;
		break;
	case mpSHIFT_WHEEL:
		shiftWheellCommand = command;
		break;
	case mpCTRL_WHEEL:
		ctrlWheellCommand = command;
		break;
	}
}


//#pragma endregion

//#pragma region mpFXYVector
//-----------------------------------------------------------------------------
// mpFXYVector implementation - by Jose Luis Blanco (AGO-2007)
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpFXYVector, mpFXY)

// Constructor
mpFXYVector::mpFXYVector(wxString name, int flags ) : mpFXY(name,flags)
{
	m_index = 0;
	m_minX  = -1;
	m_maxX  = 1;
	m_minY  = -1;
	m_maxY  = 1;
	m_type = mpLAYER_PLOT;
}

void mpFXYVector::Rewind()
{
	m_index = 0;
}

bool mpFXYVector::GetNextXY(double & x, double & y)
{
	if (m_index>=m_xs.size())
		return FALSE;
	else
	{
		x = m_xs[m_index];
		y = m_ys[m_index++];
		return m_index<=m_xs.size();
	}
}

void mpFXYVector::Clear()
{
	m_xs.clear();
	m_ys.clear();
}

void mpFXYVector::SetData( const std::vector<double> &xs,const std::vector<double> &ys)
{
	// Check if the data vectora are of the same size
	if (xs.size() != ys.size()) {
		wxLogError(_("wxMathPlot error: X and Y vector are not of the same length!"));
		return;
	}
	// Copy the data:
	m_xs = xs;
	m_ys = ys;


	// Update internal variables for the bounding box.
	if (xs.size()>0)
	{
		m_minX  = xs[0];
		m_maxX  = xs[0];
		m_minY  = ys[0];
		m_maxY  = ys[0];

		std::vector<double>::const_iterator  it;

		for (it=xs.begin();it!=xs.end();it++)
		{
			if (*it<m_minX) m_minX=*it;
			if (*it>m_maxX) m_maxX=*it;
		}
		for (it=ys.begin();it!=ys.end();it++)
		{
			if (*it<m_minY) m_minY=*it;
			if (*it>m_maxY) m_maxY=*it;
		}
		m_minX-=0.5f;
		m_minY-=0.5f;
		m_maxX+=0.5f;
		m_maxY+=0.5f;
	}
	else
	{
		m_minX  = -1;
		m_maxX  = 1;
		m_minY  = -1;
		m_maxY  = 1;
	}
}
//#pragma endregion

//#pragma region mpBAR
//-----------------------------------------------------------------------------
// mpBAR implentation (OCT-2022)
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpBAR, mpFXY)

// Constructor
mpBAR::mpBAR(wxString name, int flags ) : mpFXY(name,flags)
{
	m_index = 0;
	m_minX  = -1;
	m_maxX  = 1;
	m_minY  = -1;
	m_maxY  = 1;
	m_type = mpLAYER_PLOT;
}

void mpBAR::Rewind()
{
	m_index = 0;
}

bool mpBAR::GetNextXY(double & x, double & y)
{
	if (m_index>=m_xs.size())
		return FALSE;
	else
	{
		x = m_xs[m_index];
		y = m_ys[m_index++];
		return m_index<=m_xs.size();
	}
}

//wxRealPoint mpBAR::GetClosestXY(double x, double y, double scaleX, double scaleY){
//tuple<X value, Y value, y coordinate,>
std::tuple<double, double, double> mpBAR::GetClosestXY(double x, double y, double scaleX, double scaleY){
	double delta, closestX=0, closestY=0, closestYvalue;
	double xValue, yValue, yValue2;

	delta = 999999999999;
	std::map<double, std::tuple<double, double>>::iterator bc;
	for (bc = barCoordinates.begin(); bc != barCoordinates.end(); bc++)
	{
		xValue = bc->first;
		yValue = std::get<0>(bc->second);
		yValue2 = std::get<1>(bc->second);
		double thisPointDelta = std::min(sqrt(pow((x - xValue)*scaleX, 2) + pow((y - yValue)*scaleY, 2))
										, sqrt(pow((x - xValue)*scaleX, 2) + pow((y - yValue2)*scaleY, 2)));
		if (thisPointDelta < delta) {
			closestX = xValue;
			closestY = (std::get<0>(bc->second) + std::get<1>(bc->second))/2;//yValue;
			closestYvalue = yValue2 - yValue;
			delta = thisPointDelta;
		}
	}

	return {closestX, closestYvalue, closestY};//wxRealPoint(closestX, closestY);
}

double mpBAR::GetMaxY(){
	double max = 0;

	std::map<double, std::tuple<double, double>>::iterator bc;
	for (bc = barCoordinates.begin(); bc != barCoordinates.end(); bc++) {
		if (std::get<1>(bc->second) > max)
			max = std::get<1>(bc->second);
	}

	return max;
}

void mpBAR::Clear()
{
	m_xs.clear();
	m_ys.clear();
}

void mpBAR::SetData(const std::vector<double> &xs,const std::vector<double> &ys)
{
	// Check if the data vectora are of the same size
	if (xs.size() != ys.size()) {
		wxLogError(_("wxMathPlot error: X and Y vector are not of the same length!"));
		return;
	}
	// Copy the data:
	m_xs = xs;
	m_ys = ys;


	// Update internal variables for the bounding box.
	if (xs.size()>0)
	{
		m_minX  = xs[0];
		m_maxX  = xs[0];
		m_minY  = ys[0];
		m_maxY  = ys[0];

		std::vector<double>::const_iterator  it;

		for (it=xs.begin();it!=xs.end();it++)
		{
		    if (*it<m_minX) m_minX=*it;
		    if (*it>m_maxX) m_maxX=*it;
		}
		for (it=ys.begin();it!=ys.end();it++)
		{
		    if (*it<m_minY) m_minY=*it;
		    if (*it>m_maxY) m_maxY=*it;
		}
		m_minX-=0.5f;
		m_minY-=0.5f;
		m_maxX+=0.5f;
		m_maxY+=0.5f;
	}
	else
	{
		m_minX  = -1;
		m_maxX  = 1;
		m_minY  = -1;
		m_maxY  = 1;
	}
}

void mpBAR::Plot(wxDC & dc, mpWindow & w)
{

}
//#pragma endregion


//#pragma region mpText
//-----------------------------------------------------------------------------
// mpText - provided by Val Greene
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpText, mpLayer)


/** @param name text to be displayed
@param offsetx x position in percentage (0-100)
@param offsetx y position in percentage (0-100)
*/
mpText::mpText( wxString name, int offsetx, int offsety )
{
	SetName(name);

	if (offsetx >= 0 && offsetx <= 100)
		m_offsetx = offsetx;
	else
		m_offsetx = 5;

	if (offsety >= 0 && offsety <= 100)
		m_offsety = offsety;
	else
		m_offsetx = 50;
	m_type = mpLAYER_INFO;
}

//	mpText Layer plot handler.
//	This implementation will plot the text adjusted to the visible area.

void mpText::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen(m_pen);
		dc.SetFont(m_font);

		wxCoord tw=0, th=0;
		dc.GetTextExtent( GetName(), &tw, &th);

	//     int left = -dc.LogicalToDeviceX(0);
	//     int width = dc.LogicalToDeviceX(0) - left;
	//     int bottom = dc.LogicalToDeviceY(0);
	//     int height = bottom - -dc.LogicalToDeviceY(0);

	/*    dc.DrawText( GetName(),
		(int)((((float)width/100.0) * m_offsety) + left - (tw/2)),
		(int)((((float)height/100.0) * m_offsetx) - bottom) );*/
		int px = m_offsetx*(w.GetScrX() - w.GetMarginLeft() - w.GetMarginRight())/100;
		int py = m_offsety*(w.GetScrY() - w.GetMarginTop() - w.GetMarginBottom())/100;
		dc.DrawText( GetName(), px, py);
	}
}
//#pragma endregion

//#pragma region mpMarker
//-----------------------------------------------------------------------------
// mpMarker - provided by R1kk3r
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpMarker, mpLayer)


/** @param name text to be displayed
@param atX x absolute position
@param atY y absolute position
*/
mpMarker::mpMarker( wxString name, double atX, double atY )
{
	SetName(name);

	mX = atX;
	mY = atY;
}


void mpMarker::Plot(wxDC & dc, mpWindow & w)
{
	wxCoord     cx, cy, tw, th;
	wxColour    cc;
	wxString    ss;

	// setup

	dc.SetPen(m_pen);
	dc.SetFont(m_font);

	// part of setup is setting the text color

	cc = m_pen.GetColour();
	dc.SetTextForeground(cc);

	// what to draw

	ss = GetName();

	// where to draw

	dc.GetTextExtent(ss, &tw, &th);
	cx = (wxCoord) ((mX - w.GetPosX()) * w.GetScaleX()) - (tw / 2);
	cy = (wxCoord) ((w.GetPosY() - mY) * w.GetScaleY()) - (th / 2);

	// do it

	dc.DrawText( ss, cx, cy);
}
//#pragma endregion

//#pragma region mpPrintout
//-----------------------------------------------------------------------------
// mpPrintout - provided by Davide Rondini
//-----------------------------------------------------------------------------

mpPrintout::mpPrintout(mpWindow *drawWindow, const wxChar *title) : wxPrintout(title)
{
	drawn = false;
	plotWindow = drawWindow;
}

bool mpPrintout::OnPrintPage(int page)
{
	wxDC *trgDc = GetDC();
	if ((trgDc) && (page == 1)) {
		wxCoord m_prnX, m_prnY;
		int marginX = 50;
		int marginY = 50;
		trgDc->GetSize(&m_prnX, &m_prnY);

		m_prnX -= (2*marginX);
		m_prnY -= (2*marginY);
		trgDc->SetDeviceOrigin(marginX, marginY);

#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(wxT("Print Size: %d x %d\n"), m_prnX, m_prnY);
		wxLogMessage(wxT("Screen Size: %d x %d\n"), plotWindow->GetScrX(), plotWindow->GetScrY());
#endif

	// Set the scale according to the page:
		plotWindow->Fit(
						plotWindow->GetDesiredXmin(),
						plotWindow->GetDesiredXmax(),
						plotWindow->GetDesiredYmin(),
						plotWindow->GetDesiredYmax(),
						&m_prnX,
						&m_prnY );

		// Get the colours of the plotWindow to restore them at the end
		wxColour oldBgColour = plotWindow->GetBackgroundColour();
		wxColour oldFgColour = plotWindow->GetForegroundColour();
		wxColour oldAxColour = plotWindow->GetAxesColour();

		// Draw background, ensuring to use white background for printing.
		trgDc->SetPen( *wxTRANSPARENT_PEN );
		// wxBrush brush( plotWindow->GetBackgroundColour() );
		wxBrush brush = *wxWHITE_BRUSH;
		trgDc->SetBrush( brush );
		trgDc->DrawRectangle(0,0,m_prnX,m_prnY);

		// Draw all the layers:
		//trgDc->SetDeviceOrigin( m_prnX>>1, m_prnY>>1);  // Origin at the center
		mpLayer *layer;
		for (unsigned int li = 0; li < plotWindow->CountAllLayers(); li++) {
			layer = plotWindow->GetLayer(li);
			layer->Plot(*trgDc, *plotWindow);
		};
		// Restore device origin
		// trgDc->SetDeviceOrigin(0, 0);
		// Restore colours
		plotWindow->SetColourTheme(oldBgColour, oldFgColour, oldAxColour, oldAxColour);
		// Restore drawing
		plotWindow->Fit(plotWindow->GetDesiredXmin(), plotWindow->GetDesiredXmax(), plotWindow->GetDesiredYmin(), plotWindow->GetDesiredYmax(), NULL, NULL);
		plotWindow->UpdateAll();
	}
	return true;
}

bool mpPrintout::HasPage(int page)
{
	return (page == 1);
}
//#pragma endregion

//#pragma region mpMovableObject
//-----------------------------------------------------------------------------
// mpMovableObject - provided by Jose Luis Blanco
//-----------------------------------------------------------------------------
void mpMovableObject::TranslatePoint( double x,double y, double &out_x, double &out_y )
{
	double ccos = cos( m_reference_phi );  // Avoid computing cos/sin twice.
	double csin = sin( m_reference_phi );

	out_x = m_reference_x + ccos * x - csin * y;
	out_y = m_reference_y + csin * x + ccos * y;
}

// This method updates the buffers m_trans_shape_xs/ys, and the precomputed bounding box.
void mpMovableObject::ShapeUpdated()
{
	// Just in case...
	if (m_shape_xs.size()!=m_shape_ys.size())
	{
		wxLogError(wxT("[mpMovableObject::ShapeUpdated] Error, m_shape_xs and m_shape_ys have different lengths!"));
	}
	else
	{
		double ccos = cos( m_reference_phi );  // Avoid computing cos/sin twice.
		double csin = sin( m_reference_phi );

		m_trans_shape_xs.resize(m_shape_xs.size());
		m_trans_shape_ys.resize(m_shape_xs.size());

		std::vector<double>::iterator itXi, itXo;
		std::vector<double>::iterator itYi, itYo;

		m_bbox_min_x=1e300;
		m_bbox_max_x=-1e300;
		m_bbox_min_y=1e300;
		m_bbox_max_y=-1e300;

		for (itXo=m_trans_shape_xs.begin(),itYo=m_trans_shape_ys.begin(),itXi=m_shape_xs.begin(),itYi=m_shape_ys.begin();
			  itXo!=m_trans_shape_xs.end(); itXo++,itYo++,itXi++,itYi++)
		{
			*itXo = m_reference_x + ccos * (*itXi) - csin * (*itYi);
			*itYo = m_reference_y + csin * (*itXi) + ccos * (*itYi);

			// Keep BBox:
			if (*itXo < m_bbox_min_x) m_bbox_min_x = *itXo;
			if (*itXo > m_bbox_max_x) m_bbox_max_x = *itXo;
			if (*itYo < m_bbox_min_y) m_bbox_min_y = *itYo;
			if (*itYo > m_bbox_max_y) m_bbox_max_y = *itYo;
		}
	}
}

void mpMovableObject::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible) {
		dc.SetPen( m_pen);

		std::vector<double>::iterator  itX=m_trans_shape_xs.begin();
		std::vector<double>::iterator  itY=m_trans_shape_ys.begin();

		if (!m_continuous)
		{
			// for some reason DrawPoint does not use the current pen,
			// so we use DrawLine for fat pens
			if (m_pen.GetWidth() <= 1)
			{
				while (itX!=m_trans_shape_xs.end())
				{
					dc.DrawPoint( w.x2p(*(itX++)), w.y2p( *(itY++) ) );
				}
			}
			else
			{
				while (itX!=m_trans_shape_xs.end())
				{
					wxCoord cx = w.x2p(*(itX++));
					wxCoord cy = w.y2p(*(itY++));
					dc.DrawLine(cx, cy, cx, cy);
				}
			}
		}
		else
		{
			wxCoord cx0=0,cy0=0;
			bool    first = TRUE;
			while (itX!=m_trans_shape_xs.end())
			{
				wxCoord cx = w.x2p(*(itX++));
				wxCoord cy = w.y2p(*(itY++));
				if (first)
				{
					first=FALSE;
					cx0=cx;cy0=cy;
				}
				dc.DrawLine(cx0, cy0, cx, cy);
				if (m_contpoints) {
					dc.DrawCircle(cx,cy,m_pen.GetWidth()*1.25);
				}
				cx0=cx; cy0=cy;
			}
		}

		if (!m_name.IsEmpty() && m_showName)
		{
			dc.SetFont(m_font);

			wxCoord tx, ty;
			dc.GetTextExtent(m_name, &tx, &ty);

			if (HasBBox())
			{
				wxCoord sx = (wxCoord) (( m_bbox_max_x - w.GetPosX()) * w.GetScaleX());
				wxCoord sy = (wxCoord) ((w.GetPosY() - m_bbox_max_y ) * w.GetScaleY());

				tx = sx - tx - 8;
				ty = sy - 8 - ty;
			}
			else
			{
				const int sx = w.GetScrX()>>1;
				const int sy = w.GetScrY()>>1;

				if ((m_flags & mpALIGNMASK) == mpALIGN_NE)
				{
					tx = sx - tx - 8;
					ty = -sy + 8;
				}
				else if ((m_flags & mpALIGNMASK) == mpALIGN_NW)
				{
					tx = -sx + 8;
					ty = -sy + 8;
				}
				else if ((m_flags & mpALIGNMASK) == mpALIGN_SW)
				{
					tx = -sx + 8;
					ty = sy - 8 - ty;
				}
				else
				{
					tx = sx - tx - 8;
					ty = sy - 8 - ty;
				}
			}

			dc.DrawText( m_name, tx, ty);
		}
	}
}
//#pragma endregion

//#pragma region mpCovarianceEllipse
//-----------------------------------------------------------------------------
// mpCovarianceEllipse - provided by Jose Luis Blanco
//-----------------------------------------------------------------------------

// Called to update the m_shape_xs, m_shape_ys vectors, whenever a parameter changes.
void mpCovarianceEllipse::RecalculateShape()
{
	m_shape_xs.clear();
	m_shape_ys.clear();

	// Preliminar checks:
	if (m_quantiles<0)  { wxLogError(wxT("[mpCovarianceEllipse] Error: quantiles must be non-negative")); return; }
	if (m_cov_00<0)     { wxLogError(wxT("[mpCovarianceEllipse] Error: cov(0,0) must be non-negative")); return; }
	if (m_cov_11<0)     { wxLogError(wxT("[mpCovarianceEllipse] Error: cov(1,1) must be non-negative")); return; }

	m_shape_xs.resize( m_segments,0 );
	m_shape_ys.resize( m_segments,0 );

	// Compute the two eigenvalues of the covariance:
	// -------------------------------------------------
	double b = -m_cov_00 - m_cov_11;
	double c = m_cov_00*m_cov_11 - m_cov_01*m_cov_01;

	double D = b*b - 4*c;

	if (D<0)     { wxLogError(wxT("[mpCovarianceEllipse] Error: cov is not positive definite")); return; }

	double eigenVal0 =0.5*( -b + sqrt(D) );
	double eigenVal1 =0.5*( -b - sqrt(D) );

	// Compute the two corresponding eigenvectors:
	// -------------------------------------------------
	double  eigenVec0_x,eigenVec0_y;
	double  eigenVec1_x,eigenVec1_y;

	if (fabs(eigenVal0 - m_cov_00)>1e-6)
	{
		double k1x = m_cov_01 / ( eigenVal0 - m_cov_00 );
		eigenVec0_y = 1;
		eigenVec0_x = eigenVec0_y * k1x;
	}
	else
	{
		double k1y = m_cov_01 / ( eigenVal0 - m_cov_11 );
		eigenVec0_x = 1;
		eigenVec0_y = eigenVec0_x * k1y;
	}

	if (fabs(eigenVal1 - m_cov_00)>1e-6)
	{
		double k2x = m_cov_01 / ( eigenVal1 - m_cov_00 );
		eigenVec1_y = 1;
		eigenVec1_x = eigenVec1_y * k2x;
	}
	else
	{
		double k2y = m_cov_01 / ( eigenVal1 - m_cov_11 );
		eigenVec1_x = 1;
		eigenVec1_y = eigenVec1_x * k2y;
	}

	// Normalize the eigenvectors:
	double len = sqrt( eigenVec0_x*eigenVec0_x + eigenVec0_y*eigenVec0_y );
	eigenVec0_x /= len;  // It *CANNOT* be zero
	eigenVec0_y /= len;

	len = sqrt( eigenVec1_x*eigenVec1_x + eigenVec1_y*eigenVec1_y );
	eigenVec1_x /= len;  // It *CANNOT* be zero
	eigenVec1_y /= len;


	// Take the sqrt of the eigenvalues (required for the ellipse scale):
	eigenVal0 = sqrt(eigenVal0);
	eigenVal1 = sqrt(eigenVal1);

	// Compute the 2x2 matrix M = diag(eigVal) * (~eigVec)  (each eigen vector is a row):
	double M_00 = eigenVec0_x * eigenVal0;
	double M_01 = eigenVec0_y * eigenVal0;

	double M_10 = eigenVec1_x * eigenVal1;
	double M_11 = eigenVec1_y * eigenVal1;

	// The points of the 2D ellipse:
	double ang;
	double Aang = 6.283185308/(m_segments-1);
	int    i;
	for (i=0,ang=0;i<m_segments;i++,ang+= Aang )
	{
		double ccos = cos(ang);
		double csin = sin(ang);

		m_shape_xs[i] = m_quantiles * (ccos * M_00 + csin * M_10 );
		m_shape_ys[i] = m_quantiles * (ccos * M_01 + csin * M_11 );
	} // end for points on ellipse

	ShapeUpdated();
}
//#pragma endregion

//#pragma region mpPolygon
//-----------------------------------------------------------------------------
// mpPolygon - provided by Jose Luis Blanco
//-----------------------------------------------------------------------------
void mpPolygon::setPoints(
	const std::vector<double>&  points_xs,
	const std::vector<double>&  points_ys,
	bool                        closedShape )
{
	if ( points_xs.size()!=points_ys.size() )
	{
		wxLogError(wxT("[mpPolygon] Error: points_xs and points_ys must have the same number of elements"));
	}
	else
	{
		m_shape_xs = points_xs;
		m_shape_ys = points_ys;

		if ( closedShape && points_xs.size())
		{
			m_shape_xs.push_back( points_xs[0] );
			m_shape_ys.push_back( points_ys[0] );
		}

		ShapeUpdated();
	}
}
//#pragma endregion

//#pragma region mpBitmapLayer
//-----------------------------------------------------------------------------
// mpBitmapLayer - provided by Jose Luis Blanco
//-----------------------------------------------------------------------------
void mpBitmapLayer::GetBitmapCopy( wxImage &outBmp ) const
{
	if (m_validImg)
		outBmp = m_bitmap;
}

void mpBitmapLayer::SetBitmap( const wxImage &inBmp, double x, double y, double lx, double ly )
{
	if (!inBmp.Ok())
	{
		wxLogError(wxT("[mpBitmapLayer] Assigned bitmap is not Ok()!"));
	}
	else
	{
		m_bitmap = inBmp; //.GetSubBitmap( wxRect(0, 0, inBmp.GetWidth(), inBmp.GetHeight()));
		m_min_x = x;
		m_min_y = y;
		m_max_x = x+lx;
		m_max_y = y+ly;
		m_validImg = true;
	}
}

void mpBitmapLayer::Plot(wxDC & dc, mpWindow & w)
{
	if (m_visible && m_validImg)
	{
	/*    1st: We compute (x0,y0)-(x1,y1), the pixel coordinates of the real outer limits
		     of the image rectangle within the (screen) mpWindow. Note that these coordinates
		     might fall well far away from the real view limits when the user zoom in.

		2nd: We compute (dx0,dy0)-(dx1,dy1), the pixel coordinates the rectangle that will
		     be actually drawn into the mpWindow, i.e. the clipped real rectangle that
		     avoids the non-visible parts. (offset_x,offset_y) are the pixel coordinates
		     that correspond to the window point (dx0,dy0) within the image "m_bitmap", and
		     (b_width,b_height) is the size of the bitmap patch that will be drawn.

	(x0,y0) .................  (x1,y0)
		.                          .
		.                          .
	(x0,y1) ................   (x1,y1)
		          (In pixels!!)
	*/

		// 1st step -------------------------------
		wxCoord x0 = w.x2p(m_min_x);
		wxCoord y0 = w.y2p(m_max_y);
		wxCoord x1 = w.x2p(m_max_x);
		wxCoord y1 = w.y2p(m_min_y);

		// 2nd step -------------------------------
		// Precompute the size of the actual bitmap pixel on the screen (e.g. will be >1 if zoomed in)
		double screenPixelX = ( x1-x0 ) / (double)m_bitmap.GetWidth();
		double screenPixelY = ( y1-y0 ) / (double)m_bitmap.GetHeight();

	// The minimum number of pixels that the streched image will overpass the actual mpWindow borders:
		wxCoord borderMarginX = (wxCoord)(screenPixelX+1); // ceil
		wxCoord borderMarginY = (wxCoord)(screenPixelY+1); // ceil

		// The actual drawn rectangle (dx0,dy0)-(dx1,dy1) is (x0,y0)-(x1,y1) clipped:
		wxCoord dx0=x0,dx1=x1,dy0=y0,dy1=y1;
		if (dx0<0) dx0=-borderMarginX;
		if (dy0<0) dy0=-borderMarginY;
		if (dx1>w.GetScrX()) dx1=w.GetScrX()+borderMarginX;
		if (dy1>w.GetScrY()) dy1=w.GetScrY()+borderMarginY;

		// For convenience, compute the width/height of the rectangle to be actually drawn:
		wxCoord d_width = dx1-dx0+1;
		wxCoord d_height = dy1-dy0+1;

		// Compute the pixel offsets in the internally stored bitmap:
		wxCoord offset_x= (wxCoord) ( (dx0-x0)/screenPixelX );
		wxCoord offset_y= (wxCoord) ( (dy0-y0)/screenPixelY );

		// and the size in pixel of the area to be actually drawn from the internally stored bitmap:
		wxCoord b_width  = (wxCoord) ( (dx1-dx0+1)/screenPixelX );
		wxCoord b_height = (wxCoord) ( (dy1-dy0+1)/screenPixelY );

#ifdef MATHPLOT_DO_LOGGING
		wxLogMessage(_("[mpBitmapLayer::Plot] screenPixel: x=%f y=%f  d_width=%ix%i"), screenPixelX, screenPixelY, d_width,d_height);
		wxLogMessage(_("[mpBitmapLayer::Plot] offset: x=%i y=%i  bmpWidth=%ix%i"),offset_x,offset_y, b_width, b_height);
#endif

		// Is there any visible region?
		if (d_width>0 && d_height>0)
		{
			// Build the scaled bitmap from the image, only if it has changed:
			if (m_scaledBitmap.GetWidth()!=d_width ||
				m_scaledBitmap.GetHeight()!=d_height ||
				m_scaledBitmap_offset_x != offset_x ||
				m_scaledBitmap_offset_y != offset_y  )
			{
				wxRect r(wxRect(offset_x,offset_y,b_width,b_height));
				// Just for the case....
				if (r.x<0) r.x=0;
				if (r.y<0) r.y=0;
				if (r.width>m_bitmap.GetWidth()) r.width=m_bitmap.GetWidth();
				if (r.height>m_bitmap.GetHeight()) r.height=m_bitmap.GetHeight();

				m_scaledBitmap = wxBitmap(
					wxBitmap(m_bitmap).GetSubBitmap( r ).ConvertToImage()
					.Scale(d_width,d_height) );
					m_scaledBitmap_offset_x = offset_x;
					m_scaledBitmap_offset_y = offset_y;
			}

			// Draw it:
			dc.DrawBitmap( m_scaledBitmap, dx0,dy0, true );
		}
	}

	// Draw the name label
	if (!m_name.IsEmpty() && m_showName)
	{
		dc.SetFont(m_font);

		wxCoord tx, ty;
		dc.GetTextExtent(m_name, &tx, &ty);

		if (HasBBox())
		{
			wxCoord sx = (wxCoord) (( m_max_x - w.GetPosX()) * w.GetScaleX());
			wxCoord sy = (wxCoord) ((w.GetPosY() - m_max_y ) * w.GetScaleY());

			tx = sx - tx - 8;
			ty = sy - 8 - ty;
		}
		else
		{
			const int sx = w.GetScrX()>>1;
			const int sy = w.GetScrY()>>1;

			if ((m_flags & mpALIGNMASK) == mpALIGN_NE)
			{
				tx = sx - tx - 8;
				ty = -sy + 8;
			}
			else if ((m_flags & mpALIGNMASK) == mpALIGN_NW)
			{
				tx = -sx + 8;
				ty = -sy + 8;
			}
			else if ((m_flags & mpALIGNMASK) == mpALIGN_SW)
			{
				tx = -sx + 8;
				ty = sy - 8 - ty;
			}
			else
			{
				tx = sx - tx - 8;
				ty = sy - 8 - ty;
			}
		}

		dc.DrawText( m_name, tx, ty);
	}
}
//#pragma endregion
