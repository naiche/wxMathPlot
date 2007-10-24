/////////////////////////////////////////////////////////////////////////////
// Name:            mathplot.cpp
// Purpose:         Framework for plotting in wxWindows
// Original Author: David Schalig
// Maintainer:      Davide Rondini
// Contributors:    Jose Luis Blanco, Val Greene
// Created:         21/07/2003
// Last edit:       09/09/2007
// Copyright:       (c) David Schalig, Davide Rondini
// Licence:         wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
// #pragma implementation "plot.h"
#pragma implementation "mathplot.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include <wx/window.h>
//#include <wx/wxprec.h>

// Comment out for release operation:
// (Added by J.L.Blanco, Aug 2007)
//#define MATHPLOT_DO_LOGGING

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

#include <cmath>
#include <cstdio> // used only for debug

//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpLayer, wxObject)

mpLayer::mpLayer()
{
    SetPen((wxPen&) *wxBLACK_PEN);
    SetFont((wxFont&) *wxNORMAL_FONT);
    m_continuous = FALSE; // Default
    m_showName   = TRUE;  // Default
}

//-----------------------------------------------------------------------------
// mpLayer implementations - functions
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpFX, mpLayer)

mpFX::mpFX(wxString name, int flags)
{
    SetName(name);
    m_flags = flags;
}

void mpFX::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    if (m_pen.GetWidth() <= 1)
    {
        for (wxCoord i = -(w.GetScrX()>>1); i < (w.GetScrX()>>1); ++i)
        {
            dc.DrawPoint(i, (wxCoord) ((w.GetPosY() - GetY( (double)i / w.GetScaleX() + w.GetPosX()) ) * w.GetScaleY()));
        }
    }
    else
    {
        for (wxCoord i = -(w.GetScrX()>>1); i < (w.GetScrX()>>1); ++i)
        {
            wxCoord c = (wxCoord) ((w.GetPosY() - GetY( (double)i / w.GetScaleX() + w.GetPosX()) ) * w.GetScaleY());
            dc.DrawLine( i, c, i, c);
        }
    }

    if (!m_name.IsEmpty() && m_showName)
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        if ((m_flags & mpALIGNMASK) == mpALIGN_RIGHT)
            tx = (w.GetScrX()>>1) - tx - 8;
        else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
            tx = -tx/2;
        else
            tx = -(w.GetScrX()>>1) + 8;

        dc.DrawText( m_name, tx, (wxCoord) ((w.GetPosY() - GetY( (double)tx / w.GetScaleX() + w.GetPosX())) * w.GetScaleY()) );
    }
}

IMPLEMENT_ABSTRACT_CLASS(mpFY, mpLayer)

mpFY::mpFY(wxString name, int flags)
{
    SetName(name);
    m_flags = flags;
}

void mpFY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    wxCoord i;

    if (m_pen.GetWidth() <= 1)
    {
        for (i = -(w.GetScrY()>>1); i < (w.GetScrY()>>1); ++i)
        {
            dc.DrawPoint((wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX()), -i);
        }
    }
    else
    {
        for (i = -(w.GetScrY()>>1); i < (w.GetScrY()>>1); ++i)
        {
            wxCoord c =  (wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX());
            dc.DrawLine(c, -i, c, -i);
        }
    }

    if (!m_name.IsEmpty() && m_showName)
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        if ((m_flags & mpALIGNMASK) == mpALIGN_TOP)
            ty = (w.GetScrY()>>1) - 8;
        else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
            ty = 16 - ty/2;
        else
            ty = -(w.GetScrY()>>1) + 8;

        dc.DrawText( m_name, (wxCoord) ((GetX( (double)i / w.GetScaleY() + w.GetPosY()) - w.GetPosX()) * w.GetScaleX()), -ty);
    }
}

IMPLEMENT_ABSTRACT_CLASS(mpFXY, mpLayer)

mpFXY::mpFXY(wxString name, int flags)
{
    SetName(name);
    m_flags = flags;
}

