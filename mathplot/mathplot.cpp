/////////////////////////////////////////////////////////////////////////////
// Name:        mathplot.cpp
// Purpose:     Framework for mathematical graph plotting in wxWindows
// Author:      David Schalig
// Modified by: Davide Rondini
// Created:     21/07/2003
// Last edit:   13/06/2007
// Copyright:   (c) David Schalig, Davide Rondini
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifdef __GNUG__
// #pragma implementation "plot.h"
#pragma implementation "mathplot.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

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
#endif

#include "mathplot.h"
#include "wx/bmpbuttn.h"
#include "wx/module.h"

#include <math.h>
#include <cstdio> // used only for debug

//-----------------------------------------------------------------------------
// mpLayer
//-----------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(mpLayer, wxObject)

mpLayer::mpLayer()
{
    SetPen((wxPen&) *wxBLACK_PEN);
    SetFont((wxFont&) *wxNORMAL_FONT);
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

    if (!m_name.IsEmpty())
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

    if (!m_name.IsEmpty())
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
        //else
        //{
        //}

        dc.DrawText( m_name, tx, ty);
    }
}

//-----------------------------------------------------------------------------
// mpLayer implementations - furniture (scales, ...)
//-----------------------------------------------------------------------------

#define mpLN10 2.3025850929940456840179914546844

IMPLEMENT_CLASS(mpScaleX, mpLayer)

mpScaleX::mpScaleX(wxString name)
{ 
    SetName(name);
    SetFont( (wxFont&) *wxSMALL_FONT);
    SetPen( (wxPen&) *wxGREY_PEN);
}

void mpScaleX::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);

    const int orgy   = (int)(w.GetPosY() * w.GetScaleY());
    const int extend = w.GetScrX()/2;

    dc.DrawLine( -extend, orgy, extend, orgy);

    const double dig  = floor( log( 128.0 / w.GetScaleX() ) / mpLN10 );
    const double step = exp( mpLN10 * dig);
    const double end  = w.GetPosX() + (double)extend / w.GetScaleX();

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

    double n = floor( (w.GetPosX() - (double)extend / w.GetScaleX()) / step ) * step ;

    tmp=-65535;
    for (;n < end; n += step)
    {
        const int p = (int)((n - w.GetPosX()) * w.GetScaleX());
        dc.DrawLine( p, orgy, p, orgy+4);

        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((p-tx/2-tmp) > 64)
        {
            dc.DrawText( s, p-tx/2, orgy+4);
            tmp=p+tx/2;
        }
    }
    
    dc.GetTextExtent(m_name, &tx, &ty);
    dc.DrawText( m_name, extend - tx - 4, orgy + 4 + ty);
}

IMPLEMENT_CLASS(mpScaleY, mpLayer)

mpScaleY::mpScaleY(wxString name)
{ 
    SetName(name);
    SetFont( (wxFont&) *wxSMALL_FONT);
    SetPen( (wxPen&) *wxGREY_PEN);
}

void mpScaleY::Plot(wxDC & dc, mpWindow & w)
{
    dc.SetPen( m_pen);
    dc.SetFont( m_font);

    const int orgx   = -(int)(w.GetPosX() * w.GetScaleX());
    const int extend = w.GetScrY()/2;

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
        dc.DrawLine( orgx, p, orgx+4, p);

        s.Printf(fmt, n);
        dc.GetTextExtent(s, &tx, &ty);
        if ((tmp-p+ty/2) > 32)
        {
            dc.DrawText( s, orgx+4, p-ty/2);
            tmp=p-ty/2;
        }
    }

    dc.GetTextExtent(m_name, &tx, &ty);
    dc.DrawText( m_name, orgx-tx-4, -extend + ty + 4);
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
    EVT_RIGHT_UP ( mpWindow::OnShowPopupMenu)
    EVT_MENU( mpID_CENTER,    mpWindow::OnCenter)
    EVT_MENU( mpID_FIT,       mpWindow::OnFit)
    EVT_MENU( mpID_ZOOM_IN,   mpWindow::OnZoomIn)
    EVT_MENU( mpID_ZOOM_OUT,  mpWindow::OnZoomOut)
    EVT_MENU( mpID_LOCKASPECT,mpWindow::OnLockAspect)
END_EVENT_TABLE()

mpWindow::mpWindow( wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int flag )
    : wxScrolledWindow( parent, id, pos, size, flag, wxT("mathplot") )
{
    m_scaleX = m_scaleY = 1.0;
    m_posX   = m_posY   = 0;
    m_scrX   = m_scrX   = 64;
    m_minX   = m_minY   = 0;
    m_maxX   = m_maxY   = 0;

    m_lockaspect = FALSE;

    m_popmenu.Append( mpID_CENTER,     _("Center"),      _("Center plot view to this position"));
    m_popmenu.Append( mpID_FIT,        _("Fit"),         _("Set plot view to show all items"));
    m_popmenu.Append( mpID_ZOOM_IN,    _("Zoom in"),     _("Zoom in plot view."));
    m_popmenu.Append( mpID_ZOOM_OUT,   _("Zoom out"),    _("Zoom out plot view."));
    m_popmenu.AppendCheckItem( mpID_LOCKASPECT, _("Lock aspect"), _("Lock horizontal and vertical zoom aspect."));

    //m_layers.DeleteContents(TRUE);
    m_layers.clear();
    SetBackgroundColour( *wxWHITE );
    EnableScrolling(FALSE, FALSE);
    SetSizeHints(128, 128);

    UpdateAll();
}