void mpFXY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);

    double x, y;
    Rewind();

    if (!m_continuous)
    {
        // for some reason DrawPoint does not use the current pen,
        // so we use DrawLine for fat pens
        if (m_pen.GetWidth() <= 1)
        {
            while (GetNextXY(x, y))
            {
                dc.DrawPoint( (wxCoord) ((x - w.GetPosX()) * w.GetScaleX()) ,
                              (wxCoord) ((w.GetPosY() - y) * w.GetScaleY()) );
            }
        }
        else
        {
            while (GetNextXY(x, y))
            {
                wxCoord cx = (wxCoord) ((x - w.GetPosX()) * w.GetScaleX());
                wxCoord cy = (wxCoord) ((w.GetPosY() - y) * w.GetScaleY());
                dc.DrawLine(cx, cy, cx, cy);
            }
        }
    }
    else
    {
        wxCoord cx0=0,cy0=0;
        bool    first = TRUE;
        while (GetNextXY(x, y))
        {
            wxCoord cx = (wxCoord) ((x - w.GetPosX()) * w.GetScaleX());
            wxCoord cy = (wxCoord) ((w.GetPosY() - y) * w.GetScaleY());
            if (first)
            {
                first=FALSE;
                cx0=cx;cy0=cy;
            }
            dc.DrawLine(cx0, cy0, cx, cy);
            cx0=cx; cy0=cy;
        }
    }

    if (!m_name.IsEmpty() && m_showName)
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        // xxx implement else ... if (!HasBBox())
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

//-----------------------------------------------------------------------------
// mpProfile implementation
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpProfile, mpLayer)

mpProfile::mpProfile(wxString name, int flags)
{
    SetName(name);
    m_flags = flags;
}

void mpProfile::Plot(wxDC & dc, mpWindow & w)
{
   dc.SetPen( m_pen);

   // Plot profile linking subsequent point of the profile, instead of mpFY, which plots simple points.
   for (wxCoord i = -(w.GetScrX()>>1); i < (w.GetScrX()>>1)-1; ++i)
 	{
        wxCoord c0 = (wxCoord) ((w.GetYpos() - GetY( (double)i / w.GetXscl() + w.GetXpos()) ) * w.GetYscl());
	wxCoord c1 = (wxCoord) ((w.GetYpos() - GetY( (double)(i+1) / w.GetXscl() + (w.GetXpos() ) ) ) * w.GetYscl());
	dc.DrawLine(i, c0, i+1, c1);
        };
    if (!m_name.IsEmpty())
    {
        dc.SetFont(m_font);

        wxCoord tx, ty;
        dc.GetTextExtent(m_name, &tx, &ty);

        if ((m_flags & mpALIGNMASK) == mpALIGN_RIGHT)
            tx = (w.GetScrX()>>1) - tx - 8;
        else if ((m_flags & mpALIGNMASK) == mpALIGN_CENTER)
            tx = -tx/2;
        else
            tx = -(w.GetScrX()>>1) + 8;

        dc.DrawText( m_name, tx, (wxCoord) ((w.GetPosY() - GetY( (double)tx / w.GetScaleX() + w.GetPosX())) * w.GetScaleY()) );
    }
}

//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

#define mpLN10 2.3025850929940456840179914546844

IMPLEMENT_CLASS(mpScaleX, mpLayer)

mpScaleX::mpScaleX(wxString name, int flags, bool ticks)
{
    SetName(name);
    SetFont( (wxFont&) *wxSMALL_FONT);
    SetPen( (wxPen&) *wxGREY_PEN);
    m_flags = flags;
    m_ticks = ticks;
}

void mpScaleX::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);
    int orgy=0;

    const int extend = w.GetScrX()/2;
    if (m_flags == mpALIGN_CENTER)
       orgy   = (int)(w.GetPosY() * w.GetScaleY());
    if (m_flags == mpALIGN_TOP)
       orgy   = -(w.GetScrY()/2 - 10);
    if (m_flags == mpALIGN_BOTTOM)
       orgy   = (w.GetScrY()/2 - 25);
    if (m_flags == mpALIGN_BORDER_BOTTOM )
       orgy = dc.LogicalToDeviceY(0) - 1;
    if (m_flags == mpALIGN_BORDER_TOP )
       orgy = -dc.LogicalToDeviceY(0);

    dc.DrawLine( -extend, orgy, extend, orgy);

    const double dig  = floor( log( 128.0 / w.GetScaleX() ) / mpLN10 );
    const double step = exp( mpLN10 * dig);
    const double end  = w.GetPosX() + (double)extend / w.GetScaleX();

    wxCoord tx, ty;
    wxString s;
    wxString fmt;
    int tmp = (int)dig;
    if (tmp>=1) {
        fmt = wxT("%.f");
    } else {
        tmp=8-tmp;
        fmt.Printf(wxT("%%.%df"), tmp >= -1 ? 2 : -tmp);
    }

    double n = floor( (w.GetPosX() - (double)extend / w.GetScaleX()) / step ) * step ;

    tmp=-65535;
    for (;n < end; n += step)
    {
        const int p = (int)((n - w.GetPosX()) * w.GetScaleX());
        if (m_ticks) { // draw axis ticks
            if (m_flags == mpALIGN_BORDER_BOTTOM)
                dc.DrawLine( p, orgy, p, orgy-4);
            else
                dc.DrawLine( p, orgy, p, orgy+4);
        } else { // draw grid dotted lines
            m_pen.SetStyle(wxDOT);
            dc.SetPen(m_pen);
            dc.DrawLine( p, -w.GetScrY(), p, w.GetScrY() );
            m_pen.SetStyle(wxSOLID);
            dc.SetPen(m_pen);
        }


        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((p-tx/2-tmp) > 64) {
            if (m_flags == mpALIGN_BORDER_BOTTOM)
                dc.DrawText( s, p-tx/2, orgy-4-ty);
            else
                dc.DrawText( s, p-tx/2, orgy+4);
            tmp=p+tx/2;
        }
    }

    if (m_flags != mpALIGN_TOP) {
        dc.GetTextExtent(m_name, &tx, &ty);
        if (m_flags == mpALIGN_BORDER_BOTTOM) {
            dc.DrawText( m_name, extend - tx - 4, orgy - 4 - (ty*2));
        } else {
            dc.DrawText( m_name, extend - tx - 4, orgy - 4 - ty); //orgy + 4 + ty);
        }
    };
}

IMPLEMENT_CLASS(mpScaleY, mpLayer)

mpScaleY::mpScaleY(wxString name, int flags, bool ticks)
{
    SetName(name);
    SetFont( (wxFont&) *wxSMALL_FONT);
    SetPen( (wxPen&) *wxGREY_PEN);
    m_flags = flags;
    m_ticks = ticks;
}

void mpScaleY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);

    int orgx=0;
    const int extend = w.GetScrY()/2;
    if (m_flags == mpALIGN_CENTER)
        orgx   = -(int)(w.GetPosX() * w.GetScaleX());
    if (m_flags == mpALIGN_LEFT)
        orgx   = -(w.GetScrX()/2 - 40);
    if (m_flags == mpALIGN_RIGHT)
        orgx   = (w.GetScrX()/2 - 10);
    if (m_flags == mpALIGN_BORDER_RIGHT )
        orgx = dc.LogicalToDeviceX(0) - 1;
    if (m_flags == mpALIGN_BORDER_LEFT )
        orgx = -dc.LogicalToDeviceX(0);


    dc.DrawLine( orgx, -extend, orgx, extend);

    const double dig  = floor( log( 128.0 / w.GetScaleY() ) / mpLN10 );
    const double step = exp( mpLN10 * dig);
    const double end  = w.GetPosY() + (double)extend / w.GetScaleY();

    wxCoord tx, ty;
    wxString s;
    wxString fmt;
    int tmp = (int)dig;
    if (tmp>=1)
    {
        fmt = wxT("%.f");
    }
    else
    {
        tmp=8-tmp;
        fmt.Printf(wxT("%%.%df"), tmp >= -1 ? 2 : -tmp);
    }

    double n = floor( (w.GetPosY() - (double)extend / w.GetScaleY()) / step ) * step ;

    tmp=65536;
    for (;n < end; n += step)
    {
        const int p = (int)((w.GetPosY() - n) * w.GetScaleY());
        if (m_ticks) { // Draw axis ticks
            if (m_flags == mpALIGN_BORDER_LEFT) {
                dc.DrawLine( orgx, p, orgx+4, p);
            } else {
                dc.DrawLine( orgx-4, p, orgx, p); //( orgx, p, orgx+4, p);
            }
        } else {
            m_pen.SetStyle(wxDOT);
            dc.SetPen( m_pen);
            dc.DrawLine( -w.GetScrX(), p, w.GetScrX(), p);
            m_pen.SetStyle(wxSOLID);
            dc.SetPen( m_pen);
        }


        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((tmp-p+ty/2) > 32)
        {
            if (m_flags == mpALIGN_BORDER_LEFT)
                dc.DrawText( s, orgx+4, p-ty/2);
            else
                dc.DrawText( s, orgx-4-tx, p-ty/2); //( s, orgx+4, p-ty/2);
            tmp=p-ty/2;
        }
    }

    if (m_flags != mpALIGN_RIGHT) {
	dc.GetTextExtent(m_name, &tx, &ty);
	if (m_flags == mpALIGN_BORDER_LEFT) {
            dc.DrawText( m_name, orgx-tx-4, -extend + ty + 4);
        } else {
            if (m_flags == mpALIGN_BORDER_RIGHT )
                dc.DrawText( m_name, orgx-(tx*2)-4, -extend + ty + 4);
            else
                dc.DrawText( m_name, orgx + 4, -extend + 4);
        }
    };
}