mpWindow::~mpWindow()
{
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
            printf("mpWindow::Fit() m_scaleX = %f , cx = %d, d = %f\n", m_scaleX, cx, d);
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
            printf("mpWindow::Fit(lock) m_scaleX = %f\n", m_scaleX);
            m_scaleY = s;
        }

        UpdateAll();
    }
}

void mpWindow::ZoomIn()
{
    m_scaleX = m_scaleX * 2;
    m_scaleY = m_scaleY * 2;
    printf("mpWindow::ZoomIn() m_scaleX = %f ,", m_scaleX);
    UpdateAll();
    printf("mpWindow::ZoomIn() m_scaleX(Updated) = %f\n", m_scaleX);
}

void mpWindow::ZoomOut()
{
    m_scaleX = m_scaleX / 2;
    m_scaleY = m_scaleY / 2;
    printf("mpWindow::ZoomOut() m_scaleX = %f ,", m_scaleX);
    UpdateAll();
    printf("mpWindow::ZoomOut() m_scaleX(Updated) = %f\n", m_scaleX);
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
    m_clickedX = event.GetX();
    m_clickedY = event.GetY();
    PopupMenu( &m_popmenu, event.GetX(), event.GetY());
}

void mpWindow::OnLockAspect(wxCommandEvent &event)
{
    LockAspect( !m_popmenu.IsChecked(mpID_LOCKASPECT) );
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

bool mpWindow::AddLayer( mpLayer* layer)
{
    if (layer != NULL) {
    	int layNo = m_layers.size();
    	m_layers[layNo] = layer;
    	UpdateAll();
    	return true;
    	};
    return false;
    // Old version, using wxList
    /*bool ret = m_layers.Append( layer) != NULL;
    UpdateAll();
    return ret;*/
}

bool mpWindow::DelLayer( mpLayer* layer)
{
    //m_layers.DeleteObject( layer);
    // New version, using wxHashMap, and with layer presence check
    wxLayerList::iterator layIt;
    for (layIt = m_layers.begin(); layIt != m_layers.end(); layIt++) {
    	if (layIt->second == layer) break;
    	};
    if (layIt != m_layers.end()) {
    	m_layers.erase(layIt); // this way only refereice is deleted, layer object still exists!
    	UpdateAll();
    	return true;
    	};
    return false;
}

void mpWindow::OnPaint( wxPaintEvent &event )
{
    wxPaintDC dc(this);
    //dc.BeginDrawing();

    dc.GetSize(&m_scrX, &m_scrY);
    dc.SetDeviceOrigin( m_scrX>>1, m_scrY>>1);

    wxLayerList::iterator li;
    for (li = m_layers.begin(); li != m_layers.end(); li++) {
    	mpLayer* f = li->second;
    	//printf("0x%d : plotting layer %d (%s)\n", (unsigned int) this, (unsigned int) f, (const char*) wxConvCurrent->cWX2MB(f->GetName()) );
    	f->Plot(dc, *this);
    	};

// old version using wxList
/*    wxNode *node = m_layers.GetFirst();
    while (node)
    {
        ((mpLayer*)node->GetData())->Plot( dc, *this);
        node = node->GetNext();
    }
*/
    //dc.EndDrawing();
}

void mpWindow::OnScroll2(wxScrollWinEvent &event)
{
    int width, height;
    GetClientSize( &width, &height);
    int px, py;
    GetViewStart( &px, &py);

    if (event.GetOrientation() == wxHORIZONTAL)
    {
        SetPosX( (double)px / GetScaleX() + m_minX + (double)(width>>1)/GetScaleX());
    }
    else
    {
        SetPosY( m_maxY - (double)py / GetScaleY() - (double)(height>>1)/GetScaleY());
    }
    event.Skip();
}

bool mpWindow::UpdateBBox()
{
    bool first = TRUE;

    //wxNode *node = m_layers.GetFirst();
    //printf("0x%d : %d layers\n", (unsigned int) this, m_layers.size());
    for (wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++) //while(node)
    {
        mpLayer* f = li->second; //(mpLayer*)node->GetData();
        
        //printf("f = 0x%X (name = %s)\n", (unsigned int) f, (const char*) wxConvCurrent->cWX2MB(f->GetName()));
        //bool box2 = f->HasBBox();
        //printf("box2 %d\n", box2);
        
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
    //printf("placeholder 1\n");
    bool box = UpdateBBox();
    //printf("placeholder 3 (Box = %d)\n", (int) box);
    if (box)
    {
        int cx, cy;
        GetClientSize( &cx, &cy);

        const int sx = (int)((m_maxX - m_minX) * GetScaleX());
        const int sy = (int)((m_maxY - m_minY) * GetScaleY());
        const int px = (int)((GetPosX() - m_minX) * GetScaleX() - (cx>>1));
        const int py = (int)((GetPosY() - m_minY) * GetScaleY() - (cy>>1));
        SetScrollbars( 1, 1, sx, sy, px, py);
    }

    FitInside();
    Refresh( TRUE );
}

void mpWindow::SetScaleX(double scaleX)
{
    if (scaleX!=0) m_scaleX=scaleX;
    printf("mpWindow::SetScaleX() m_scaleX = %f, scaleX = %f ", m_scaleX, scaleX);
    UpdateAll();
    printf(" m_scaleX(Updated) = %f\n", m_scaleX);
}

// New methods implemented by Davide Rondini

unsigned int mpWindow::CountLayers()
{	
    //wxNode *node = m_layers.GetFirst();
    unsigned int layerNo = 0;
    for(wxLayerList::iterator li = m_layers.begin(); li != m_layers.end(); li++)//while(node)
    	{
        if (li->second->HasBBox()) layerNo++;
	// node = node->GetNext();
    	};
    return layerNo;
}