//-----------------------------------------------------------------------------
// mpWindow
//-----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(mpWindow, wxScrolledWindow)

BEGIN_EVENT_TABLE(mpWindow, wxScrolledWindow)
    EVT_PAINT    ( mpWindow::OnPaint)
    EVT_SIZE     ( mpWindow::OnSize)
    EVT_SCROLLWIN( mpWindow::OnScroll2)

    EVT_MIDDLE_UP( mpWindow::OnShowPopupMenu)
    EVT_RIGHT_DOWN( mpWindow::OnMouseRightDown) // JLB
    EVT_RIGHT_UP ( mpWindow::OnShowPopupMenu)
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

mpWindow::mpWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int flag )
    : wxScrolledWindow( parent, id, pos, size, flag, wxT("mathplot") )
{
    m_scaleX = m_scaleY = 1.0;
    m_posX   = m_posY   = 0;
    m_scrX   = m_scrX   = 64;
    m_minX   = m_minY   = 0;
    m_maxX   = m_maxY   = 0;
    m_last_lx= m_last_ly= 0;
    m_buff_bmp = NULL;
    m_enableDoubleBuffer        = FALSE;
    m_enableMouseNavigation     = TRUE;
    m_mouseMovedAfterRightClick = FALSE;

    m_lockaspect = FALSE;

    m_popmenu.Append( mpID_CENTER,     _("Center"),      _("Center plot view to this position"));
    m_popmenu.Append( mpID_FIT,        _("Fit"),         _("Set plot view to show all items"));
    m_popmenu.Append( mpID_ZOOM_IN,    _("Zoom in"),     _("Zoom in plot view."));
    m_popmenu.Append( mpID_ZOOM_OUT,   _("Zoom out"),    _("Zoom out plot view."));
    m_popmenu.AppendCheckItem( mpID_LOCKASPECT, _("Lock aspect"), _("Lock horizontal and vertical zoom aspect."));
    m_popmenu.Append( mpID_HELP_MOUSE,   _("Show mouse commands..."),    _("Show help about the mouse commands."));

    m_layers.clear();
    SetBackgroundColour( *wxWHITE );
    EnableScrolling(FALSE, FALSE);
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

// Mouse handler, for detecting when the user drag with the right button or just "clicks" for the menu
// JLB
void mpWindow::OnMouseRightDown(wxMouseEvent     &event)
{
    m_mouseMovedAfterRightClick = FALSE;
    m_mouseRClick_X = event.GetX();
    m_mouseRClick_Y = event.GetY();
    if (m_enableMouseNavigation)
    {
        SetCursor( *wxCROSS_CURSOR );
    }
}

// Process mouse wheel events
// JLB
void mpWindow::OnMouseWheel( wxMouseEvent &event )
{
    if (!m_enableMouseNavigation)
    {
        event.Skip();
        return;
    }

    int width, height;
    GetClientSize( &width, &height);

    if (event.m_controlDown)
    {
        // CTRL key hold: Zoom in/out:
        if (event.GetWheelRotation()>0)
                ZoomIn();
        else    ZoomOut();
    }
    else
    {
        // Scroll vertically or horizontally (this is SHIFT is hold down).
        int change = - event.GetWheelRotation(); // Opposite direction (More intuitive)!
        float changeUnitsX = change / m_scaleX;
        float changeUnitsY = change / m_scaleY;

        if (event.m_shiftDown)
                m_posX += changeUnitsX;
        else    m_posY -= changeUnitsY;

        UpdateAll();
    }
}

// If the user "drags" with the right buttom pressed, do "pan"
// JLB
void mpWindow::OnMouseMove(wxMouseEvent     &event)
{
    if (!m_enableMouseNavigation)
    {
        event.Skip();
        return;
    }

    if (event.m_rightDown)
    {
        m_mouseMovedAfterRightClick = TRUE;  // Hides the popup menu after releasing the button!

        // The change:
        int  Ax= m_mouseRClick_X - event.GetX();
        int  Ay= m_mouseRClick_Y - event.GetY();

        // For the next event, use relative to this coordinates.
        m_mouseRClick_X = event.GetX();
        m_mouseRClick_Y = event.GetY();

        float   Ax_units = Ax / m_scaleX;
        float   Ay_units = -Ay / m_scaleY;

        m_posX += Ax_units;
        m_posY += Ay_units;

        UpdateAll();

#ifdef MATHPLOT_DO_LOGGING
        wxLogMessage(_("[mpWindow::OnMouseMove] Ax:%i Ay:%i"),Ax,Ay);
#endif
    } else {
        if (event.m_leftDown) {
            wxPaintDC dc(this);
            wxPen pen(*wxLIGHT_GREY, 1, wxDOT);
            dc.SetPen(pen);
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.DrawRectangle(m_mouseLClick_X, m_mouseLClick_Y, abs(m_mouseLClick_X - event.GetX()), abs(m_mouseLClick_Y - event.GetY()));
            UpdateAll();
        }
    }
    event.Skip();
}

void mpWindow::OnMouseLeftDown (wxMouseEvent &event)
{
    m_mouseLClick_X = event.GetX();
    m_mouseLClick_Y = event.GetY();
#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::OnMouseLeftDown() X = %d , Y = %d"), event.GetX(), event.GetY());/*m_mouseLClick_X, m_mouseLClick_Y);*/
#endif
    event.Skip();
}

void mpWindow::OnMouseLeftRelease (wxMouseEvent &event)
{
    wxPoint release(event.GetX(), event.GetY());
    wxPoint press(m_mouseLClick_X, m_mouseLClick_Y);
    if (release != press) {
        ZoomRect(press, release);
    }
    event.Skip();
}

void mpWindow::Fit()
{
    if (UpdateBBox())
    {
        int cx, cy;
        GetClientSize( &cx, &cy);

        double d;
        d = m_maxX - m_minX;
        if (d!=0)
        {
            m_scaleX = cx/d;
#ifdef MATHPLOT_DO_LOGGING
            wxLogMessage(_("mpWindow::Fit() m_scaleX = %f , cx = %d, d = %f"), m_scaleX, cx, d);
#endif
            m_posX = m_minX + d/2;
        }
        d = m_maxY - m_minY;
        if (d!=0)
        {
            m_scaleY = cy/d;
            m_posY = m_minY + d/2;
        }

        if (m_lockaspect)
        {
            double s = (m_scaleX + m_scaleY)/2;
            m_scaleX = s;
#ifdef MATHPLOT_DO_LOGGING
            wxLogMessage(_("mpWindow::Fit(lock) m_scaleX = %f"), m_scaleX);
#endif
            m_scaleY = s;
        }

        UpdateAll();
    }
}

void mpWindow::ZoomIn()
{
    m_scaleX = m_scaleX * 2;
    m_scaleY = m_scaleY * 2;

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomIn() m_scaleX = %f"), m_scaleX);
#endif

    UpdateAll();

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomIn() m_scaleX(Updated) = %f"), m_scaleX);
#endif
}

void mpWindow::ZoomOut()
{
    m_scaleX = m_scaleX / 2;
    m_scaleY = m_scaleY / 2;

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomOut() m_scaleX = %f"), m_scaleX);
#endif
    UpdateAll();

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomOut() m_scaleX(Updated) = %f"), m_scaleX);
#endif
}

void mpWindow::ZoomInX()
{
    m_scaleX = m_scaleX * 2;
    UpdateAll();
}

void mpWindow::ZoomOutX()
{
    m_scaleX = m_scaleX / 2;
    UpdateAll();
}

void mpWindow::ZoomInY()
{
    m_scaleY = m_scaleY * 2;
    UpdateAll();
}

void mpWindow::ZoomOutY()
{
    m_scaleY = m_scaleY / 2;
    UpdateAll();
}


void mpWindow::ZoomRect(wxPoint p0, wxPoint p1)
{
    int deltaX = abs(p0.x - p1.x);
    int deltaY = abs(p0.y - p1.y);
    double scaleFactorX = (double) deltaX/m_scrX;
    double scaleFactorY = (double) deltaY/m_scrY;
    double scaleFactor = (scaleFactorX > scaleFactorY) ? scaleFactorX : scaleFactorY;
#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomRect() scaleX = %f, scaleY = %f, Scale Factor = %f"), scaleFactorX, scaleFactorY, scaleFactor);
#endif

    m_scaleX = m_scaleX / scaleFactor;
    m_scaleY = m_scaleY / scaleFactor;

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomRect() m_scaleX = %f"), m_scaleX);
#endif

    UpdateAll();

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("mpWindow::ZoomRect() m_scaleX(Updated) = %f"), m_scaleX);
#endif

}

void mpWindow::LockAspect(bool enable)
{
    m_lockaspect = enable;

    m_popmenu.Check(mpID_LOCKASPECT, enable);

    if (m_lockaspect)
    {
        double s = (m_scaleX + m_scaleY)/2;
        m_scaleX = s;
        m_scaleY = s;
    }

    UpdateAll();
}

void mpWindow::OnShowPopupMenu(wxMouseEvent &event)
{
    // Only display menu if the user has not "dragged" the figure
    if (m_enableMouseNavigation)
    {
        SetCursor( *wxSTANDARD_CURSOR );
    }

    if (!m_mouseMovedAfterRightClick)   // JLB
    {
        m_clickedX = event.GetX();
        m_clickedY = event.GetY();
        PopupMenu( &m_popmenu, event.GetX(), event.GetY());
    }
}

void mpWindow::OnLockAspect(wxCommandEvent &event)
{
    LockAspect( !m_lockaspect ); //m_popmenu.IsChecked(mpID_LOCKASPECT) );
}

void mpWindow::OnMouseHelp(wxCommandEvent &event)
{
    wxMessageBox(_("Supported Mouse commands:\n \
        - Mouse Left Click + Move: Rectangular zoom\n \
        - Mouse Right Click + Move: Pan (Move)\n \
        - Mouse Wheel: Vertical scroll\n \
        - Mouse Wheel + SHIFT: Horizontal scroll\n \
        - Mouse Wheel + CTRL: Zoom in/out"),_("wxMathPlot help"),wxOK,this);
}

void mpWindow::OnFit(wxCommandEvent &event)
{
    Fit();
}

void mpWindow::OnCenter(wxCommandEvent &event)
{
    int cx, cy;
    GetClientSize(&cx, &cy);
    SetPos( (double)(m_clickedX-cx/2) / m_scaleX + m_posX, (double)(cy/2-m_clickedY) / m_scaleY + m_posY);
}

void mpWindow::OnZoomIn(wxCommandEvent &event)
{
    int cx, cy;
    GetClientSize(&cx, &cy);
    m_posX = (double)(m_clickedX-cx/2) / m_scaleX + m_posX;
    m_posY = (double)(cy/2-m_clickedY) / m_scaleY + m_posY;
    ZoomIn();
}

void mpWindow::OnZoomOut(wxCommandEvent &event)
{
    ZoomOut();
}

void mpWindow::OnSize( wxSizeEvent &event )
{
    UpdateAll();
}

bool mpWindow::AddLayer( mpLayer* layer, bool refreshDisplay )
{
    if (layer != NULL) {
	m_layers.push_back( layer );
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


void mpWindow::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc(this);
    dc.GetSize(&m_scrX, &m_scrY);   // This is the size of the visible area only!


#ifdef MATHPLOT_DO_LOGGING
    {
        int px, py;
        GetViewStart( &px, &py );
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
    trgDc->SetDeviceOrigin(0,0);
    trgDc->SetPen( *wxTRANSPARENT_PEN );
    wxBrush brush( GetBackgroundColour() );
    trgDc->SetBrush( brush );
    trgDc->DrawRectangle(0,0,m_scrX,m_scrY);

    // Draw all the layers:
    trgDc->SetDeviceOrigin( m_scrX>>1, m_scrY>>1);  // Origin at the center
    wxLayerList::iterator li;
    for (li = m_layers.begin(); li != m_layers.end(); li++)
    {
    	(*li)->Plot(*trgDc, *this);
    };

    // If doublebuffer, draw now to the window:
    if (m_enableDoubleBuffer)
    {
        trgDc->SetDeviceOrigin(0,0);
        dc.SetDeviceOrigin(0,0);  // Origin at the center
        dc.Blit(0,0,m_scrX,m_scrY,trgDc,0,0);
    }
}

void mpWindow::OnScroll2(wxScrollWinEvent &event)
{
    int width, height;
    GetClientSize( &width, &height);
    int px, py;
    GetViewStart( &px, &py);

    if (event.GetOrientation() == wxHORIZONTAL)
    {
        //SetPosX( (double)px / GetScaleX() + m_minX + (double)(width>>1)/GetScaleX());
        m_posX = m_minX + (double)(px + (width>>1))/GetScaleX();
    }
    else
    {
        //SetPosY( m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY());
        //m_posY = m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY();
        m_posY = m_maxY - (double)(py + (height>>1))/GetScaleY();
    }

#ifdef MATHPLOT_DO_LOGGING
    wxLogMessage(_("[mpWindow::OnScroll2] m_posX=%f m_posY=%f"),m_posX,m_posY);
#endif

    FitInside();
    Refresh( FALSE );
    event.Skip();
}

bool mpWindow::UpdateBBox()
{
    bool first = TRUE;

    for (wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)
    {
        mpLayer* f = *li;

        if (f->HasBBox())
        {
            if (first)
            {
                first = FALSE;
                m_minX = f->GetMinX(); m_maxX=f->GetMaxX();
                m_minY = f->GetMinY(); m_maxY=f->GetMaxY();
            }
            else
            {
                if (f->GetMinX()<m_minX) m_minX=f->GetMinX(); if (f->GetMaxX()>m_maxX) m_maxX=f->GetMaxX();
                if (f->GetMinY()<m_minY) m_minY=f->GetMinY(); if (f->GetMaxY()>m_maxY) m_maxY=f->GetMaxY();
            }
        }
        //node = node->GetNext();
    }

    return first == FALSE;
}

void mpWindow::UpdateAll()
{
    bool box = UpdateBBox();
    if (box)
    {
        int cx, cy;
        GetClientSize( &cx, &cy);

        // The "virtual size" of the scrolled window:
        const int sx = (int)((m_maxX - m_minX) * GetScaleX());
        const int sy = (int)((m_maxY - m_minY) * GetScaleY());

        const int px = (int)((GetPosX() - m_minX) * GetScaleX() - (cx>>1));

        // J.L.Blanco, Aug 2007: Formula fixed:
        const int py = (int)((m_maxY - GetPosY()) * GetScaleY() - (cy>>1));

        SetScrollbars( 1, 1, sx, sy, px, py, TRUE);

#ifdef MATHPLOT_DO_LOGGING
        wxLogMessage(_("[mpWindow::UpdateAll] Size:%ix%i ScrollBars:%i,%i"),sx,sy,px,py);
#endif
    }

    FitInside();
    Refresh( FALSE );
}

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

//-----------------------------------------------------------------------------
// mpFXYVector implementation - by Jose Luis Blanco (AGO-2007)
//-----------------------------------------------------------------------------

IMPLEMENT_CLASS(mpFXYVector, mpFXY)

// Constructor
mpFXYVector::mpFXYVector(wxString name, int flags ) : mpFXY(name,flags)
{
    m_index = 0;
    m_minX  = -1;
    m_maxX  = 1;
    m_minY  = -1;
    m_maxY  = 1;
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

void mpFXYVector::SetData( const std::vector<float> &xs,const std::vector<float> &ys)
{
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

        std::vector<float>::const_iterator  it;

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

//-----------------------------------------------------------------------------
// mpText - provided by Val Greene
//-----------------------------------------------------------------------------

IMPLEMENT_CLASS(mpText, mpLayer)


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
}

/** mpText Layer plot handler.
This implementation will plot the text adjusted to the visible area.
*/

void mpText::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen(m_pen);
    dc.SetFont(m_font);

    wxCoord tw=0, th=0;
    dc.GetTextExtent( GetName(), &tw, &th);

    int left = -dc.LogicalToDeviceX(0);
    int width = dc.LogicalToDeviceX(0) - left;
    int bottom = dc.LogicalToDeviceY(0);
    int height = bottom - -dc.LogicalToDeviceY(0);

    dc.DrawText( GetName(),
    (int)((((float)width/100.0) * m_offsety) + left - (tw/2)),
    (int)((((float)height/100.0) * m_offsetx) - bottom) );
}

//-----------------------------------------------------------------------------
// mpPrintout - provided by Davide Rondini
//-----------------------------------------------------------------------------

mpPrintout::mpPrintout(mpWindow *drawWindow, wxChar *title) : wxPrintout(title)
{
    drawn = false;
    plotWindow = drawWindow;
}

bool mpPrintout::OnPrintPage(int page)
{
   wxDC *trgDc = GetDC();
   if ((trgDc) && (page == 1)) {
        int m_prnX, m_prnY;
        trgDc->GetSize(&m_prnX, &m_prnY);
        // Draw background:
        trgDc->SetDeviceOrigin(0,0);
        trgDc->SetPen( *wxTRANSPARENT_PEN );
        wxBrush brush( plotWindow->GetBackgroundColour() );
        trgDc->SetBrush( brush );
        trgDc->DrawRectangle(0,0,m_prnX,m_prnY);

        // Draw all the layers:
        trgDc->SetDeviceOrigin( m_prnX>>1, m_prnY>>1);  // Origin at the center
        mpLayer *layer;
        for (unsigned int li = 0; li <= plotWindow->CountLayers(); li++) {
            layer = plotWindow->GetLayer(li);
            layer->Plot(*trgDc, *plotWindow);
        };
   }
   return true;
}

bool mpPrintout::HasPage(int page)
{
    return (page == 1);
}


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
        ::wxLogError(wxT("[mpMovableObject::ShapeUpdated] Error, m_shape_xs and m_shape_ys have different lengths!"));
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
                dc.DrawPoint( (wxCoord) (( *(itX++) - w.GetPosX()) * w.GetScaleX()) ,
                              (wxCoord) ((w.GetPosY() - *(itY++)) * w.GetScaleY()) );
            }
        }
        else
        {
            while (itX!=m_trans_shape_xs.end())
            {
                wxCoord cx = (wxCoord) ((*(itX++)  - w.GetPosX()) * w.GetScaleX());
                wxCoord cy = (wxCoord) ((w.GetPosY() - *(itY++)) * w.GetScaleY());
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
            wxCoord cx = (wxCoord) ((*(itX++) - w.GetPosX()) * w.GetScaleX());
            wxCoord cy = (wxCoord) ((w.GetPosY() - *(itY++)) * w.GetScaleY());
            if (first)
            {
                first=FALSE;
                cx0=cx;cy0=cy;
            }
            dc.DrawLine(cx0, cy0, cx, cy);
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

//-----------------------------------------------------------------------------
// mpCovarianceEllipse - provided by Jose Luis Blanco
//-----------------------------------------------------------------------------

// Called to update the m_shape_xs, m_shape_ys vectors, whenever a parameter changes.
void mpCovarianceEllipse::RecalculateShape()
{
    m_shape_xs.clear();
    m_shape_ys.clear();

    // Preliminar checks:
    if (m_quantiles<0)  { ::wxLogError(wxT("[mpCovarianceEllipse] Error: quantiles must be non-negative")); return; }
    if (m_cov_00<0)     { ::wxLogError(wxT("[mpCovarianceEllipse] Error: cov(0,0) must be non-negative")); return; }
    if (m_cov_11<0)     { ::wxLogError(wxT("[mpCovarianceEllipse] Error: cov(1,1) must be non-negative")); return; }

    m_shape_xs.resize( m_segments,0 );
    m_shape_ys.resize( m_segments,0 );

    // Compute the two eigenvalues of the covariance:
    // -------------------------------------------------
    double b = -m_cov_00 - m_cov_11;
    double c = m_cov_00*m_cov_11 - m_cov_01*m_cov_01;

    double D = b*b - 4*c;

    if (D<0)     { ::wxLogError(wxT("[mpCovarianceEllipse] Error: cov is not positive definite")); return; }

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
        ::wxLogError(wxT("[mpPolygon] Error: points_xs and points_ys must have the same number of elements"));
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
        ::wxLogError(wxT("[mpBitmapLayer] Assigned bitmap is not Ok()!"));
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
    if (m_validImg)
    {
        // ** In pixels: **
        //
        // (x0,y0) .................  (x1,y0)
        //    .                          .
        //    .                          .
        // (x0,y1) ................   (x1,y1)

        wxCoord x0 = (wxCoord) (( m_min_x - w.GetPosX()) * w.GetScaleX());
        wxCoord y0 = (wxCoord) (( w.GetPosY() - m_max_y) * w.GetScaleY());
        wxCoord x1 = (wxCoord) (( m_max_x - w.GetPosX()) * w.GetScaleX());
        wxCoord y1 = (wxCoord) (( w.GetPosY() - m_min_y) * w.GetScaleY());

        // Build scaled bitmap from the image:
        if (m_scaledBitmap.GetWidth()!=(x1-x0) || m_scaledBitmap.GetHeight()!=(y1-y0))
        {
            m_scaledBitmap = wxBitmap(m_bitmap.Scale(x1-x0,y1-y0));
        }

        // Draw it:
        dc. DrawBitmap( m_scaledBitmap, x0,y0, true );
    }

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
